project(gettext)
cmake_minimum_required(VERSION 3.4)

#set(PATCH_CMD sed -ie "/gets is a security hole/d" gettext-tools/gnulib-lib/stdio.in.h ";" sed -ie "/gets is a security hole/d" gettext-tools/libgettextpo/stdio.in.h ";" sed -ie "/gets is a security hole/d" gettext-runtime/gnulib-lib/stdio.in.h ";" sed -ie "s/SUBDIRS = doc intl gnulib-lib libgrep src libgettextpo po projects styles misc man m4 tests gnulib-tests examples/SUBDIRS = doc intl gnulib-lib libgrep src libgettextpo po projects styles misc man m4/" gettext-tools/Makefile.in)
set(PATCH_CMD sed -ie "/gets is a security hole/d" gettext-tools/gnulib-lib/stdio.in.h gettext-tools/libgettextpo/stdio.in.h gettext-runtime/gnulib-lib/stdio.in.h && sed -ie "s/SUBDIRS = doc intl gnulib-lib libgrep src libgettextpo po projects styles misc man m4 tests gnulib-tests examples/SUBDIRS = doc intl gnulib-lib libgrep src libgettextpo po projects styles misc man m4/" gettext-tools/Makefile.in)

include(ExternalProject)
externalproject_add(gettext
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    PATCH_COMMAND ${PATCH_CMD}
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --disable-shared --disable-curses --disable-acl --disable-c++ --disable-nls
                      --disable-csharp --disable-native-java --disable-java --without-emacs
                      --disable-libasprintf --disable-openmp
                      --with-included-gettext
                      --with-included-glib
                      --with-included-libcroco
                      --with-included-libxml
                      --without-git --without-cvs
                      ${EXTRA_CFLAGS}
                    INSTALL_COMMAND make install
                    BUILD_IN_SOURCE 1)
