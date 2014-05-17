-include ../../Makefile.include
include AUDIO-ADDONS-VERSION

LIBNAME = xbmc-audioencoder-addons
ifeq ($(RETRIEVE_TOOL),)
  RETRIEVE_TOOL = curl
endif
ifeq ($(ARCHIVE_TOOL),)
  ARCHIVE_TOOL = tar --strip-components=1 -xf
endif
ifeq ($(PLATFORM),)
  PLATFORM = native
endif
ifeq ($(TARBALLS_LOCATION),)
  TARBALLS_LOCATION = $(shell pwd)
endif
ifeq ($(CMAKE),)
  CMAKE = cmake
  ifneq ($(PREFIX),)
    INSTALL_PREFIX = -DCMAKE_INSTALL_PREFIX=$(PREFIX)
  endif
endif


all: .installed-$(PLATFORM)

$(foreach addon,$(ADDONS),.$(addon)-installed):
ifeq ($(PREFIX),)
	@echo
	@echo "ERROR: please set PREFIX to the xbmc install path e.g. make PREFIX=/usr/local"
	@exit 1
endif
	addon=$(subst -installed,,$(subst .audio,audio,$@)) ;\
        VERSION=$$(grep $$addon-VERSION AUDIO-ADDONS-VERSION | sed 's/.*=//g') ; echo "Version: $$VERSION" ;\
        ARCHIVE=$$VERSION.tar.gz ;\
        [ -f $(TARBALLS_LOCATION)/$$addon-$$ARCHIVE ] || $(RETRIEVE_TOOL) -Ls --create-dirs -f -o $(TARBALLS_LOCATION)/$$addon-$$ARCHIVE $(BASE_URL)/$$addon/archive/$$ARCHIVE ;\
	mkdir -p $(PLATFORM) ;\
        rm -rf $(PLATFORM)/$$addon ;\
        mkdir $(PLATFORM)/$$addon ;\
        cd $(PLATFORM)/$$addon ;\
        $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$$addon-$$ARCHIVE ;\
        $(CMAKE) $(INSTALL_PREFIX) -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 . ;\
        make install || exit 1 ;\
        [ $$(echo $(HOST) | grep darwin) ] && cp -rf $$addon $(XBMCROOT)/addons && cp -fL $${addon}.dylib $(XBMCROOT)/addons/$$addon ;\
        cd - && echo $$VERSION > $@

clean:
	@for addon in $(ADDONS) ; do \
          if [ -d $(PLATFORM)/$$addon ]; then  make -C $(PLATFORM)/$$addon clean ; fi ;\
        done
	rm -f .installed-$(PLATFORM) $(addprefix .,$(notdir $(ADDONS)))

distclean:
	rm -rf $(PLATFORM) .installed-$(PLATFORM) $(foreach addon,$(ADDONS),.$(addon)-installed) *.tar.gz

.installed-$(PLATFORM): $(foreach addon,$(ADDONS),.$(addon)-installed)
	@touch $@
	@echo "# All done #"

