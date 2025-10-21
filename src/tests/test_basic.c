#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the core data structures we want to test
#include "../appdata.h"

// Simple test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s\n", message); \
            return 0; \
        } else { \
            printf("PASS: %s\n", message); \
        } \
    } while(0)

// Test functions
int test_station_creation() {
    Station station = {0};

    station.name = "Test Radio Station";
    station.url = "http://example.com/stream";
    station.bitrate = 128;
    station.favorite = EINA_TRUE;

    TEST_ASSERT(station.name != NULL, "Station name should be set");
    TEST_ASSERT(strcmp(station.name, "Test Radio Station") == 0, "Station name should match");
    TEST_ASSERT(station.bitrate == 128, "Station bitrate should be 128");
    TEST_ASSERT(station.favorite == EINA_TRUE, "Station favorite should be true");

    return 1;
}

int test_view_mode_enum() {
    ViewMode search_mode = VIEW_SEARCH;
    ViewMode favorites_mode = VIEW_FAVORITES;

    TEST_ASSERT(search_mode == 0, "VIEW_SEARCH should be 0");
    TEST_ASSERT(favorites_mode == 1, "VIEW_FAVORITES should be 1");
    TEST_ASSERT(search_mode != favorites_mode, "View modes should be different");

    return 1;
}

int test_appdata_structure() {
    AppData appdata = {0};

    // Initialize some basic fields
    appdata.playing = EINA_FALSE;
    appdata.filters_visible = EINA_TRUE;
    appdata.view_mode = VIEW_SEARCH;
    appdata.loading_requests = 5;

    TEST_ASSERT(appdata.playing == EINA_FALSE, "Playing should be false initially");
    TEST_ASSERT(appdata.filters_visible == EINA_TRUE, "Filters visible should be true");
    TEST_ASSERT(appdata.view_mode == VIEW_SEARCH, "View mode should be SEARCH");
    TEST_ASSERT(appdata.loading_requests == 5, "Loading requests should be 5");

    return 1;
}

int test_station_string_fields() {
    Station station = {0};

    station.name = "Jazz FM";
    station.url = "http://jazz.fm/stream.mp3";
    station.favicon = "http://jazz.fm/icon.png";
    station.stationuuid = "1234-5678-91011";
    station.country = "USA";
    station.language = "English";
    station.codec = "MP3";
    station.tags = "jazz,music";

    TEST_ASSERT(strcmp(station.name, "Jazz FM") == 0, "Station name should be Jazz FM");
    TEST_ASSERT(strcmp(station.url, "http://jazz.fm/stream.mp3") == 0, "Station URL should match");
    TEST_ASSERT(strcmp(station.country, "USA") == 0, "Station country should be USA");
    TEST_ASSERT(strcmp(station.language, "English") == 0, "Station language should be English");
    TEST_ASSERT(strcmp(station.codec, "MP3") == 0, "Station codec should be MP3");
    TEST_ASSERT(strcmp(station.tags, "jazz,music") == 0, "Station tags should match");

    return 1;
}

// Main test runner
int main() {
    printf("Running eradio basic tests...\n");
    printf("================================\n");

    int passed = 0;
    int total = 0;

    // Run tests
    total++; passed += test_station_creation();
    total++; passed += test_view_mode_enum();
    total++; passed += test_appdata_structure();
    total++; passed += test_station_string_fields();

    printf("================================\n");
    printf("Test Results: %d/%d tests passed\n", passed, total);

    if (passed == total) {
        printf("All tests PASSED!\n");
        return EXIT_SUCCESS;
    } else {
        printf("Some tests FAILED!\n");
        return EXIT_FAILURE;
    }
}