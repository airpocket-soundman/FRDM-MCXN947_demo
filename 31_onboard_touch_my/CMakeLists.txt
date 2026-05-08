
# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22.0)

cmake_path(APPEND conf_file_path ${CMAKE_CURRENT_LIST_DIR} ${board}_${core_id} prj.conf)
set(CONF_FILE ${conf_file_path} ${CONF_FILE})
include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(31_onboard_touch_my LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/demo_apps/touch_sensing)


include("${board}_${core_id}/board_files.cmake")
include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_macro(
  CC "SDK_DEBUGCONSOLE_UART"
  )


mcux_add_source(
    SOURCES cm33_core0/board.c
            cm33_core0/board.h
            cm33_core0/freemaster_cfg.h
            cm33_core0/main.c
            cm33_core0/main.h
            cm33_core0/nt_setup.c
            cm33_core0/nt_setup.h
            cm33_core0/pin_mux.c
            cm33_core0/pin_mux.h
)

mcux_add_include(
    INCLUDES cm33_core0
)
mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)

