#include "radio_player.h"
#include "ui.h"
#include "visualizer.h"

static Ecore_Timer *stream_error_timer = NULL;
static Ecore_Timer *audio_progress_timer = NULL;
static double last_position = 0.0;
static const char *current_station_name = NULL;

static void
_title_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   const char *title = emotion_object_title_get(obj);

   if (title && strlen(title) > 0)
     {
        printf("Station metadata: %s\n", title);
        elm_object_text_set(ad->statusbar, title);
     }
   else if (current_station_name)
     {
        printf("No metadata, showing station name: %s\n", current_station_name);
        elm_object_text_set(ad->statusbar, current_station_name);
     }
}

static void
_playback_error_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   printf("Playback error detected\n");
   ui_show_error_dialog(ad, "Unable to stream this station");
}

static void
_decode_error_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   printf("Decode error detected\n");
   ui_show_error_dialog(ad, "Unable to stream this station");
}

static Eina_Bool
_audio_progress_cb(void *data)
{
   AppData *ad = data;

   if (!ad->playing || !ad->emotion)
     {
        audio_progress_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   double current_position = emotion_object_position_get(ad->emotion);

   // If position has advanced from 0, we have audio playback!
   if (current_position > 0.0 || current_position != last_position)
     {
        printf("Audio playback detected - canceling error timers\n");

        // Cancel both timers since audio is playing
        if (stream_error_timer)
          {
             ecore_timer_del(stream_error_timer);
             stream_error_timer = NULL;
          }

        audio_progress_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   last_position = current_position;
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_stream_timeout_cb(void *data)
{
   AppData *ad = data;
   printf("Stream timeout detected - no audio playback\n");

   // Check if we're still "playing" but no actual audio progress is being made
   if (ad->playing && ad->emotion)
     {
        double position = emotion_object_position_get(ad->emotion);

        // If position is still 0 after timeout, likely no audio is being decoded
        if (position == 0.0)
          {
             printf("No audio playback detected, showing error\n");
             ui_show_error_dialog(ad, "Unable to stream this station");
             radio_player_stop(ad);
          }
     }

   stream_error_timer = NULL;

   // Also cancel the audio progress timer
   if (audio_progress_timer)
     {
        ecore_timer_del(audio_progress_timer);
        audio_progress_timer = NULL;
     }

   return ECORE_CALLBACK_CANCEL;
}

void
radio_player_init(AppData *ad)
{
   ad->emotion = emotion_object_add(ad->win);
   evas_object_smart_callback_add(ad->emotion, "title_change", _title_changed_cb, ad);
   evas_object_smart_callback_add(ad->emotion, "playback_error", _playback_error_cb, ad);
   evas_object_smart_callback_add(ad->emotion, "decode_error", _decode_error_cb, ad);

   // Set initial volume
   emotion_object_audio_volume_set(ad->emotion, 0.7);
}

void
radio_player_shutdown(void)
{
   // Nothing to do here yet
}

void
radio_player_play(AppData *ad, const char *url, const char *station_name)
{
   fprintf(stderr, "LOG: radio_player_play: ad=%p, url=%s, station=%s\n", ad, url, station_name ? station_name : "(unknown)");
   if (url && url[0])
     {
        // Store the current station name
        if (current_station_name)
          eina_stringshare_del(current_station_name);
        current_station_name = station_name ? eina_stringshare_add(station_name) : NULL;

        // Cancel any existing timers
        if (stream_error_timer)
          {
             ecore_timer_del(stream_error_timer);
             stream_error_timer = NULL;
          }
        if (audio_progress_timer)
          {
             ecore_timer_del(audio_progress_timer);
             audio_progress_timer = NULL;
          }

        emotion_object_file_set(ad->emotion, url);
        emotion_object_play_set(ad->emotion, EINA_TRUE);
        ad->playing = EINA_TRUE;

        // Start visualizer playback if active
        visualizer_play(ad);
        if (ad->play_pause_item)
          elm_toolbar_item_icon_set(ad->play_pause_item, "media-playback-pause");

        // Show station name initially
        if (current_station_name)
          elm_object_text_set(ad->statusbar, current_station_name);

        // Update visualizer with new station
        visualizer_set_station(ad, url);

        // Reset position tracking
        last_position = 0.0;

        // Start timers to detect streaming failures
        stream_error_timer = ecore_timer_add(10.0, _stream_timeout_cb, ad);
        audio_progress_timer = ecore_timer_add(0.5, _audio_progress_cb, ad);
     }
}

void
radio_player_stop(AppData *ad)
{
   emotion_object_play_set(ad->emotion, EINA_FALSE);
   emotion_object_position_set(ad->emotion, 0.0);
   ad->playing = EINA_FALSE;

   // Stop visualizer if active
   visualizer_stop(ad);

   // Cancel all timers
   if (stream_error_timer)
     {
        ecore_timer_del(stream_error_timer);
        stream_error_timer = NULL;
     }
   if (audio_progress_timer)
     {
        ecore_timer_del(audio_progress_timer);
        audio_progress_timer = NULL;
     }

   // Clean up station name
   if (current_station_name)
     {
        eina_stringshare_del(current_station_name);
        current_station_name = NULL;
     }

   if (ad->play_pause_item)
     elm_toolbar_item_icon_set(ad->play_pause_item, "media-playback-start");
   if (ad->statusbar)
     elm_object_text_set(ad->statusbar, " ");
}

void
radio_player_toggle_pause(AppData *ad)
{
   ad->playing = !ad->playing;
   emotion_object_play_set(ad->emotion, ad->playing);

   // Sync visualizer with playback state
   if (ad->playing)
     visualizer_play(ad);
   else
     visualizer_pause(ad);

   if (ad->play_pause_item)
     {
        if (ad->playing)
          elm_toolbar_item_icon_set(ad->play_pause_item, "media-playback-pause");
        else
          elm_toolbar_item_icon_set(ad->play_pause_item, "media-playback-start");
     }
}

void
_play_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   radio_player_toggle_pause(ad);
}

void
_stop_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   radio_player_stop(ad);
}

void
_visualizer_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   visualizer_toggle(ad);
}
