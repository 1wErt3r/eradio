#pragma once

#include <Elementary.h>

typedef struct _AppData AppData;

void ui_create(AppData *ad);
void ui_update_server_list(AppData *ad);
void ui_set_load_more_button_visibility(AppData *ad, Eina_Bool visible);
