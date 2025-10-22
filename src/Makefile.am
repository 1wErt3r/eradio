bin_PROGRAMS = eradio

eradio_SOURCES = main.c ui.c radio_player.c station_list.c http.c favorites.c visualizer.c \
                 appdata.h ui.h radio_player.h station_list.h http.h favorites.h visualizer.h

eradio_CFLAGS = $(EFL_CFLAGS) $(LIBXML_CFLAGS)
eradio_LDADD = $(EFL_LIBS) $(LIBXML_LIBS)

# Tests are only built in development, not in distribution
# The test files are excluded from the distribution package

# Exclude test files and vendor directory from distribution
nodist_noinst_HEADERS =

# Don't include test or vendor files in distribution
EXTRA_DIST =

# Ensure tests and vendor are never included in distribution
dist_noinst_DATA =

# Clean any remaining test/vendor files
dist-hook:
	@echo "Cleaning distribution..."
	rm -rf $(distdir)/tests
	rm -rf $(distdir)/vendor
	@echo "Distribution cleaned successfully"