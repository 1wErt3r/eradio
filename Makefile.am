SUBDIRS = src data

EXTRA_DIST = LICENSE README.md

# Clean distribution from vendor dependencies
dist-hook:
	@echo "Cleaning distribution..."
	rm -rf $(distdir)/src/vendor
	@echo "Distribution cleaned successfully" 
