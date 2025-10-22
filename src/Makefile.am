bin_PROGRAMS = eradio

eradio_SOURCES = main.c ui.c radio_player.c station_list.c http.c favorites.c visualizer.c \
                 appdata.h ui.h radio_player.h station_list.h http.h favorites.h visualizer.h

eradio_CFLAGS = $(EFL_CFLAGS) $(LIBXML_CFLAGS)
eradio_LDADD = $(EFL_LIBS) $(LIBXML_LIBS)

# Exclude vendor directory from distribution
nodist_noinst_HEADERS =

# Don't include vendor files in distribution
EXTRA_DIST =

# Ensure vendor is never included in distribution
dist_noinst_DATA =

# Clean any remaining vendor files
dist-hook:
	@echo "Cleaning distribution..."
	rm -rf $(distdir)/vendor
	@echo "Distribution cleaned successfully"