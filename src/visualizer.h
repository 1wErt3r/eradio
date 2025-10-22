#pragma once

#include "appdata.h"

void visualizer_init(AppData *ad);
void visualizer_show(AppData *ad);
void visualizer_hide(AppData *ad);
void visualizer_toggle(AppData *ad);
void visualizer_set_station(AppData *ad, const char *url);
void visualizer_play(AppData *ad);
void visualizer_pause(AppData *ad);
void visualizer_stop(AppData *ad);
void visualizer_shutdown(AppData *ad);