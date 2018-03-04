project(autoconf)
cmake_minimum_required(VERSION 3.4)

include(ExternalProject)
externalproject_add(autoconf
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --enable-static
                      --disable-shared
                      --with-pic
                      ${EXTRA_CFLAGS}
                    INSTALL_COMMAND make install
                    BUILD_IN_SOURCE 1)
