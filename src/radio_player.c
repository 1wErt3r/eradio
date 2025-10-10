#include "radio_player.h"

void
radio_player_init(AppData *ad)
{
   ad->emotion = emotion_object_add(ad->win);
}

void
radio_player_shutdown(void)
{
   // Nothing to do here yet
}

void
radio_player_play(AppData *ad, const char *url)
{
   if (url && url[0])
     {
        emotion_object_file_set(ad->emotion, url);
        emotion_object_play_set(ad->emotion, EINA_TRUE);
        ad->playing = EINA_TRUE;
        Evas_Object *ic = elm_object_part_content_get(ad->play_pause_btn, "icon");
        elm_icon_standard_set(ic, "media-playback-pause");
     }
}

void
radio_player_stop(AppData *ad)
{
   emotion_object_play_set(ad->emotion, EINA_FALSE);
   emotion_object_position_set(ad->emotion, 0.0);
   ad->playing = EINA_FALSE;
   Evas_Object *ic = elm_object_part_content_get(ad->play_pause_btn, "icon");
   elm_icon_standard_set(ic, "media-playback-start");
}

void
radio_player_toggle_pause(AppData *ad)
{
   ad->playing = !ad->playing;
   emotion_object_play_set(ad->emotion, ad->playing);

   Evas_Object *ic = elm_object_part_content_get(ad->play_pause_btn, "icon");
   if (ad->playing)
     elm_icon_standard_set(ic, "media-playback-pause");
   else
     elm_icon_standard_set(ic, "media-playback-start");
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
