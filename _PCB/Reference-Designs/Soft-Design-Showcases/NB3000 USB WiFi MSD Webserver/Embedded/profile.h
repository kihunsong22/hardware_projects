#ifndef PROFILE__H
#define PROFILE__H

unsigned GetProfileInteger(char *section, char *entry, int def_val);
int GetProfileStr(char *section, char *entry, char *def_val,char *buffer, int len);
double GetProfileFloat(char *section, char *entry, double def_val);
int WriteProfileStr(char *section, char *entry, char *string);
int WriteProfileInt(char *section, char *entry, int value);
int WriteProfileFloat(char *section, char *entry, double value);
unsigned GetPrivateProfileInteger(char *section, char *entry, int def_val,char *filename);
int GetPrivateProfileStr(char *section, char *entry, char *def_val,char *buffer, int len, char *filename);
double GetPrivateProfileFloat(char *section, char *entry, double def_val,char *filename);
int WritePrivateProfileStr(char *section, char *entry, char *string,char *filename);
int WritePrivateProfileInt(char *section, char *entry, int value,char *filename);
int WritePrivateProfileFloat(char *section, char *entry, double value,char *filename);
void WriteOutProfiles(void);
int get_option(char *section, char *entry, char *def);
int get_private_option(char *section, char *entry, char *def, char *name);
void PROFILE_Reload (char *filename);
void SetIniFile(char *name);

#endif	/* PROFILE__H */
