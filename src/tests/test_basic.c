#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include Unity testing framework
#include "unity.h"

// Include the core data structures we want to test
#include "../appdata.h"

// setUp and tearDown functions (optional)
void setUp(void) {
    // This runs before each test
}

void tearDown(void) {
    // This runs after each test
}

// Test functions
void test_station_creation(void) {
    Station station = {0};

    station.name = "Test Radio Station";
    station.url = "http://example.com/stream";
    station.bitrate = 128;
    station.favorite = EINA_TRUE;

    TEST_ASSERT_NOT_NULL(station.name);
    TEST_ASSERT_EQUAL_STRING("Test Radio Station", station.name);
    TEST_ASSERT_EQUAL(128, station.bitrate);
    TEST_ASSERT_EQUAL(EINA_TRUE, station.favorite);
}

void test_view_mode_enum(void) {
    ViewMode search_mode = VIEW_SEARCH;
    ViewMode favorites_mode = VIEW_FAVORITES;

    TEST_ASSERT_EQUAL(0, search_mode);
    TEST_ASSERT_EQUAL(1, favorites_mode);
    TEST_ASSERT_NOT_EQUAL(search_mode, favorites_mode);
}

void test_appdata_structure(void) {
    AppData appdata = {0};

    // Initialize some basic fields
    appdata.playing = EINA_FALSE;
    appdata.filters_visible = EINA_TRUE;
    appdata.view_mode = VIEW_SEARCH;
    appdata.loading_requests = 5;

    TEST_ASSERT_EQUAL(EINA_FALSE, appdata.playing);
    TEST_ASSERT_EQUAL(EINA_TRUE, appdata.filters_visible);
    TEST_ASSERT_EQUAL(VIEW_SEARCH, appdata.view_mode);
    TEST_ASSERT_EQUAL(5, appdata.loading_requests);
}

void test_station_string_fields(void) {
    Station station = {0};

    station.name = "Jazz FM";
    station.url = "http://jazz.fm/stream.mp3";
    station.favicon = "http://jazz.fm/icon.png";
    station.stationuuid = "1234-5678-91011";
    station.country = "USA";
    station.language = "English";
    station.codec = "MP3";
    station.tags = "jazz,music";

    TEST_ASSERT_EQUAL_STRING("Jazz FM", station.name);
    TEST_ASSERT_EQUAL_STRING("http://jazz.fm/stream.mp3", station.url);
    TEST_ASSERT_EQUAL_STRING("USA", station.country);
    TEST_ASSERT_EQUAL_STRING("English", station.language);
    TEST_ASSERT_EQUAL_STRING("MP3", station.codec);
    TEST_ASSERT_EQUAL_STRING("jazz,music", station.tags);
}

// Main test runner
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_station_creation);
    RUN_TEST(test_view_mode_enum);
    RUN_TEST(test_appdata_structure);
    RUN_TEST(test_station_string_fields);

    return UNITY_END();
}