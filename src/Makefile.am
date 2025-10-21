bin_PROGRAMS = eradio

eradio_SOURCES = main.c ui.c radio_player.c station_list.c http.c favorites.c \
                 appdata.h ui.h radio_player.h station_list.h http.h favorites.h

eradio_CFLAGS = $(EFL_CFLAGS) $(LIBXML_CFLAGS)
eradio_LDADD = $(EFL_LIBS) $(LIBXML_LIBS)

# Test program
noinst_PROGRAMS = test_basic

test_basic_SOURCES = tests/test_basic.c appdata.h
test_basic_CFLAGS = $(EFL_CFLAGS)
test_basic_LDADD = $(EFL_LIBS_NO_CON)