-include ../../Makefile.include
include AUDIO-ADDONS-VERSION

LIBNAME = xbmc-audio-addons
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
ifeq ($(HOST),)
  HOST = $(shell uname -s)
endif

workdir = $(shell pwd)
ALLADDONS = $(foreach addon,$(ADDONS),$(PLATFORM)/$(addon)/.installed)

all: .installed-$(PLATFORM)

$(foreach addon,$(ADDONS),$(PLATFORM)/$(addon)/.installed):
ifeq ($(PREFIX),)
	@echo
	@echo "ERROR: please set PREFIX to the xbmc install path e.g. make PREFIX=/usr/local"
	@exit 1
endif
	echo "param: $@"
#	ADDON=$(subst .audio,audio,$(notdir $@)) 
	addon=$(subst /.installed,,$(subst $(PLATFORM)/,,$@)) ;\
        VERSION=$$(grep $$ADDON-VERSION AUDIO-ADDONS-VERSION | sed 's/.*=//g') ; echo "Version: $$VERSION" ;\
        ARCHIVE=$$VERSION.tar.gz ;\
        [ -f $(TARBALLS_LOCATION)/$$ADDON-$$ARCHIVE ] || $(RETRIEVE_TOOL) -Ls --create-dirs -f -o $(TARBALLS_LOCATION)/$$ADDON-$$ARCHIVE $(BASE_URL)/$$ADDON/archive/$$ARCHIVE ;\
        mkdir -p $(PLATFORM) ;\
        rm -rf $(PLATFORM)/$$addon ;\
        mkdir $(PLATFORM)/$$addon ;\
        cd $(PLATFORM)/$$addon ;\
        $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$$ADDON-$$ARCHIVE ;\
        $(CMAKE) $(INSTALL_PREFIX) -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 . ;\
        make install && touch .installed ;\
        if [ $$(echo $(HOST) | grep darwin) ]; then \
          cp -rf $$addon $(XBMCROOT)/addons ;\
          cp -fL $${addon}.dylib $(XBMCROOT)/addons/$$addon ;\
        fi

clean:
	for addon in $(ADDONS) ; do \
          ARCHIVE=$$(cat .$$addon) ;\
          make -C $(PLATFORM)/$$addon clean ;\
        done
	rm -f .installed-$(PLATFORM) $(addprefix .,$(notdir $(ADDONS)))

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM) .audio*

.installed-$(PLATFORM): $(ALLADDONS)
	touch $@

