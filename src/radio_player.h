#pragma once

#include "appdata.h"

void radio_player_init(AppData *ad);
void radio_player_shutdown(void);
void radio_player_play(AppData *ad, const char *url);
void radio_player_stop(AppData *ad);
void radio_player_toggle_pause(AppData *ad);
