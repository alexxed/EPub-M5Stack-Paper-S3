// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#define __BATTERY_VIEWER__ 1
#include "viewers/battery_viewer.hpp"

// Battery viewer relies on Inkplate's battery driver; not available for M5Paper.
// This file is excluded from the build for BOARD_TYPE_M5PAPER.