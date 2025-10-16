#pragma once

#include <Elementary.h>
#include <Ecore_Evas.h>
#include <Emotion.h>

typedef struct _Station
{
   const char *name;
   const char *url;
   const char *favicon;
   const char *stationuuid;
   Eina_Bool favorite;
} Station;

typedef enum {
   VIEW_SEARCH,
   VIEW_FAVORITES
} ViewMode;

typedef struct _AppData
{
   Evas_Object *win;
   Evas_Object *top_toolbar;
   Evas_Object *main_box;
   Evas_Object *list;
   Evas_Object *emotion;
   Evas_Object *search_entry;
   Evas_Object *search_hoversel;
   Evas_Object *server_hoversel;
   Evas_Object *controls_toolbar;
   Elm_Object_Item *play_pause_item;
   Elm_Object_Item *stop_item;
   Evas_Object *separator;
   Evas_Object *statusbar;
   Evas_Object *volume_slider;
   Evas_Object *search_btn;

   Evas_Object *search_bar;
   Evas_Object *filters_toggle_btn;
   Evas_Object *filters_box;
   Evas_Object *progressbar;   // loading indicator
   Evas_Object *sort_hoversel;
   Evas_Object *reverse_check;
   Eina_List *stations;
   Eina_List *api_servers;    // list of strings (hostnames)
   const char *api_selected;  // currently selected server hostname
   Eina_Bool playing;
   Eina_Bool filters_visible;
   int loading_requests;       // refcount of in-flight HTTP requests
   Eina_Hash *favorites;
   Eina_List *favorites_stations;
   ViewMode view_mode;
   int search_offset;
   int displayed_stations_count;
} AppData;
