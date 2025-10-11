#pragma once

#include "appdata.h"

void station_list_populate(AppData *ad, Eina_List *stations, Eina_Bool new_search);
void station_list_clear(AppData *ad);
void _list_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
