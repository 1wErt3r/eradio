#pragma once

#include "appdata.h"

// Initialize favorites storage (hash) in AppData
void favorites_init(AppData *ad);

// Load favorites from XML (~/.config/eradio/favorites.xml) into AppData
void favorites_load(AppData *ad);

// Apply loaded favorites to current stations list (sets Station.favorite)
void favorites_apply_to_stations(AppData *ad);

// Save current favorites (from stations list) to XML
void favorites_save(AppData *ad);

// Shutdown and free any favorites-related resources
void favorites_shutdown(AppData *ad);

// Update favorites hash from a station toggle (add/remove)
void favorites_set(AppData *ad, Station *st, Eina_Bool on);

// Rebuild an in-memory list of favorite stations from the favorites hash
void favorites_rebuild_station_list(AppData *ad);