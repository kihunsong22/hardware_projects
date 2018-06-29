/*****************************************************************************\
|*
|*  IN PACKAGE:         Software Platform Builder
|*
|*  COPYRIGHT:          Copyright (c) 2009, Altium
|*
|*  DESCRIPTION:        Video player demo - GUI form definitions
|*
 */

#include <agui.h>
#include <stdio.h>
#include "form1.h"
#include "devices.h"
//#include "wave.h"

#include <unistd.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#define DIR_SEP   "/"
#define FAILED()    { printf( "*** Failed ***\n" ); }

listbox_t form1_dirs;
listbox_t form1_files;
label_t form1_dirs_label;
label_t form1_files_label;
button_t form1_play;
button_t form1_stop;
textbox_t form1_info;
obj_t *form1_children[];
char *gui_playfile;

void form1_dirs_action(obj_t *obj, const action_event_t *action);
void form1_files_action(obj_t *obj, const action_event_t *action);
void form1_play_action(obj_t *obj, const action_event_t *action);
void form1_stop_action(obj_t *obj, const action_event_t *action);
void form2_action(obj_t *obj, const action_event_t *action);

obj_t *form1_children[] =
{
    AGUI_HANDLE(form1_dirs),
    AGUI_HANDLE(form1_files),
    AGUI_HANDLE(form1_dirs_label),
    AGUI_HANDLE(form1_files_label),
    AGUI_HANDLE(form1_play),
    AGUI_HANDLE(form1_info),
};

form_t form1 =
{
    .obj.x = 0,
    .obj.y = 0,
    .obj.width = 320,
    .obj.height = 240,
    .obj.draw = form_draw,
    .obj.handler = form_handler,
    .obj.parent = NULL,
    .obj.agui_index = AGUI,
    .obj.cursor_shape = &cursor_arrow,
    .obj.visible = true,
    .obj.enabled = true,
    .caption.x = 0,
    .caption.y = 0,
    .caption.text = "Video Player",
    .caption.font = &bitstreamverasans10,
    .caption.color = BLACK,
    .caption.fontstyle = FS_BOLD,
    .caption.align = ALIGN_CENTRE,
    .captionbarcolor = LIGHTSKYBLUE,
    .children = form1_children,
    .n_children = sizeof(form1_children) / sizeof(form1_children[0]),
    .relief = RELIEF_NONE,
//  .visible = true,
    .color = GRAY15
};

listbox_t form1_dirs =
{
    .obj.x = 20,
    .obj.y = 40,
    .obj.width = 130,
    .obj.height = 123,
    .obj.draw = listbox_draw,
    .obj.handler = listbox_handler,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.agui_index = AGUI,
    .obj.cursor_shape = &cursor_arrow,
    .obj.visible = true,
    .obj.enabled = true,
    .obj.action = form1_dirs_action,
    .count = 0,
    .first = 0,
    .select = NULL,
    .window = 4,
    .relief = RELIEF_LOWERED,
    .separator = true,
    .multiselect = false,
    .item_string.x = 4,
    .item_string.y = 1,
//    .item_string.font = &bitstreamverasans10,
    .item_string.color = BLACK,
    .item_string.fontstyle = FS_NONE,
    .item_string.align = ALIGN_LEFT,
    .selectioncolor = WHITE,
    .color = LIGHTCYAN
};

listbox_t form1_files =
{
    .obj.x = 170,
    .obj.y = 40,
    .obj.width = 130,
    .obj.height = 123,
    .obj.draw = listbox_draw,
    .obj.handler = listbox_handler,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.agui_index = AGUI,
    .obj.cursor_shape = &cursor_arrow,
    .obj.visible = true,
    .obj.enabled = true,
    .obj.action = form1_files_action,
    .count = 0,
    .first = 0,
    .select = NULL,
    .window = 4,
    .relief = RELIEF_LOWERED,
    .separator = true,
    .multiselect = false,
    .item_string.x = 4,
    .item_string.y = 1,
//    .item_string.font = &bitstreamverasans10,
    .item_string.color = BLACK,
    .item_string.fontstyle = FS_NONE,
    .item_string.align = ALIGN_LEFT,
    .selectioncolor = SNOW,
    .color = POWDERBLUE
};

label_t form1_dirs_label =
{
    .obj.x = 22,
    .obj.y = 40,
    .obj.draw = label_draw,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.visible = true,
    .obj.enabled = true,
    .text.text = "Directories",
    .text.x = 0,
    .text.y = 0,
//    .text.font = &bitstreamverasans10,
    .text.color = LIGHTCYAN,
    .text.align = ALIGN_BOTTOM_LEFT,
    .text.fontstyle = FS_NONE
};

label_t form1_files_label =
{
    .obj.x = 172,
    .obj.y = 40,
    .obj.draw = label_draw,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.visible = true,
    .obj.enabled = true,
    .text.text = "Files",
    .text.x = 0,
    .text.y = 0,
//    .text.font = &bitstreamverasans10,
    .text.color = POWDERBLUE,
    .text.align = ALIGN_BOTTOM_LEFT,
    .text.fontstyle = FS_NONE
};

button_t form1_play =
{
    .obj.x = 20,
    .obj.y = 205,
    .obj.width = 90,
    .obj.height = 25,
    .obj.draw = button_draw,
    .obj.handler = button_handler,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.agui_index = AGUI,
    .obj.action = form1_play_action,
    .obj.cursor_shape = &cursor_arrow,
    .obj.visible = true,
    .obj.enabled = false,
    .obj.pressed = false,
    .label.x = 0,
    .label.y = 0,
    .label.text = "Play",
    .label.font = &bitstreamverasans10,
    .label.color = BLACK,
    .label.fontstyle = FS_BOLD,
    .label.align = ALIGN_CENTRE,
    .relief = RELIEF_RAISED,
    .color = MEDIUMSPRINGGREEN
};

char info_text[64] = "";

textbox_t form1_info =
{
    .obj.x = 20,
    .obj.y = 175,
    .obj.width = 280,
    .obj.height = 19,
    .obj.draw = textbox_draw,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.agui_index = AGUI,
    .obj.cursor_shape = &cursor_arrow,
    .obj.visible = true,
    .obj.enabled = true,
    .color = PEACHPUFF,
    .text.x = 2,
    .text.y = 2,
    .text.font = &bitstreamverasans10,
    .text.text = info_text
};

form_t form2 =
{
    .obj.x = 0,
    .obj.y = 0,
    .obj.width = 320,
    .obj.height = 240,
    .obj.draw = form_draw,
    .obj.handler = form_handler,
    .obj.parent = NULL,
    .obj.agui_index = AGUI,
    .obj.cursor_shape = NULL,
    .obj.visible = false,
    .obj.enabled = true,
    .caption.x = 0,
    .caption.y = 0,
    .caption.text = NULL,
    .children = NULL,
    .n_children = 0,
    .relief = RELIEF_NONE,
    .color = GRAY20,
    .obj.action = form2_action,
};


char dir_file[300];

void form1_dirs_action(obj_t *obj, const action_event_t *action)
{
    int index;

    if (action->event == ACTION_RELEASED)
    {
        if (listbox_get_selection(&form1_dirs, &index, 1))
        {
            strcpy(dir_file, listbox_get_text(&form1_dirs, index));
            chdir(dir_file);
            getcwd(dir_file, sizeof(dir_file));
            dirlisting(dir_file);
            listbox_sort(&form1_dirs, false);
            obj_set_enabled(AGUI_HANDLE(form1_play), false);
        }
    }
}

void form1_files_action(obj_t *obj, const action_event_t *action)
{
    int index;

    if (action->event == ACTION_RELEASED)
    {
        if (listbox_get_selection(&form1_files, &index, 1))
        {
            getcwd(dir_file, sizeof(dir_file));
            strcat(dir_file, "/");
            strcat(dir_file, listbox_get_text(&form1_files, index));

            obj_set_enabled(AGUI_HANDLE(form1_play), avi_parseheader(dir_file, form1_info.text.text));
            obj_invalidate(AGUI_HANDLE(form1_info));
        }
        else
        {
            obj_set_enabled(AGUI_HANDLE(form1_play), false);
        }
    }
}

void form1_play_action(obj_t *obj, const action_event_t *action)
{
    int index;

    if (action->event == ACTION_CLICKED)
    {
        if (listbox_get_selection(&form1_files, &index, 1))
        {
            getcwd(dir_file, sizeof(dir_file));
            strcat(dir_file, "/");
            strcat(dir_file, listbox_get_text(&form1_files, index));

            gui_playfile = dir_file;
        }
    }
}


void form2_action(obj_t *obj, const action_event_t *action)
{
    if (action->event == ACTION_PRESSED)
    {
        gui_playfile = NULL;
    }
}


/**********************************************************************
|*
|*  FUNCTION    : dirlisting
|*
|*  PARAMETERS  : path: path of the dir
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : List the contents of the dir
 */

void dirlisting( const char *path )
{
    DIR             *dir;
    struct dirent   *dirent;
    struct stat     buf;
    char            path_to_file[PATH_MAX];

    chdir(path);
    dir = opendir(path);
    if ( dir != NULL )
    {
        printf( "Reading folder \"%s\"\n\n", path );
        listbox_clear(&form1_dirs);
        listbox_clear(&form1_files);
        while ( 1 )
        {
            dirent = readdir(dir);
            if (dirent == NULL)
            {
                break;
            }
            sprintf(path_to_file, "%s%s%s", path, DIR_SEP, dirent->d_name);
            if (stat(path_to_file, &buf) == 0)
            {
                if (S_ISDIR(buf.st_mode))
                {
                    if (strcmp(dirent->d_name, ".") == 0)
                    {
                        /* skip */
                    }
                    else
                    {
                        listbox_add(&form1_dirs, dirent->d_name, NULL);
                    }
                }
                else if (S_ISREG(buf.st_mode))
                {
                    listbox_add(&form1_files, dirent->d_name, NULL);
                }
                else
                {
                    printf("[UNK]\t%s\n", dirent->d_name);
                }
            }
            else
            {
                FAILED();
            }
        }

        if (closedir(dir) != 0)
        {
            FAILED();
        }
    }

    listbox_sort(&form1_dirs, false);
}


