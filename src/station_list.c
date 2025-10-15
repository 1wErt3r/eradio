#include <Ecore_File.h>
#include "station_list.h"
#include "radio_player.h"
#include "http.h"
#include "favorites.h"
#include "ui.h"

static void _favorite_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _favorite_remove_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);

static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
    Station *st = data;
    return strdup(st->name);
}

static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
    Station *st = data;
    AppData *ad = evas_object_data_get(obj, "ad");
    if (!strcmp(part, "elm.swallow.icon"))
    {
        Evas_Object *icon = elm_icon_add(obj);
        elm_icon_standard_set(icon, "media-playback-start");
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
                // We can't easily get the list item here to pass to http_download_icon,
                // so we'll just show the default icon.
                // A more advanced implementation might trigger the download differently.
            }
        }
        return icon;
    }
    else if (!strcmp(part, "elm.swallow.end"))
    {
        if (ad->view_mode == VIEW_SEARCH)
        {
            Evas_Object *fav_btn = elm_button_add(obj);
            evas_object_size_hint_min_set(fav_btn, 40, 40);
            evas_object_propagate_events_set(fav_btn, EINA_FALSE);
            if (st->favorite)
                elm_object_text_set(fav_btn, "★");
            else
                elm_object_text_set(fav_btn, "☆");
            evas_object_smart_callback_add(fav_btn, "clicked", _favorite_btn_clicked_cb, st);
            evas_object_data_set(fav_btn, "ad", ad);
            return fav_btn;
        }
        else if (ad->view_mode == VIEW_FAVORITES)
        {
            Evas_Object *fav_btn = elm_button_add(obj);
            evas_object_size_hint_min_set(fav_btn, 60, 30);
            evas_object_propagate_events_set(fav_btn, EINA_FALSE);
            elm_object_text_set(fav_btn, "Remove");
            evas_object_smart_callback_add(fav_btn, "clicked", _favorite_remove_btn_clicked_cb, st);
            evas_object_data_set(fav_btn, "ad", ad);
            return fav_btn;
        }
    }
    return NULL;
}

static Eina_Bool
_gl_state_get(void *data, Evas_Object *obj, const char *part)
{
    return EINA_FALSE;
}

static void
_gl_del(void *data, Evas_Object *obj)
{
    // Station data is owned by the ad->stations list, so we don't free it here.
}

static Elm_Genlist_Item_Class *itc = NULL;

static void _favorite_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _favorite_remove_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);


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

static void
_favorite_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    Station *st = data;
    AppData *ad = evas_object_data_get(obj, "ad");
    st->favorite = !st->favorite;
    favorites_set(ad, st, st->favorite);
    favorites_save(ad);
    // Find the genlist item and update it
    Elm_Object_Item *it = elm_genlist_selected_item_get(ad->list);
    if (it && elm_object_item_data_get(it) == st)
        elm_genlist_item_update(it);
}

static void
_favorite_remove_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    Station *st = data;
    AppData *ad = evas_object_data_get(obj, "ad");
    st->favorite = EINA_FALSE;
    favorites_set(ad, st, EINA_FALSE);
    favorites_save(ad);
    favorites_rebuild_station_list(ad);
    station_list_populate_favorites(ad);
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
    elm_genlist_clear(ad->list);
}

void
station_list_populate(AppData *ad, Eina_Bool new_search)
{
    Eina_List *l;
    Station *st;

    if (!itc)
    {
        itc = elm_genlist_item_class_new();
        itc->item_style = "default";
        itc->func.text_get = _gl_text_get;
        itc->func.content_get = _gl_content_get;
        itc->func.state_get = _gl_state_get;
        itc->func.del = _gl_del;
    }

    if (new_search)
    {
      station_list_clear(ad);
      ad->displayed_stations_count = 0;
    }

    evas_object_data_set(ad->list, "ad", ad);
    Eina_List *stations_to_display = ad->stations;

    EINA_LIST_FOREACH(stations_to_display, l, st)
    {
        elm_genlist_item_append(ad->list,
                                itc,
                                st,
                                NULL,
                                ELM_GENLIST_ITEM_NONE,
                                _list_item_selected_cb,
                                ad);
    }
}



void
station_list_populate_favorites(AppData *ad)
{
    Eina_List *l;
    Station *st;

    if (!itc)
    {
        itc = elm_genlist_item_class_new();
        itc->item_style = "default";
        itc->func.text_get = _gl_text_get;
        itc->func.content_get = _gl_content_get;
        itc->func.state_get = _gl_state_get;
        itc->func.del = _gl_del;
    }

    station_list_clear(ad);
    evas_object_data_set(ad->list, "ad", ad);

    EINA_LIST_FOREACH(ad->favorites_stations, l, st)
    {
        elm_genlist_item_append(ad->list,
                                itc,
                                st,
                                NULL,
                                ELM_GENLIST_ITEM_NONE,
                                _list_item_selected_cb,
                                ad);
    }
}