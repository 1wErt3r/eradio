#include "ui.h"
#include "appdata.h"
#include "favorites.h"
#include "station_list.h"
#include "http.h" // Include http.h for http_search_stations

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
static void _load_more_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);


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
   elm_box_padding_set(box, 10, 10); // Add padding to the main box
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
   elm_box_padding_set(ad->search_bar, 10, 10);
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

   // Search options in a separate box
   Evas_Object *search_options_box = elm_box_add(ad->win);
   elm_box_horizontal_set(search_options_box, EINA_TRUE);
   elm_box_padding_set(search_options_box, 10, 0);
   elm_box_pack_end(ad->search_bar, search_options_box);
   evas_object_show(search_options_box);

   Evas_Object *lbl;
   lbl = elm_label_add(ad->win);
   elm_object_text_set(lbl, "Search by:");
   elm_box_pack_end(search_options_box, lbl);
   evas_object_show(lbl);

   ad->search_hoversel = elm_hoversel_add(ad->win);
   elm_hoversel_hover_parent_set(ad->search_hoversel, ad->win);
   elm_object_text_set(ad->search_hoversel, "name");
   elm_hoversel_item_add(ad->search_hoversel, "name", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->search_hoversel, "country", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->search_hoversel, "language", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->search_hoversel, "tag", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_box_pack_end(search_options_box, ad->search_hoversel);
   evas_object_show(ad->search_hoversel);

   lbl = elm_label_add(ad->win);
   elm_object_text_set(lbl, "Sort by:");
   elm_box_pack_end(search_options_box, lbl);
   evas_object_show(lbl);

   ad->sort_hoversel = elm_hoversel_add(ad->win);
   elm_hoversel_hover_parent_set(ad->sort_hoversel, ad->win);
   elm_object_text_set(ad->sort_hoversel, "name");
   elm_hoversel_item_add(ad->sort_hoversel, "name", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "url", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "homepage", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "favicon", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "tags", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "country", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "state", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "language", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "votes", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "codec", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "bitrate", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "lastcheckok", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "lastchecktime", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "clicktimestamp", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "clickcount", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "clicktrend", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "changetimestamp", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad->sort_hoversel, "random", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_box_pack_end(search_options_box, ad->sort_hoversel);
   evas_object_show(ad->sort_hoversel);

   ad->reverse_check = elm_check_add(ad->win);
   elm_object_text_set(ad->reverse_check, "Reverse");
   elm_box_pack_end(search_options_box, ad->reverse_check);
   evas_object_show(ad->reverse_check);

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

   /* Separator */
   Evas_Object *sep = elm_separator_add(ad->win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(box, sep);
   evas_object_show(sep);

   ad->list = elm_list_add(ad->win);
   evas_object_size_hint_weight_set(ad->list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad->list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ad->list);
   evas_object_show(ad->list);

   Evas_Object *controls_hbox = elm_box_add(ad->win);
   elm_box_horizontal_set(controls_hbox, EINA_TRUE);
   elm_box_padding_set(controls_hbox, 10, 0);
   elm_box_homogeneous_set(controls_hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(controls_hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(controls_hbox, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(box, controls_hbox);
   evas_object_show(controls_hbox);

   ad->statusbar = elm_label_add(ad->win);
   elm_object_text_set(ad->statusbar, "");
   elm_object_style_set(ad->statusbar, "marquee");
   evas_object_size_hint_weight_set(ad->statusbar, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(ad->statusbar, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(box, ad->statusbar);
   evas_object_show(ad->statusbar);

   Evas_Object *ic;

   ad->play_pause_btn = elm_button_add(ad->win);
   ic = elm_icon_add(ad->play_pause_btn);
   elm_icon_standard_set(ic, "media-playback-start");
   evas_object_size_hint_min_set(ic, 48, 48);
   elm_object_part_content_set(ad->play_pause_btn, "icon", ic);
   elm_box_pack_end(controls_hbox, ad->play_pause_btn);
   evas_object_show(ad->play_pause_btn);

   ad->stop_btn = elm_button_add(ad->win);
   ic = elm_icon_add(ad->stop_btn);
   elm_icon_standard_set(ic, "media-playback-stop");
   evas_object_size_hint_min_set(ic, 48, 48);
   elm_object_part_content_set(ad->stop_btn, "icon", ic);
   elm_box_pack_end(controls_hbox, ad->stop_btn);
   evas_object_show(ad->stop_btn);

   ad->load_more_btn = elm_button_add(ad->win);
   elm_object_text_set(ad->load_more_btn, "Load More");
   elm_box_pack_end(controls_hbox, ad->load_more_btn);
   evas_object_hide(ad->load_more_btn); // Hide by default

   evas_object_smart_callback_add(ad->play_pause_btn, "clicked", _play_pause_btn_clicked_cb, ad);
   evas_object_smart_callback_add(ad->stop_btn, "clicked", _stop_btn_clicked_cb, ad);
   evas_object_smart_callback_add(ad->search_btn, "clicked", _search_btn_clicked_cb, ad);
   evas_object_smart_callback_add(ad->search_entry, "activated", _search_entry_activated_cb, ad);
   evas_object_smart_callback_add(ad->list, "selected", _list_item_selected_cb, ad);
   evas_object_smart_callback_add(ad->load_more_btn, "clicked", _load_more_btn_clicked_cb, ad);

   /* Default to Search view on startup */
   ad->view_mode = VIEW_SEARCH;
   ad->search_offset = 0; // Initialize offset
   evas_object_show(ad->search_bar);
   if (ad->stations)
     station_list_populate(ad, ad->stations, EINA_TRUE);
   else
     station_list_clear(ad);

   evas_object_resize(ad->win, 480, 800);
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
   ui_set_load_more_button_visibility(ad, EINA_FALSE);
   favorites_rebuild_station_list(ad);
   station_list_populate(ad, ad->favorites_stations, EINA_TRUE);
}

static void
_tb_search_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   AppData *ad = data;
   ad->view_mode = VIEW_SEARCH;
   evas_object_show(ad->search_bar);
   ui_set_load_more_button_visibility(ad, EINA_FALSE);
   if (ad->stations)
     station_list_populate(ad, ad->stations, EINA_TRUE);
   else
     station_list_clear(ad);
}

static void
_load_more_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   AppData *ad = data;
   const char *search_term = elm_object_text_get(ad->search_entry);
   const char *search_type = elm_object_text_get(ad->search_hoversel);
   const char *order = elm_object_text_get(ad->sort_hoversel);
   Eina_Bool reverse = elm_check_state_get(ad->reverse_check);
   ad->search_offset += 100; // Increment offset for next page
   http_search_stations(ad, search_term, search_type, order, reverse, ad->search_offset, 100, EINA_FALSE);
}

void ui_set_load_more_button_visibility(AppData *ad, Eina_Bool visible)
{
    if (visible)
        evas_object_show(ad->load_more_btn);
    else
        evas_object_hide(ad->load_more_btn);
}
