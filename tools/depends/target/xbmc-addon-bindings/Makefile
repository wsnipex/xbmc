include ../../Makefile.include
include $(XBMCROOT)/xbmc/addons/addon-bindings.mk
DEPS= ../../Makefile.include Makefile $(XBMCROOT)/xbmc/addons/addon-bindings.mk

XBMC_MAJOR_VERSION = $(shell awk '/VERSION_MAJOR/ {print $$3}' $(XBMCROOT)/xbmc/GUIInfoManager.h)
XBMC_MINOR_VERSION = $(shell awk '/VERSION_MINOR/ {print $$3}' $(XBMCROOT)/xbmc/GUIInfoManager.h)

all: .installed-$(PLATFORM)

$(foreach header,$(notdir $(BINDINGS)),$(PREFIX)/include/xbmc/$(header)):
	@echo "Copy addon bindings to $(PREFIX)/include/xbmc"
	@mkdir -p $(PREFIX)/include/xbmc
	@for f in $(BINDINGS); do \
	  cp -f $(XBMCROOT)/$$f $(PREFIX)/include/xbmc ; \
	done

.installed-$(PLATFORM): $(DEPS) $(foreach header,$(notdir $(BINDINGS)),$(PREFIX)/include/xbmc/$(header))
	@mkdir -p $(PREFIX)/lib/xbmc
	@cat $(XBMCROOT)/project/cmake/xbmc-config.cmake.in | sed -e 's|@prefix@|$(PREFIX)|' -e 's|@ARCH_DEFINES@|$(ARCH_DEFINES)|' -e 's|@XBMC_MAJOR_VERSION@|$(XBMC_MAJOR_VERSION)|' -e 's|@XBMC_MINOR_VERSION@|$(XBMC_MINOR_VERSION)|' > $(PREFIX)/lib/xbmc/xbmc-config.cmake
	@cp -f $(XBMCROOT)/project/cmake/xbmc-addon-helpers.cmake $(PREFIX)/lib/xbmc
	@cp -f $(XBMCROOT)/project/cmake/AddOptions.cmake $(PREFIX)/lib/xbmc
	@touch $@

clean:
	@rm -rf .installed-$(PLATFORM) $(PREFIX)/include/xbmc

distclean:: clean
