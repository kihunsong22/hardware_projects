#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <util_string.h>

#include "profile.h"


#define MAXPATH 1024

#ifndef MIN
#define MIN(a,b)      ((a)<(b)?(a):(b))
#endif

////////////////////////////////////////////////////////////////////////////////
static char *inifile;

////////////////////////////////////////////////////////////////////////////////
typedef struct tagPROFILEKEY {
    char *name;
    char *value;
    struct tagPROFILEKEY *next;
} PROFILEKEY;

////////////////////////////////////////////////////////////////////////////////
typedef struct tagPROFILESECTION {
    char *name;
    struct tagPROFILEKEY *key;
    struct tagPROFILESECTION *next;
} PROFILESECTION;

////////////////////////////////////////////////////////////////////////////////
typedef struct {
    int changed;
    PROFILESECTION *section;
    char *dos_name;
} PROFILE;


/* Cached profile file */
static PROFILE CurProfile = { 0, NULL, NULL };

#define PROFILE_MAX_LINE_LEN   (8 * 1024)

/* Check for comments in profile */
#define IS_ENTRY_COMMENT(str)  ((str)[0] == ';')

////////////////////////////////////////////////////////////////////////////////
// Free a profile tree.
////////////////////////////////////////////////////////////////////////////////
static void PROFILE_Free(PROFILESECTION * section)
{
    PROFILESECTION *next_section;
    PROFILEKEY *key, *next_key;

    for (; section; section = next_section) {
    if (section->name)
        free(section->name);
    for (key = section->key; key; key = next_key) {
        next_key = key->next;
        if (key->name)
        free(key->name);
        if (key->value)
        free(key->value);
        free(key);
    }
    next_section = section->next;
    free(section);
    }
}

////////////////////////////////////////////////////////////////////////////////*
//           PROFILE_CopyEntry
//
// Copy the content of an entry into a buffer, removing quotes, and possibly
// translating environment variables.
////////////////////////////////////////////////////////////////////////////////
static void PROFILE_CopyEntry(char *buffer, const char *value, int len,
                  int handle_env)
{
    char quote = '\0';
    const char *p;

    if ((*value == '\'') || (*value == '\"')) {
    if (value[1] && (value[strlen(value) - 1] == *value))
        quote = *value++;
    }
    if (!handle_env) {
    strncpy(buffer, value, len);
    if (quote && (len >= strlen(value)))
        buffer[strlen(buffer) - 1] = '\0';
    return;
    }
    for (p = value; (*p && (len > 1)); *buffer++ = *p++, len--) {
    if ((*p == '$') && (p[1] == '{')) {
        char env_val[1024];
        const char *env_p;
        const char *p2 = strchr(p, '}');
        if (!p2)
        continue;   /* ignore it */
        strncpy(env_val, p + 2,
            MIN(sizeof(env_val), (int) (p2 - p) - 2));
        env_val[(int) (p2 - p) - 2] = quote;
        if ((env_p = getenv(env_val)) != NULL) {
        strncpy(buffer, env_p, len);
        buffer += strlen(buffer);
        len -= strlen(buffer);
        }
        p = p2 + 1;
    }
    }
    *buffer = '\0';
}

////////////////////////////////////////////////////////////////////////////////
//           PROFILE_GetSection
//
// Enumerate all the keys of a section.
////////////////////////////////////////////////////////////////////////////////
static int PROFILE_GetSection(PROFILESECTION * section,
                  const char *section_name,
                  char *buffer, int len, int handle_env)
{
    PROFILEKEY *key;
    while (section) {
    if (section->name && !strcmp(section->name, section_name)) {
        int oldlen = len;
        for (key = section->key; key; key = key->next) {
        if (len <= 2)
            break;
        if (IS_ENTRY_COMMENT(key->name))
            continue;   /* Skip comments */
        PROFILE_CopyEntry(buffer, key->name, len - 1, handle_env);
        len -= strlen(buffer) + 1;
        buffer += strlen(buffer) + 1;
        }
        *buffer = '\0';
        return oldlen - len + 1;
    }
    section = section->next;
    }
    buffer[0] = buffer[1] = '\0';
    return 2;
}

////////////////////////////////////////////////////////////////////////////////
//           PROFILE_DeleteSection
//
// Delete a section from a profile tree.
////////////////////////////////////////////////////////////////////////////////
static int PROFILE_DeleteSection(PROFILESECTION ** section,
                 const char *name)
{
    while (*section) {
    if ((*section)->name && !strcmp((*section)->name, name)) {
        PROFILESECTION *to_del = *section;
        *section = to_del->next;
        to_del->next = NULL;
        PROFILE_Free(to_del);
        return 1;
    }
    section = &(*section)->next;
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//           PROFILE_DeleteKey
//
// Delete a key from a profile tree.
////////////////////////////////////////////////////////////////////////////////
static int PROFILE_DeleteKey(PROFILESECTION ** section,
                 const char *section_name,
                 const char *key_name)
{
    while (*section) {
    if ((*section)->name && !strcmp((*section)->name, section_name)) {
        PROFILEKEY **key = &(*section)->key;
        while (*key) {
        if (!strcmp((*key)->name, key_name)) {
            PROFILEKEY *to_del = *key;
            *key = to_del->next;
            if (to_del->name)
            free(to_del->name);
            if (to_del->value)
            free(to_del->value);
            free(to_del);
            return 1;
        }
        key = &(*key)->next;
        }
    }
    section = &(*section)->next;
    }
    return 0;
}



////////////////////////////////////////////////////////////////////////////////
//           PROFILE_Load
//
// Load a profile tree from a file.
////////////////////////////////////////////////////////////////////////////////
static PROFILESECTION *PROFILE_Load(FILE * file)
{
    char buffer[PROFILE_MAX_LINE_LEN];
    char *p, *p2;
    int line = 0;
    PROFILESECTION *section, *first_section;
    PROFILESECTION **prev_section;
    PROFILEKEY *key, **prev_key;

    first_section = (PROFILESECTION *) malloc(sizeof(*section));
    first_section->name = NULL;
    first_section->key = NULL;
    first_section->next = NULL;
    prev_section = &first_section->next;
    prev_key = &first_section->key;

    while (fgets(buffer, PROFILE_MAX_LINE_LEN, file)) {
    line++;
    p = buffer + strlen(buffer) - 1;
    while ((p > buffer) && ((*p == '\n') || isspace((int)*p)))
        *p-- = '\0';
    p = buffer;
    while (*p && isspace((int)*p))
        p++;
    if (*p == '[') {    /* section start */
        if (!(p2 = strrchr(p, ']'))) {
        fprintf(stderr,
            "PROFILE_Load: Invalid section header at line %d: '%s'\n",
            line, p);
        } else {
        *p2 = '\0';
        p++;
        section = (PROFILESECTION *) malloc(sizeof(*section));
        section->name = strdup(p);
        section->key = NULL;
        section->next = NULL;
        *prev_section = section;
        prev_section = &section->next;
        prev_key = &section->key;
        continue;
        }
    }
    if ((p2 = strchr(p, '=')) != NULL) {
        char *p3 = p2 - 1;
        while ((p3 > p) && isspace((int)*p3))
        *p3-- = '\0';
        *p2++ = '\0';
        while (*p2 && isspace((int)*p2))
        p2++;
    }
    key = (PROFILEKEY *) malloc(sizeof(*key));
    key->name = strdup(p);
    key->value = p2 ? strdup(p2) : NULL;
    key->next = NULL;
    *prev_key = key;
    prev_key = &key->next;
    }
    return first_section;
}



////////////////////////////////////////////////////////////////////////////////
//           PROFILE_Save
//
// Save a profile tree to a file.
////////////////////////////////////////////////////////////////////////////////
static void PROFILE_Save(FILE * file, PROFILESECTION * section)
{
    PROFILEKEY *key;

    for (; section; section = section->next) {
    if (section->name)
        fprintf(file, "[%s]\r\n", section->name);
    for (key = section->key; key; key = key->next) {
        fprintf(file, "%s", key->name);
        if (key->value)
        fprintf(file, "=%s", key->value);
        fprintf(file, "\r\n");
    }
    }
}



////////////////////////////////////////////////////////////////////////////////
//           PROFILE_FlushFile
//
// Flush the current profile to disk if changed.
////////////////////////////////////////////////////////////////////////////////
static int PROFILE_FlushFile(void)
{
    FILE *file = NULL;

    if (!CurProfile.changed || !CurProfile.dos_name)
    return 1;
    if (!(file = fopen(CurProfile.dos_name, "wb"))) {
    fprintf(stderr, "Warning: could not save profile file %s\n",
        CurProfile.dos_name);
    return 0;
    }
    PROFILE_Save(file, CurProfile.section);
    fclose(file);
    CurProfile.changed = 0;
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
//           PROFILE_Open
//
// Open a profile file, checking the cached file first.
////////////////////////////////////////////////////////////////////////////////
static int PROFILE_Open(const char *filename)
{
    char *newdos_name;
    FILE *file = NULL;

    assert( ((int)filename) );
    assert( strlen(filename) );

    if (CurProfile.dos_name && !strcmp(filename, CurProfile.dos_name)) {
    return 1;
    }
    /* Flush the previous profile */

    newdos_name = strdup(filename);
    PROFILE_FlushFile();
    PROFILE_Free(CurProfile.section);
    if (CurProfile.dos_name)
    free(CurProfile.dos_name);
    CurProfile.section = NULL;
    CurProfile.dos_name = newdos_name;

    if ((file = fopen(newdos_name, "r"))) {
    CurProfile.section = PROFILE_Load(file);
    fclose(file);
    }
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
//           PROFILE_Find
//
// Find a key in a profile tree, optionally creating it.
////////////////////////////////////////////////////////////////////////////////
static PROFILEKEY *PROFILE_Find(PROFILESECTION ** section,
                const char *section_name,
                const char *key_name, int create)
{
    while (*section) {
    if ((*section)->name && !strcmp((*section)->name, section_name)) {
        PROFILEKEY **key = &(*section)->key;
        while (*key) {
        if (!strcmp((*key)->name, key_name))
            return *key;
        key = &(*key)->next;
        }
        if (!create)
        return NULL;
        *key = (PROFILEKEY *) malloc(sizeof(PROFILEKEY));
        (*key)->name = strdup(key_name);
        (*key)->value = NULL;
        (*key)->next = NULL;
        return *key;
    }
    section = &(*section)->next;
    }
    if (!create)
    return NULL;
    *section = (PROFILESECTION *) malloc(sizeof(PROFILESECTION));
    (*section)->name = strdup(section_name);
    (*section)->next = NULL;
    (*section)->key = (PROFILEKEY *) malloc(sizeof(PROFILEKEY));
    (*section)->key->name = strdup(key_name);
    (*section)->key->value = NULL;
    (*section)->key->next = NULL;
    return (*section)->key;
}




////////////////////////////////////////////////////////////////////////////////
//           PROFILE_GetString
//
// Get a profile string.
////////////////////////////////////////////////////////////////////////////////
static int PROFILE_GetStr(const char *section, const char *key_name,
                 const char *def_val, char *buffer, int len)
{
    PROFILEKEY *key = NULL;

    if (!def_val)
    def_val = "";
    if (key_name) {
    key = PROFILE_Find(&CurProfile.section, section, key_name, 0);
    PROFILE_CopyEntry(buffer,
              (key
               && key->value) ? key->value : def_val, len, 1);
    return strlen(buffer);
    }
    return PROFILE_GetSection(CurProfile.section, section, buffer, len, 0);
}


////////////////////////////////////////////////////////////////////////////////
//           PROFILE_SetString
//
// Set a profile string.
////////////////////////////////////////////////////////////////////////////////
static int PROFILE_SetString(const char *section_name,
                 const char *key_name, const char *value)
{
    int ret;

    if (!key_name) {        /* Delete a whole section */
    ret = PROFILE_DeleteSection(&CurProfile.section, section_name);
    CurProfile.changed |= ret;
    return ret;
    } else if (!value) {    /* Delete a key */
    ret =
        PROFILE_DeleteKey(&CurProfile.section, section_name, key_name);
    CurProfile.changed |= ret;
    return ret;
    } else {            /* Set the key value */
    PROFILEKEY *key = PROFILE_Find(&CurProfile.section, section_name,
                       key_name, 1);
    if (key->value) {
        if (!strcmp(key->value, value)) {
        return 1;   /* No change needed */
        }
        free(key->value);
    }
    key->value = strdup(value);
    CurProfile.changed = 1;
    return 1;
    }
}



////////////////////////////////////////////////////////////////////////////////
//        GetPrivateProfileInt
//
////////////////////////////////////////////////////////////////////////////////
unsigned GetPrivateProfileInteger(char *section, char *entry, int def_val,
                  char *filename)
{
    char buffer[20];
    char *p;
    long result;

    GetPrivateProfileStr(section, entry, "",
                buffer, sizeof(buffer), filename);
    if (!buffer[0])
    return (unsigned) def_val;
    result = strtol(buffer, &p, 0);
    if (p == buffer)
    return 0;       /* No digits at all */
    return (unsigned) result;
}

unsigned GetProfileInteger(char *section, char *entry, int def_val)
{
    return GetPrivateProfileInteger(section, entry, def_val, inifile);
}

////////////////////////////////////////////////////////////////////////////////
//          GetPrivateProfileString
//
////////////////////////////////////////////////////////////////////////////////
int GetPrivateProfileStr(char *section, char *entry, char *def_val,
                char *buffer, int len, char *filename)
{
    if (PROFILE_Open(filename))
    return PROFILE_GetStr(section, entry, def_val, buffer, len);
    strncpy(buffer, def_val, len);
    return strlen(buffer);
}

int GetProfileStr(char *section, char *entry, char *def_val,
             char *buffer, int len)
{
    return GetPrivateProfileStr(section, entry, def_val,
                   buffer, len, inifile);
}

////////////////////////////////////////////////////////////////////////////////
//          GetPrivateProfileFloat
//
////////////////////////////////////////////////////////////////////////////////
double GetPrivateProfileFloat(char *section, char *entry, double def_val,
                  char *filename)
{
    char buffer[20];
    char *p;
    double result;

    GetPrivateProfileStr(section, entry, "",
                buffer, sizeof(buffer), filename);
    if (!buffer[0])
    return (double) def_val;
    result = strtod(buffer, &p);
    if (p == buffer)
    return 0.0;     /* No digits at all */
    return (double) result;
}

double GetProfileFloat(char *section, char *entry, double def_val)
{
    return GetPrivateProfileFloat(section, entry, def_val, inifile);
}

////////////////////////////////////////////////////////////////////////////////
//          WritePrivateProfileString
//
////////////////////////////////////////////////////////////////////////////////
int WritePrivateProfileStr(char *section, char *entry, char *string,
                  char *filename)
{
    if (!PROFILE_Open(filename))
    return 0;
    if (!section)
    return PROFILE_FlushFile();
    return PROFILE_SetString(section, entry, string);
}

int WriteProfileStr(char *section, char *entry, char *string)
{
    return WritePrivateProfileStr(section, entry, string, inifile);
}

////////////////////////////////////////////////////////////////////////////////
//          WritePrivateProfileInt
//
////////////////////////////////////////////////////////////////////////////////
int WritePrivateProfileInt(char *section, char *entry, int value,
               char *filename)
{
    char bufp[32];

    if (!PROFILE_Open(filename))
    return 0;
    if (!section)
    return PROFILE_FlushFile();
    sprintf(bufp, "%d", value);
    return PROFILE_SetString(section, entry, bufp);
}

int WriteProfileInt(char *section, char *entry, int value)
{
    return WritePrivateProfileInt(section, entry, value, inifile);
}

////////////////////////////////////////////////////////////////////////////////
//          WritePrivateProfileFloat
//
////////////////////////////////////////////////////////////////////////////////
int WritePrivateProfileFloat(char *section, char *entry, double value,
                 char *filename)
{
    char bufp[32];

    if (!PROFILE_Open(filename))
    return 0;
    if (!section)
    return PROFILE_FlushFile();
    sprintf(bufp, "%f", value);
    return PROFILE_SetString(section, entry, bufp);
}

int WriteProfileFloat(char *section, char *entry, double value)
{
    return WritePrivateProfileFloat(section, entry, value, inifile);
}

////////////////////////////////////////////////////////////////////////////////
//          WriteOutProfiles
//
////////////////////////////////////////////////////////////////////////////////
void WriteOutProfiles(void)
{
    PROFILE_FlushFile();
}

////////////////////////////////////////////////////////////////////////////////
//          PROFILE_reload
//
////////////////////////////////////////////////////////////////////////////////
void PROFILE_Reload (char *filename)
{
    if (CurProfile.dos_name) {
        free (CurProfile.dos_name);
        CurProfile.dos_name = NULL;
    }
    PROFILE_Open (filename);
}

////////////////////////////////////////////////////////////////////////////////
void SetIniFile(char *name)
{
    inifile = name;
}

////////////////////////////////////////////////////////////////////////////////
int get_private_option(char *section, char *entry, char *def, char *name)
{
    char bufp[10];

    GetPrivateProfileStr(section, entry, def, bufp, 10, name);

    if (strncasecmp(bufp, "off", 3) == 0)
    return 0;
    if (strncasecmp(bufp, "false", 5) == 0)
    return 0;
    if (strcasecmp(bufp, "not") == 0)
    return 0;
    if (strcasecmp(bufp, "no") == 0)
    return 0;
    if (strcasecmp(bufp, "0") == 0)
    return 0;
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
int get_option(char *section, char *entry, char *def)
{
    return get_private_option(section, entry, def, inifile);
}
