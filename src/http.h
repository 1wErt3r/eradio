#pragma once

#include "appdata.h"

void http_init(AppData *ad);
void http_shutdown(void);
void http_search_stations(AppData *ad, const char *search_term, const char *search_type, int offset, int limit, Eina_Bool new_search);
void http_download_icon(AppData *ad, Elm_Object_Item *list_item, const char *url);
void _search_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
void _search_entry_activated_cb(void *data, Evas_Object *obj, void *event_info);
void http_station_click_counter(AppData *ad, const char *uuid);
