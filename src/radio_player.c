#include "radio_player.h"

static void
_title_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   const char *title = emotion_object_title_get(obj);
   if (title)
     {
        printf("Station metadata: %s\n", title);
        elm_object_text_set(ad->statusbar, title);
     }
}

void
radio_player_init(AppData *ad)
{
   ad->emotion = emotion_object_add(ad->win);
   evas_object_smart_callback_add(ad->emotion, "title_change", _title_changed_cb, ad);
}

void
radio_player_shutdown(void)
{
   // Nothing to do here yet
}

void
radio_player_play(AppData *ad, const char *url)
{
   fprintf(stderr, "LOG: radio_player_play: ad=%p, url=%s\n", ad, url);
   if (url && url[0])
     {
        emotion_object_file_set(ad->emotion, url);
        emotion_object_play_set(ad->emotion, EINA_TRUE);
        ad->playing = EINA_TRUE;
        if (ad->play_pause_item)
          elm_toolbar_item_icon_set(ad->play_pause_item, "media-playback-pause");
     }
}

void
radio_player_stop(AppData *ad)
{
   emotion_object_play_set(ad->emotion, EINA_FALSE);
   emotion_object_position_set(ad->emotion, 0.0);
   ad->playing = EINA_FALSE;
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
