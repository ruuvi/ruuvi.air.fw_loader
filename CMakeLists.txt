# @copyright Ruuvi Innovations Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ruuvi.fw_loader)

# Find the directory containing <module_name>.cmake by walking up:
#   <this>/cmake/modules → parent/cmake/modules → grandparent/cmake/modules
#
# Usage:
#   find_module_dir_upwards(OUT_VAR git_describe_tag [REQUIRED] [SUBDIR "cmake/modules"])
#
# Notes:
# - module_name may be given with or without the .cmake suffix
# - If REQUIRED is passed and not found → FATAL_ERROR
# - On success, OUT_VAR is set to the *directory* path (normalized)
# - On failure (without REQUIRED), OUT_VAR is set to an empty string
function(find_module_dir_upwards OUT_VAR MODULE_NAME)
  set(options REQUIRED)
  set(oneValueArgs SUBDIR)
  cmake_parse_arguments(FMDU "${options}" "${oneValueArgs}" "" ${ARGN})

  if(MODULE_NAME MATCHES "\\.cmake$")
    set(_module_file "${MODULE_NAME}")
  else()
    set(_module_file "${MODULE_NAME}.cmake")
  endif()

  if(NOT FMDU_SUBDIR)
    set(FMDU_SUBDIR "cmake/modules")
  endif()

  # Build candidate search dirs: current → parent → grandparent
  set(_dirs)
  list(APPEND _dirs "${CMAKE_CURRENT_LIST_DIR}/${FMDU_SUBDIR}")

  get_filename_component(_p1 "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
  list(APPEND _dirs "${_p1}/${FMDU_SUBDIR}")

  get_filename_component(_p2 "${_p1}" DIRECTORY)
  list(APPEND _dirs "${_p2}/${FMDU_SUBDIR}")

  # Find the directory containing the module file
  find_path(_hit_dir
    NAMES "${_module_file}"
    HINTS ${_dirs}
    NO_DEFAULT_PATH
  )

  if(NOT _hit_dir)
    if(FMDU_REQUIRED)
      message(FATAL_ERROR
        "Could not find '${_module_file}'. Searched:\n  ${_dirs}")
    endif()
    set(${OUT_VAR} "" PARENT_SCOPE)
    return()
  endif()

  # Normalize and return
  get_filename_component(_norm "${_hit_dir}" REALPATH)
  set(${OUT_VAR} "${_norm}" PARENT_SCOPE)
endfunction()

find_module_dir_upwards(MOD_DIR git_describe_tag.cmake REQUIRED SUBDIR "cmake/modules")

set_property(TARGET app_version_h PROPERTY APP_VERSION_CUSTOMIZATION
    "//#include;\\\"fwloader_version_extra.h\\\";"
)

include("${MOD_DIR}/git_describe_tag.cmake")

SET(TAG_PREFIX "fwloader_v")
SET(TAG_WORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

set(GENERATED_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")

set(VH ${PROJECT_BINARY_DIR}/zephyr/include/generated/zephyr/app_version.h)
set(MYVH ${PROJECT_BINARY_DIR}/zephyr/include/generated/zephyr/app_version.override.h)

git_describe_tag(${TAG_WORK_DIR} ${TAG_PREFIX} FWLOADER_BUILD_VERSION)

add_custom_command(
  OUTPUT ${MYVH}
  COMMAND ${CMAKE_COMMAND} -DZEPHYR_BASE=${ZEPHYR_BASE}
    -DOUT_FILE=${MYVH}
    -DVERSION_TYPE=APP
    -DVERSION_FILE=${GENERATED_VERSION}
    -DAPP_VERSION_CUSTOMIZATION="$<TARGET_PROPERTY:app_version_h,APP_VERSION_CUSTOMIZATION>"
    -DAPP_BUILD_VERSION=${FWLOADER_BUILD_VERSION}
    -DCMAKE_MODULE_PATH=${MOD_DIR}
    -P ${ZEPHYR_BASE}/cmake/gen_version_h.cmake
  DEPENDS ${GENERATED_VERSION} ${git_dependency}
  COMMAND_EXPAND_LISTS
)

# stamp so we don't collide with Zephyr's rule that already outputs ${VH}
add_custom_command(
  OUTPUT ${MYVH}.stamp
  COMMAND ${CMAKE_COMMAND} -E copy ${MYVH} ${VH}
  COMMAND ${CMAKE_COMMAND} -E touch ${MYVH}.stamp
  DEPENDS version_h ${MYVH}             # ensure the default header exists first
)

add_custom_target(override_version_h ALL
  DEPENDS ${MYVH}.stamp)

# Make sure all generated headers wait for our override too
add_dependencies(zephyr_generated_headers override_version_h)
# Make the Zephyr 'app' target depend on the VERSION file and generted header
add_dependencies(app override_version_h)


target_sources(app PRIVATE
        src/fwloader_bluetooth.c
        src/fwloader_bluetooth.h
        src/fwloader_early_init.c
        src/fwloader_err_handler.c
        src/fwloader_ext_flash_power.c
        src/fwloader_ext_flash_power.h
        src/fwloader_fw_ver.c
        src/fwloader_fw_ver.h
        src/fwloader_button.c
        src/fwloader_button.h
        src/fwloader_button_cb.c
        src/fwloader_gpio_input.c
        src/fwloader_gpio_input.h
        src/fwloader_led.c
        src/fwloader_led.h
        src/fwloader_led_err.c
        src/fwloader_led_err.h
        src/fwloader_mcumgr_mgmt_callbacks.c
        src/fwloader_mcumgr_mgmt_callbacks.h
        src/fwloader_segger_rtt.c
        src/fwloader_segger_rtt.h
        src/fwloader_settings.c
        src/fwloader_settings.h
        src/fwloader_supercap.c
        src/fwloader_supercap.h
        src/fwloader_watchdog.c
        src/fwloader_watchdog.h
        src/fw_img_hw_rev.c
        src/fw_img_hw_rev.h
        src/main.c
)

target_include_directories(app PRIVATE
    ../include
    ${ZEPHYR_BASE}/kernel/include
    ${ARCH_DIR}/${ARCH}/include
)

target_link_options(app INTERFACE
        "-Wl,--wrap=sys_reboot"
)
