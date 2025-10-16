#pragma once

#include <Elementary.h>

typedef struct _AppData AppData;

void ui_create(AppData *ad);
void ui_update_server_list(AppData *ad);

void ui_loading_start(AppData *ad);
void ui_loading_stop(AppData *ad);
void ui_show_error_dialog(AppData *ad, const char *message);
