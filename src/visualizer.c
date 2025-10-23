#include "visualizer.h"
#include "ui.h"
#include "radio_player.h"

static void _visualizer_win_del_cb(void *data, Evas_Object *obj, void *event_info);
static void _visualizer_title_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _visualizer_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _visualizer_menu_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
static void _fullscreen_toggle_cb(void *data, Evas_Object *obj, void *event_info);

void
visualizer_init(AppData *ad)
{
   ad->visualizer_win = NULL;
   ad->visualizer_emotion = NULL;
   ad->visualizer_active = EINA_FALSE;
   ad->visualizer_fullscreen = EINA_FALSE;
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
   emotion_object_vis_set(ad->visualizer_emotion, EMOTION_VIS_LIBVISUAL_DANCING_PARTICLES);

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

   // Add mouse down callback for right-click context menu
   evas_object_event_callback_add(ad->visualizer_emotion, EVAS_CALLBACK_MOUSE_DOWN, _visualizer_mouse_down_cb, ad);
 
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

   // Stop the visualizer player
   if (ad->visualizer_emotion)
     {
        emotion_object_play_set(ad->visualizer_emotion, EINA_FALSE);
        evas_object_del(ad->visualizer_emotion);
        ad->visualizer_emotion = NULL;
     }

   // Stop playback completely instead of resuming main player
   radio_player_stop(ad);

   if (ad->visualizer_win)
     {
        evas_object_del(ad->visualizer_win);
        ad->visualizer_win = NULL;
     }

   ad->visualizer_active = EINA_FALSE;
   ad->visualizer_fullscreen = EINA_FALSE; // Reset fullscreen state
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

// Mouse down callback to show context menu
static void
_visualizer_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button == 3) // Right click
     {
        Evas_Object *menu = elm_ctxpopup_add(ad->visualizer_win);
        elm_ctxpopup_auto_hide_disabled_set(menu, EINA_FALSE);
        evas_object_smart_callback_add(menu, "dismissed", _visualizer_menu_dismissed_cb, ad);

        // Add fullscreen/window mode toggle
        const char *menu_text = ad->visualizer_fullscreen ? "Window Mode" : "Fullscreen Mode";
        elm_ctxpopup_item_append(menu, menu_text, NULL, _fullscreen_toggle_cb, ad);

        // Position and show the menu
        evas_object_move(menu, ev->canvas.x, ev->canvas.y);
        evas_object_show(menu);
     }
}

// Menu dismissed callback
static void
_visualizer_menu_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del(obj);
}

// Fullscreen toggle callback
static void
_fullscreen_toggle_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;

   if (ad->visualizer_fullscreen)
     {
        // Switch to window mode
        elm_win_fullscreen_set(ad->visualizer_win, EINA_FALSE);
        ad->visualizer_fullscreen = EINA_FALSE;
        printf("Switched to window mode\n");
     }
   else
     {
        // Switch to fullscreen mode
        elm_win_fullscreen_set(ad->visualizer_win, EINA_TRUE);
        ad->visualizer_fullscreen = EINA_TRUE;
        printf("Switched to fullscreen mode\n");
     }
}
