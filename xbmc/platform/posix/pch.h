/*
 *  Copyright (C) 2009-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

// The times in comments are how much time was spent parsing
// the header file according to C++ Build Insights in VS2019
#define _CRT_RAND_S
#include <algorithm> // 32 seconds
#include <chrono> // 72 seconds
#include <fstream>
#include <iostream>
#include <map>
#include <mutex> // 19 seconds
#include <string>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h> // 53 seconds
#include <locale>
#include <memory>

// anything below here should be headers that very rarely (hopefully never)
// change yet are included almost everywhere.
/* empty */

#include "FileItem.h" //63 seconds
#include "utils/log.h"
#include "addons/addoninfo/AddonInfo.h" // 62 seconds
