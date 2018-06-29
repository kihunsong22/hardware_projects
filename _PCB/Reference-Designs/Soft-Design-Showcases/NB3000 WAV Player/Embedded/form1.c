#include <agui.h>
#include <stdio.h>
#include "form1.h"
#include "devices.h"
#include "wave.h"

#include <unistd.h>
#include <string.h>

obj_t *form1_children[] =
{
    AGUI_HANDLE(form1_dirs),
    AGUI_HANDLE(form1_files),
    AGUI_HANDLE(form1_dirs_label),
    AGUI_HANDLE(form1_files_label),
    AGUI_HANDLE(form1_play),
    AGUI_HANDLE(form1_stop),
    AGUI_HANDLE(form1_progress),
};

bool play = false;

form_t form1 =
{
    .obj.x = 0,
    .obj.y = 0,
    .obj.width = 320,
    .obj.height = 240,
    .obj.draw = form_draw,
    .obj.handler = form_handler,
    .obj.parent = NULL,
    .obj.agui_index = AGUI_1,
    .obj.cursor_shape = &cursor_arrow,
    .obj.visible = true,
    .obj.enabled = true,
    .caption.x = 0,
    .caption.y = 0,
    .caption.text = "Audio Player",
    .caption.font = &bitstreamverasans10,
    .caption.color = BLACK,
    .caption.fontstyle = FS_BOLD,
    .caption.align = ALIGN_CENTRE,
    .captionbarcolor = LIGHTSKYBLUE,
    .children = form1_children,
    .n_children = sizeof(form1_children) / sizeof(form1_children[0]),
    .relief = RELIEF_NONE,
    .color = GRAY15
};

listbox_t form1_dirs =
{
    .obj.x = 20,
    .obj.y = 40,
    .obj.width = 135,
    .obj.height = 120,
    .obj.draw = listbox_draw,
    .obj.handler = listbox_handler,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.agui_index = AGUI_1,
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
    .obj.x = 165,
    .obj.y = 40,
    .obj.width = 135,
    .obj.height = 120,
    .obj.draw = listbox_draw,
    .obj.handler = listbox_handler,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.agui_index = AGUI_1,
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
    .obj.x = 167,
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
    .obj.agui_index = AGUI_1,
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

button_t form1_stop =
{
    .obj.x = 210,
    .obj.y = 205,
    .obj.width = 90,
    .obj.height = 25,
    .obj.draw = button_draw,
    .obj.handler = button_handler,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.agui_index = AGUI_1,
    .obj.action = form1_stop_action,
    .obj.cursor_shape = &cursor_arrow,
    .obj.visible = true,
    .obj.enabled = false,
    .obj.pressed = false,
    .label.x = 0,
    .label.y = 0,
    .label.text = "Stop",
    .label.font = &bitstreamverasans10,
    .label.color = BLACK,
    .label.fontstyle = FS_BOLD,
    .label.align = ALIGN_CENTRE,
    .relief = RELIEF_RAISED,
    .color = CRIMSON
};

progressbar_t form1_progress =
{
    .obj.x = 20,
    .obj.y = 175,
    .obj.width = 280,
    .obj.height = 19,
    .obj.draw = progressbar_draw,
    .obj.parent = AGUI_HANDLE(form1),
    .obj.agui_index = AGUI_1,
    .obj.cursor_shape = &cursor_arrow,
    .obj.visible = true,
    .obj.enabled = true,
    .percentage = 0,
    .progresscolor = GRAY30,
    .color = PEACHPUFF
};

char info_string[100];
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
            if (wav_readheader(dir_file))
            {
                obj_set_enabled(AGUI_HANDLE(form1_play), true);
            }
            else
            {
                obj_set_enabled(AGUI_HANDLE(form1_play), false);
            }
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

    if (action->event == ACTION_RELEASED)
    {
        if (listbox_get_selection(&form1_files, &index, 1))
        {
            getcwd(dir_file, sizeof(dir_file));
            strcat(dir_file, "/");
            strcat(dir_file, listbox_get_text(&form1_files, index));
            /* play */
            play = true;
            obj_set_enabled(AGUI_HANDLE(form1_play), false);
            obj_set_enabled(AGUI_HANDLE(form1_stop), true);
            obj_set_enabled(AGUI_HANDLE(form1_files), false);
            obj_set_enabled(AGUI_HANDLE(form1_dirs), false);
        }
    }
}


void form1_stop_action(obj_t *obj, const action_event_t *action)
{
    if (action->event == ACTION_RELEASED)
    {
        play = false;
        obj_set_enabled(AGUI_HANDLE(form1_play), true);
        obj_set_enabled(AGUI_HANDLE(form1_stop), false);
        obj_set_enabled(AGUI_HANDLE(form1_files), true);
        obj_set_enabled(AGUI_HANDLE(form1_dirs), true);
    }
}

