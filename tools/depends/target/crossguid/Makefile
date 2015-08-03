include ../../Makefile.include
DEPS= ../../Makefile.include Makefile

# lib name, version
LIBNAME=crossguid
VERSION=8f399e8bd4
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# define specifying the native GUID implementation to use
GUID_PLATFORM_DEFINE=GUID_LIBUUID
ifeq ($(OS),android)
	GUID_PLATFORM_DEFINE=GUID_ANDROID
endif
ifeq ($(OS),osx)
	GUID_PLATFORM_DEFINE=GUID_CFUUID
endif
ifeq ($(OS),ios)
	GUID_PLATFORM_DEFINE=GUID_CFUUID
endif

LIBDYLIB=$(PLATFORM)/lib$(LIBNAME).a

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)

$(LIBDYLIB): $(PLATFORM)
	cd $(PLATFORM); $(CXX) -c guid.cpp -o guid.o $(CXXFLAGS) -D$(GUID_PLATFORM_DEFINE)
	cd $(PLATFORM); $(AR) rvs lib$(LIBNAME).a guid.o

.installed-$(PLATFORM): $(LIBDYLIB)
	cp $(PLATFORM)/guid.h $(PREFIX)/include/
	cp $(LIBDYLIB) $(PREFIX)/lib/
	touch $@

clean:
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

