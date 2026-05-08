
# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

mcux_add_configuration(
    CC "-DSDK_DEBUGCONSOLE=1"
    CX "-DSDK_DEBUGCONSOLE=1"
)


mcux_add_source(
    SOURCES frdmmcxn947/clock_config.c
            frdmmcxn947/clock_config.h
)

mcux_add_include(
    INCLUDES frdmmcxn947
)
