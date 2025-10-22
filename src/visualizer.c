#include "visualizer.h"
#include "ui.h"

static void _visualizer_win_del_cb(void *data, Evas_Object *obj, void *event_info);
static void _visualizer_title_changed_cb(void *data, Evas_Object *obj, void *event_info);

void
visualizer_init(AppData *ad)
{
   ad->visualizer_win = NULL;
   ad->visualizer_emotion = NULL;
   ad->visualizer_active = EINA_FALSE;
}

static void
_visualizer_win_del_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   visualizer_hide(ad);
}

static void
_visualizer_title_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   const char *title = emotion_object_title_get(obj);

   if (ad->visualizer_win && title && strlen(title) > 0)
     {
        char window_title[256];
        snprintf(window_title, sizeof(window_title), "eradio Visualizer - %s", title);
        elm_win_title_set(ad->visualizer_win, window_title);
     }
}

void
visualizer_show(AppData *ad)
{
   if (ad->visualizer_active)
     return;

   // Create visualizer window
   ad->visualizer_win = elm_win_add(NULL, "eradio_visualizer", ELM_WIN_BASIC);
   elm_win_title_set(ad->visualizer_win, "eradio Visualizer");
   elm_win_autodel_set(ad->visualizer_win, EINA_TRUE);
   evas_object_smart_callback_add(ad->visualizer_win, "delete,request", _visualizer_win_del_cb, ad);

   // Set a reasonable default size
   evas_object_resize(ad->visualizer_win, 640, 480);

   // Create emotion object for visualization
   ad->visualizer_emotion = emotion_object_add(ad->visualizer_win);

   // Set GOOM visualization
   emotion_object_vis_set(ad->visualizer_emotion, EMOTION_VIS_GOOM);

   // Set initial volume to match the current volume slider setting
   if (ad->volume_slider) {
       double current_volume = elm_slider_value_get(ad->volume_slider);
       emotion_object_audio_volume_set(ad->visualizer_emotion, current_volume);
   }

   // Add title change callback for window title updates
   evas_object_smart_callback_add(ad->visualizer_emotion, "title_change", _visualizer_title_changed_cb, ad);

   // Make emotion object fill the window
   evas_object_size_hint_weight_set(ad->visualizer_emotion, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad->visualizer_emotion, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(ad->visualizer_win, ad->visualizer_emotion);
   evas_object_show(ad->visualizer_emotion);

   // If there's already a station playing, connect the visualizer to it
   // BUT pause the main player to prevent dual audio
   if (ad->playing && ad->emotion)
     {
        const char *current_url = emotion_object_file_get(ad->emotion);
        if (current_url)
          {
             // Pause the main player to prevent dual audio
             emotion_object_play_set(ad->emotion, EINA_FALSE);
             // Connect visualizer to the same station
             emotion_object_file_set(ad->visualizer_emotion, current_url);
             emotion_object_play_set(ad->visualizer_emotion, EINA_TRUE);
          }
     }

   ad->visualizer_active = EINA_TRUE;
   evas_object_show(ad->visualizer_win);
}

void
visualizer_hide(AppData *ad)
{
   if (!ad->visualizer_active)
     return;

   // Stop the visualizer player and restore main player if needed
   if (ad->visualizer_emotion)
     {
        emotion_object_play_set(ad->visualizer_emotion, EINA_FALSE);
        evas_object_del(ad->visualizer_emotion);
        ad->visualizer_emotion = NULL;
     }

   // If we were playing before showing visualizer, restore main player
   if (ad->playing && ad->emotion)
     {
        emotion_object_play_set(ad->emotion, EINA_TRUE);
     }

   if (ad->visualizer_win)
     {
        evas_object_del(ad->visualizer_win);
        ad->visualizer_win = NULL;
     }

   ad->visualizer_active = EINA_FALSE;
}

void
visualizer_toggle(AppData *ad)
{
   if (ad->visualizer_active)
     visualizer_hide(ad);
   else
     visualizer_show(ad);
}

void
visualizer_set_station(AppData *ad, const char *url)
{
   if (!ad->visualizer_active || !ad->visualizer_emotion || !url)
     return;

   emotion_object_file_set(ad->visualizer_emotion, url);
}

void
visualizer_play(AppData *ad)
{
   if (!ad->visualizer_active || !ad->visualizer_emotion)
     return;

   // Pause main player and start visualizer player
   if (ad->emotion)
     emotion_object_play_set(ad->emotion, EINA_FALSE);
   emotion_object_play_set(ad->visualizer_emotion, EINA_TRUE);
}

void
visualizer_pause(AppData *ad)
{
   if (!ad->visualizer_active || !ad->visualizer_emotion)
     return;

   emotion_object_play_set(ad->visualizer_emotion, EINA_FALSE);
   // Resume main player if it was playing
   if (ad->playing && ad->emotion)
     emotion_object_play_set(ad->emotion, EINA_TRUE);
}

void
visualizer_stop(AppData *ad)
{
   if (!ad->visualizer_active || !ad->visualizer_emotion)
     return;

   emotion_object_play_set(ad->visualizer_emotion, EINA_FALSE);
   emotion_object_position_set(ad->visualizer_emotion, 0.0);
   // Only resume main player if we're not in a global stop operation
   if (ad->playing && ad->emotion)
     emotion_object_play_set(ad->emotion, EINA_TRUE);
}

void
visualizer_shutdown(AppData *ad)
{
   visualizer_hide(ad);
}