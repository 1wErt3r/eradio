#include "appdata.h"
#include "ui.h"
#include "radio_player.h"
#include "http.h"
#include "favorites.h"

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   AppData ad = {0};

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   ui_create(&ad);
   favorites_init(&ad);
   favorites_load(&ad);
   http_init(&ad);
   radio_player_init(&ad);

   elm_run();

   http_shutdown();
   radio_player_shutdown();
   favorites_shutdown(&ad);

   return 0;
}
ELM_MAIN()