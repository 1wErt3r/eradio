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

typedef struct _AppData
{
   Evas_Object *win;
   Evas_Object *list;
   Evas_Object *emotion;
   Evas_Object *search_entry;
   Evas_Object *search_hoversel;
   Evas_Object *play_pause_btn;
   Evas_Object *stop_btn;
   Evas_Object *search_btn;
   Eina_List *stations;
   Eina_Bool playing;
   Eina_Hash *favorites;
   Eina_List *favorites_stations;
   Evas_Object *favorites_btn;
} AppData;
