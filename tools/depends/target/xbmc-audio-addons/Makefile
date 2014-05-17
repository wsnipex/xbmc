include ../../Makefile.include
include AUDIO-ADDONS-VERSION

LIBNAME=xbmc-audio-addons

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/.$(ADDONS):
	for addon in $(ADDONS) ; do \
          VERSION=$$(grep $${addon}-VERSION AUDIO-ADDONS-VERSION | sed 's/.*=//g'); \
          ARCHIVE=$$VERSION.tar.gz ; \
          cd $(TARBALLS_LOCATION); \
          [ -f $$addon-$$ARCHIVE ] || $(RETRIEVE_TOOL) -Ls --create-dirs -f -o $(TARBALLS_LOCATION)/$$addon-$$ARCHIVE $(BASE_URL)/$$addon/archive/$$ARCHIVE ;\
          cd - ;\
          echo $$addon-$$ARCHIVE > .$$addon ;\
        done 

$(PLATFORM): $(TARBALLS_LOCATION)/.$(ADDONS) $(DEPS)
	rm -rf $(PLATFORM)
	mkdir -p $(PLATFORM)
	for addon in $(ADDONS) ; do \
          mkdir $(PLATFORM)/$$addon ;\
	  ARCHIVE=$$(cat .$$addon) ;\
          cd $(PLATFORM)/$$addon; $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$$ARCHIVE ;\
          $(CMAKE) -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 . && make install || exit 1 ;\
          [ $$(echo $(HOST) | grep darwin) ] && cp -rf $$addon $(XBMCROOT)/addons && cp -fL $${addon}.dylib $(XBMCROOT)/addons/$$addon ;\
          cd - ;\
        done

clean:
	for addon in $(ADDONS) ; do \
          ARCHIVE=$$(cat .$$addon) ;\
          make -C $(PLATFORM)/$$addon clean ;\
        done
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM) .audio*

.installed-$(PLATFORM): $(TARBALLS_LOCATION)/.$(ADDONS) $(PLATFORM)
	touch $@

