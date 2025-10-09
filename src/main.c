// Minimal EFL (Elementary) "Hello, World!" application
//
// This file serves as a clean, well-commented starting point for
// EFL-based desktop apps. It demonstrates:
// - Elementary initialization and shutdown
// - Creating a standard window
// - Adding background and a centered label
// - Entering the main event loop
//
// Build & run:
//   make
//   make run

#include <Elementary.h>

// Entry point using Elementary’s macro wrapper. This ensures proper
// initialization and clean shutdown, and keeps main() concise.
EAPI_MAIN int
elm_main(int argc, char **argv)
{
    // Create a standard window with title and autodel behavior
    Evas_Object *win = elm_win_util_standard_add("hello", "EFL Hello World");
    elm_win_autodel_set(win, EINA_TRUE);

    // Optional: a background to occupy the full window
    Evas_Object *bg = elm_bg_add(win);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, bg);
    evas_object_show(bg);

    // Create and configure a label centered in the window
    Evas_Object *label = elm_label_add(win);
    elm_object_text_set(label, "Hello, World!");
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, 0.5, 0.5);
    elm_win_resize_object_add(win, label);
    evas_object_show(label);

    // Initial window size and show
    evas_object_resize(win, 400, 240);
    evas_object_show(win);

    // Enter the main loop
    elm_run();
    return 0;
}

ELM_MAIN()