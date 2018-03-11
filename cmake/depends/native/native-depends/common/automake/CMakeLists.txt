project(automake)
cmake_minimum_required(VERSION 3.4)

include(ExternalProject)
externalproject_add(automake
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure
                      --prefix=${OUTPUT_DIR}
                      ${EXTRA_CFLAGS}
                    INSTALL_COMMAND make install
                    BUILD_IN_SOURCE 1)
