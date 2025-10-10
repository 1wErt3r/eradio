#include "station_list.h"
#include "radio_player.h"
#include "http.h"

static void
_station_click_counter_request(Station *st)
{
   char url_str[1024];
   Ecore_Con_Url *url;

   if (!st || !st->stationuuid) return;

   snprintf(url_str, sizeof(url_str), "http://de2.api.radio-browser.info/xml/url/%s",
            st->stationuuid);

   url = ecore_con_url_new(url_str);
   ecore_con_url_additional_header_add(url, "User-Agent", "eradio/1.0");
   ecore_con_url_get(url);
}

void
_list_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   Elm_Object_Item *it = event_info;
   Station *st = elm_object_item_data_get(it);

   if (!st) return;

   _station_click_counter_request(st);
   radio_player_play(ad, st->url);
}

void
station_list_clear(AppData *ad)
{
    elm_list_clear(ad->list);
}

void
station_list_populate(AppData *ad, Eina_List *stations)
{
    Eina_List *l;
    Station *st;

    station_list_clear(ad);

    EINA_LIST_FOREACH(stations, l, st)
    {
        Evas_Object *icon = elm_icon_add(ad->win);
        elm_icon_standard_set(icon, "radio");
        Elm_Object_Item *li = elm_list_item_append(ad->list, st->name, icon, NULL, _list_item_selected_cb, ad);
        elm_object_item_data_set(li, st);

        if (st->favicon && st->favicon[0])
          {
             http_download_icon(ad, li, st->favicon);
          }
    }
    elm_list_go(ad->list);
}
