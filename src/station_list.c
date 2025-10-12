#include <Ecore_File.h>
#include "station_list.h"
#include "radio_player.h"
#include "http.h"
#include "favorites.h"
#include "ui.h"

static void _favorite_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _favorite_remove_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);

static void
_station_click_counter_request(AppData *ad, Station *st)
{
   fprintf(stderr, "LOG: _station_click_counter_request: ad=%p, st=%p\n", ad, st);
   if (!st || !st->stationuuid) return;
   fprintf(stderr, "LOG: _station_click_counter_request: stationuuid=%s\n", st->stationuuid);
   http_station_click_counter(ad, st->stationuuid);
}

void
_list_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   Elm_Object_Item *it = event_info;
   Station *st = elm_object_item_data_get(it);

   fprintf(stderr, "LOG: _list_item_selected_cb: ad=%p, it=%p, st=%p\n", ad, it, st);

   if (!st) return;

   fprintf(stderr, "LOG: _list_item_selected_cb: station name='%s', url='%s'\n", st->name, st->url);

   _station_click_counter_request(ad, st);
   radio_player_play(ad, st->url);
}

void
station_list_clear(AppData *ad)
{
    elm_list_clear(ad->list);
}

void
station_list_populate(AppData *ad, Eina_Bool new_search)
{
    Eina_List *l;
    Station *st;
    int i = 0;

    if (new_search)
    {
      station_list_clear(ad);
      ad->displayed_stations_count = 0;
    }

    Eina_List *stations_to_display = eina_list_nth_list(ad->stations, ad->displayed_stations_count);

    EINA_LIST_FOREACH(stations_to_display, l, st)
    {
        if (i >= 100) break;

        Evas_Object *icon = elm_icon_add(ad->win);
        elm_icon_standard_set(icon, "media-playback-start");
	
        Evas_Object *fav_btn = NULL;
        if (ad->view_mode == VIEW_SEARCH)
          {
             fav_btn = elm_button_add(ad->win);
             evas_object_size_hint_min_set(fav_btn, 40, 40);
             evas_object_propagate_events_set(fav_btn, EINA_FALSE);
             if (st->favorite)
               elm_object_text_set(fav_btn, "★");
             else
               elm_object_text_set(fav_btn, "☆");
          }
        else if (ad->view_mode == VIEW_FAVORITES)
          {
             fav_btn = elm_button_add(ad->win);
             evas_object_size_hint_min_set(fav_btn, 60, 30);
             evas_object_propagate_events_set(fav_btn, EINA_FALSE);
             elm_object_text_set(fav_btn, "Remove");
          }

        Elm_Object_Item *li = elm_list_item_append(ad->list, st->name, icon, fav_btn, NULL, NULL);
        elm_object_item_data_set(li, st);

        // Attach callback only when a button exists
        if (fav_btn)
          {
             typedef struct {
                 AppData *ad;
                 Elm_Object_Item *li;
             } FavCtx;
             FavCtx *ctx = calloc(1, sizeof(FavCtx));
             ctx->ad = ad;
             ctx->li = li;
             if (ad->view_mode == VIEW_SEARCH)
               evas_object_smart_callback_add(fav_btn, "clicked", _favorite_btn_clicked_cb, ctx);
             else
               evas_object_smart_callback_add(fav_btn, "clicked", _favorite_remove_btn_clicked_cb, ctx);
          }

        if (st->favicon && st->favicon[0] && st->stationuuid)
          {
             char cache_path[PATH_MAX];
             const char *home = getenv("HOME");
             snprintf(cache_path, sizeof(cache_path), "%s/.cache/eradio/favicons/%s", home, st->stationuuid);

             if (ecore_file_exists(cache_path))
               {
                  elm_image_file_set(icon, cache_path, NULL);
               }
             else
               {
                  http_download_icon(ad, li, st->favicon);
               }
          }
        i++;
    }
    ad->displayed_stations_count += i;

    if (ad->displayed_stations_count < eina_list_count(ad->stations))
        ui_set_load_more_button_visibility(ad, EINA_TRUE);
    else
        ui_set_load_more_button_visibility(ad, EINA_FALSE);


    evas_object_smart_callback_add(ad->list, "selected", _list_item_selected_cb, ad);
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

static void
_favorite_remove_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    typedef struct {
        AppData *ad;
        Elm_Object_Item *li;
    } FavCtx;

    FavCtx *ctx = data;
    if (!ctx) return;
    Station *st = elm_object_item_data_get(ctx->li);
    if (!st) { free(ctx); return; }

    st->favorite = EINA_FALSE;
    favorites_set(ctx->ad, st, EINA_FALSE);
    favorites_save(ctx->ad);

    // Rebuild favorites list to reflect removal
    favorites_rebuild_station_list(ctx->ad);
    station_list_populate_favorites(ctx->ad);

    free(ctx);
}

void
station_list_populate_favorites(AppData *ad)
{
    Eina_List *l;
    Station *st;

    station_list_clear(ad);

    EINA_LIST_FOREACH(ad->favorites_stations, l, st)
    {
      
        Evas_Object *icon_box = elm_box_add(ad->win);
        elm_box_horizontal_set(icon_box, EINA_TRUE);
        evas_object_size_hint_min_set(icon_box, 64, 64);

        Evas_Object *icon = elm_icon_add(ad->win);
        evas_object_size_hint_min_set(icon, 64, 64);
        elm_icon_standard_set(icon, "media-playback-start");
        elm_box_pack_end(icon_box, icon);
        evas_object_show(icon);

        Evas_Object *fav_btn = elm_button_add(ad->win);
        evas_object_size_hint_min_set(fav_btn, 60, 30);
        evas_object_propagate_events_set(fav_btn, EINA_FALSE);
        elm_object_text_set(fav_btn, "Remove");

        evas_object_show(icon_box);
        Elm_Object_Item *li = elm_list_item_append(ad->list, st->name, icon_box,
        fav_btn, NULL, NULL);
        elm_object_item_data_set(li, st);

        typedef struct {
            AppData *ad;
            Elm_Object_Item *li;
        } FavCtx;
        FavCtx *ctx = calloc(1, sizeof(FavCtx));
        ctx->ad = ad;
        ctx->li = li;
        evas_object_smart_callback_add(fav_btn, "clicked", _favorite_remove_btn_clicked_cb, ctx);

        fprintf(stderr, "ICON: Station: %s, Favicon URL: %s\n", st->name, st->favicon);
        if (st->favicon && st->favicon[0] && st->stationuuid)
        {
            char cache_path[PATH_MAX];
            const char *home = getenv("HOME");
            snprintf(cache_path, sizeof(cache_path), "%s/.cache/eradio/favicons/%s", home, st->stationuuid);

            if (ecore_file_exists(cache_path))
            {
                fprintf(stderr, "ICON: Using cached icon for %s: %s\n", st->name, cache_path);
                elm_image_file_set(icon, cache_path, NULL);
            }
            else
            {
                fprintf(stderr, "ICON: Downloading icon for %s from %s\n", st->name, st->favicon);
                elm_icon_standard_set(icon, "emblem-unreadable");
                http_download_icon(ad, li, st->favicon);
            }
        }
        else
        {
            fprintf(stderr, "ICON: No favicon for %s\n", st->name);
        }
    }
    evas_object_smart_callback_add(ad->list, "selected", _list_item_selected_cb, ad);
    elm_list_go(ad->list);
}
