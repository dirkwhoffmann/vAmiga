// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

static const char *script[] = {

    "# Self-test script for vAmiga",
    "# ",
    "# This script is executed in nightly-builts to check the integrity of ",
    "# the compiled application. It runs several shell commands to check if ",
    "# anything breaks.",
    "# ",
    "# Dirk W. Hoffmann, 2023",

    "",
    "help",
    "",
    "# Verify auto-completion",
    "hel",
    "",
    "# Execute a hidden command",
    "joshua",
    "",
    "# Catch an error with 'try' (hidden commands are not auto-completed)",
    "try joshu",

    "",
    "regression setup A1000_OCS_1MB",
    "regression setup A500_OCS_1MB",
    "regression setup A500_ECS_1MB",
    "regression setup A500_PLUS_1MB",

    "",
    "amiga",
    "amiga defaults",
    "amiga set type PAL",
    "amiga set type NTSC",
    "amiga set fps 60",
    "amiga init A1000_OCS_1MB",
    "amiga init A500_OCS_1MB",
    "amiga init A500_ECS_1MB",
    "amiga init A500_PLUS_1MB",
    "amiga power off",
    "amiga reset",
    
};

