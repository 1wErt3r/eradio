#include "station_list.h"
#include "radio_player.h"
#include "http.h"
#include "favorites.h"

static void _favorite_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);

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
        Evas_Object *fav_btn = elm_button_add(ad->win);
        evas_object_size_hint_min_set(fav_btn, 40, 40);
        evas_object_propagate_events_set(fav_btn, EINA_FALSE);
        if (st->favorite)
          elm_object_text_set(fav_btn, "★");
        else
          elm_object_text_set(fav_btn, "☆");

        Elm_Object_Item *li = elm_list_item_append(ad->list, st->name, icon, fav_btn, _list_item_selected_cb, ad);
        elm_object_item_data_set(li, st);

        // Attach callback with context so we can save favorites on toggle
        typedef struct {
            AppData *ad;
            Elm_Object_Item *li;
        } FavCtx;
        FavCtx *ctx = calloc(1, sizeof(FavCtx));
        ctx->ad = ad;
        ctx->li = li;
        evas_object_smart_callback_add(fav_btn, "clicked", _favorite_btn_clicked_cb, ctx);

        if (st->favicon && st->favicon[0])
          {
             http_download_icon(ad, li, st->favicon);
          }
    }
    elm_list_go(ad->list);
}

static void
_favorite_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    typedef struct {
        AppData *ad;
        Elm_Object_Item *li;
    } FavCtx;

    FavCtx *ctx = data;
    if (!ctx) return;
    Station *st = elm_object_item_data_get(ctx->li);
    if (!st) { free(ctx); return; }

    st->favorite = !st->favorite;

    // Update button text to reflect state; use star characters for a clear fallback
    if (st->favorite)
      elm_object_text_set(obj, "★");
    else
      elm_object_text_set(obj, "☆");

    favorites_set(ctx->ad, st, st->favorite);
    favorites_save(ctx->ad);

    // Ensure the list item doesn't get selected when clicking the favorite button
    evas_object_propagate_events_set(obj, EINA_FALSE);

    free(ctx);
}
