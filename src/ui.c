#include "ui.h"
#include "appdata.h"
#include "favorites.h"
#include "station_list.h"

static void _win_del_cb(void *data, Evas_Object *obj, void *event_info);
static void _app_exit_cb(void *data, Evas_Object *obj, void *event_info);
static void _hoversel_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _tb_favorites_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _tb_search_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _server_item_selected_cb(void *data, Evas_Object *obj, void *event_info);

// Forward declarations for callbacks
void _play_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
void _stop_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
void _search_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
void _search_entry_activated_cb(void *data, Evas_Object *obj, void *event_info);
void _list_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _favorites_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);


static void
_win_del_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_exit();
}

static void
_app_exit_cb(void *data, Evas_Object *obj, void *event_info)
{
    Ecore_Evas *ee = data;
    if (ee) ecore_evas_free(ee);
}

static void
_hoversel_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it = event_info;
    const char *label = elm_object_item_text_get(it);
    elm_object_text_set(obj, label);
}

void
ui_create(AppData *ad)
{
   Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 0, 0, NULL);
   if (ee)
     {
        int dpi = 0;
        ecore_evas_screen_dpi_get(ee, NULL, &dpi);
        if (dpi >= 192)
          elm_config_scale_set(4.0);
     }

   ad->win = elm_win_add(NULL, "eradio", ELM_WIN_BASIC);
   elm_win_title_set(ad->win, "eradio");
   elm_win_autodel_set(ad->win, EINA_TRUE);
   evas_object_smart_callback_add(ad->win, "delete,request", _win_del_cb, ad);
   if (ee) evas_object_smart_callback_add(ad->win, "delete,request", _app_exit_cb, ee);

   Evas_Object *box = elm_box_add(ad->win);
   elm_box_padding_set(box, 0, 10);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(ad->win, box);
   evas_object_show(box);

   /* Toolbar at the top */
   Evas_Object *toolbar = elm_toolbar_add(ad->win);
   elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_MENU);
   elm_toolbar_homogeneous_set(toolbar, EINA_TRUE);
   elm_object_style_set(toolbar, "transparent");
   evas_object_size_hint_weight_set(toolbar, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(toolbar, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, toolbar);
   evas_object_show(toolbar);

   /* Toolbar items */
   elm_toolbar_item_append(toolbar, "system-search", "Search", _tb_search_clicked_cb, ad);
   elm_toolbar_item_append(toolbar, "emblem-favorite", "Favorites", _tb_favorites_clicked_cb, ad);

   ad->search_bar = elm_box_add(ad->win);
   elm_box_horizontal_set(ad->search_bar, EINA_TRUE);
   elm_box_padding_set(ad->search_bar, 10, 0);
   evas_object_size_hint_weight_set(ad->search_bar, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(ad->search_bar, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, ad->search_bar);
   evas_object_show(ad->search_bar);

   ad->search_entry = elm_entry_add(ad->win);
   elm_entry_single_line_set(ad->search_entry, EINA_TRUE);
   elm_entry_scrollable_set(ad->search_entry, EINA_TRUE);
   evas_object_size_hint_weight_set(ad->search_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad->search_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_text_set(ad->search_entry, "guide", "Search for stations...");
   elm_box_pack_end(ad->search_bar, ad->search_entry);
   evas_object_show(ad->search_entry);
   elm_object_focus_set(ad->search_entry, EINA_TRUE);

   ad->search_hoversel = elm_hoversel_add(ad->win);
   elm_hoversel_hover_parent_set(ad->search_hoversel, ad->win);
   elm_object_text_set(ad->search_hoversel, "name");
   elm_hoversel_item_add(ad->search_hoversel, "name", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->search_hoversel, "country", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->search_hoversel, "language", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->search_hoversel, "tag", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_box_pack_end(ad->search_bar, ad->search_hoversel);
   evas_object_show(ad->search_hoversel);

   // Server selection hoversel, populated after HTTP init discovers servers
   ad->server_hoversel = elm_hoversel_add(ad->win);
   elm_hoversel_hover_parent_set(ad->server_hoversel, ad->win);
   elm_object_text_set(ad->server_hoversel, "server");
   elm_box_pack_end(ad->search_bar, ad->server_hoversel);
   evas_object_show(ad->server_hoversel);

   ad->search_btn = elm_button_add(ad->win);
   elm_object_text_set(ad->search_btn, "Search");
   elm_box_pack_end(ad->search_bar, ad->search_btn);
   evas_object_show(ad->search_btn);

   /* Removed redundant Favorites button from search bar; toolbar handles view switching */

   ad->list = elm_list_add(ad->win);
   evas_object_size_hint_weight_set(ad->list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad->list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ad->list);
   evas_object_show(ad->list);

   Evas_Object *controls_hbox = elm_box_add(ad->win);
   elm_box_horizontal_set(controls_hbox, EINA_TRUE);
   elm_box_padding_set(controls_hbox, 10, 0);
   elm_box_align_set(controls_hbox, 0.5, 1.0);
   evas_object_size_hint_weight_set(controls_hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(controls_hbox, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, controls_hbox);
   evas_object_show(controls_hbox);

   Evas_Object *ic;

   ad->play_pause_btn = elm_button_add(ad->win);
   ic = elm_icon_add(ad->play_pause_btn);
   elm_icon_standard_set(ic, "media-playback-start");
   evas_object_size_hint_min_set(ic, 40, 40);
   elm_object_part_content_set(ad->play_pause_btn, "icon", ic);
   elm_box_pack_end(controls_hbox, ad->play_pause_btn);
   evas_object_show(ad->play_pause_btn);

   ad->stop_btn = elm_button_add(ad->win);
   ic = elm_icon_add(ad->stop_btn);
   elm_icon_standard_set(ic, "media-playback-stop");
   evas_object_size_hint_min_set(ic, 40, 40);
   elm_object_part_content_set(ad->stop_btn, "icon", ic);
   elm_box_pack_end(controls_hbox, ad->stop_btn);
   evas_object_show(ad->stop_btn);

   evas_object_smart_callback_add(ad->play_pause_btn, "clicked", _play_pause_btn_clicked_cb, ad);
   evas_object_smart_callback_add(ad->stop_btn, "clicked", _stop_btn_clicked_cb, ad);
   evas_object_smart_callback_add(ad->search_btn, "clicked", _search_btn_clicked_cb, ad);
   evas_object_smart_callback_add(ad->search_entry, "activated", _search_entry_activated_cb, ad);
   evas_object_smart_callback_add(ad->list, "selected", _list_item_selected_cb, ad);
   /* Favorites view button removed; toolbar callbacks handle switching */

   /* Default to Search view on startup */
   ad->view_mode = VIEW_SEARCH;
   evas_object_show(ad->search_bar);
   if (ad->stations)
     station_list_populate(ad, ad->stations);
   else
     station_list_clear(ad);

   evas_object_resize(ad->win, 400, 600);
   evas_object_show(ad->win);
}

void ui_update_server_list(AppData *ad)
{
   if (!ad || !ad->server_hoversel) return;
   Eina_List *l; const char *host;
   EINA_LIST_FOREACH(ad->api_servers, l, host)
   {
      elm_hoversel_item_add(ad->server_hoversel, host, NULL, ELM_ICON_NONE, _server_item_selected_cb, ad);
   }
   if (ad->api_selected && ad->api_selected[0])
      elm_object_text_set(ad->server_hoversel, ad->api_selected);
}

static void _server_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   if (!ad) return;
   Elm_Object_Item *it = event_info;
   const char *label = elm_object_item_text_get(it);
   if (label && label[0])
   {
      elm_object_text_set(obj, label);
      if (ad->api_selected) eina_stringshare_del(ad->api_selected);
      ad->api_selected = eina_stringshare_add(label);
   }
}

static void
_tb_favorites_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   AppData *ad = data;
   ad->view_mode = VIEW_FAVORITES;
   evas_object_hide(ad->search_bar);
   favorites_rebuild_station_list(ad);
   station_list_populate(ad, ad->favorites_stations);
}

static void
_tb_search_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   AppData *ad = data;
   ad->view_mode = VIEW_SEARCH;
   evas_object_show(ad->search_bar);
   if (ad->stations)
     station_list_populate(ad, ad->stations);
   else
     station_list_clear(ad);
}
