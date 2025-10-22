bin_PROGRAMS = eradio

eradio_SOURCES = main.c ui.c radio_player.c station_list.c http.c favorites.c visualizer.c \
                 appdata.h ui.h radio_player.h station_list.h http.h favorites.h visualizer.h

eradio_CFLAGS = $(EFL_CFLAGS) $(LIBXML_CFLAGS)
eradio_LDADD = $(EFL_LIBS) $(LIBXML_LIBS)

# Test program with Unity framework
noinst_PROGRAMS = test_basic

# Unity framework sources
UNITY_SRC = $(top_srcdir)/vendor/unity/src/unity.c
UNITY_HDR = $(top_srcdir)/vendor/unity/src/unity.h

test_basic_SOURCES = tests/test_basic.c appdata.h $(UNITY_SRC)
test_basic_CFLAGS = $(EFL_CFLAGS) -I$(top_srcdir)/vendor/unity/src
test_basic_LDADD = $(EFL_LIBS_NO_CON)