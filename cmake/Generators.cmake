# This file is part of SoFCheck
#
# Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
#
# SoFCheck is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SoFCheck is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

include_guard(GLOBAL)


# Declare targets for generators
add_library(sof_gen_common STATIC
  gen/common.cpp
)
target_include_directories(sof_gen_common PUBLIC gen/)
target_link_libraries(sof_gen_common PUBLIC sof_util)

add_library(sof_gen_main STATIC
  gen/gen_main.cpp
)
target_link_libraries(sof_gen_main
  PUBLIC sof_gen_common
  PRIVATE sof_util
)

add_library(sof_gen_json_main STATIC
  gen/gen_json_main.cpp
)
target_link_libraries(sof_gen_json_main
  PUBLIC sof_gen_common
  PRIVATE ${JSONCPP_TARGET} sof_util
)


# Creates the directory in which `file` will be located
function(_ensure_dir file)
  string(REGEX REPLACE "^(.*)/[^/]+$" "\\1" dir "${file}")
  file(MAKE_DIRECTORY "${dir}")
endfunction()

# Deduces target name from generator source file name `src` and stores the result in `result`
function(_gen_get_name src result)
  string(REPLACE "/" "_" src "${src}")
  string(REGEX REPLACE "\.cpp$" "" src "${src}")
  set("${result}" "${src}" PARENT_SCOPE)
endfunction()

# Generates a file with generator, which is compiled from source `gen_src`. The result is stored
# in `${PROJECT_BINARY_DIR}/out_file`
function(generate_file gen_src out_file)
  _gen_get_name("${gen_src}" gen_target)
  add_executable("${gen_target}" "${gen_src}")
  target_link_libraries("${gen_target}" sof_gen_main)
  _ensure_dir("${PROJECT_BINARY_DIR}/${out_file}")
  add_custom_command(
    OUTPUT "${PROJECT_BINARY_DIR}/${out_file}"
    COMMAND "${gen_target}" "${PROJECT_BINARY_DIR}/${out_file}"
    DEPENDS "${gen_target}"
  )
endfunction()

# Generates a file with generator, which is compiled from source `gen_src`. The generator reads
# the input from JSON file `${PROJECT_SOURCE_DIR}/in_file` and stores the result in
# `${PROJECT_BINARY_DIR}/out_file`
function(generate_file_json gen_src out_file in_file)
  _gen_get_name("${gen_src}" gen_target)
  add_executable("${gen_target}" "${gen_src}")
  target_link_libraries("${gen_target}" sof_gen_json_main)
  _ensure_dir("${PROJECT_BINARY_DIR}/${out_file}")
  add_custom_command(
    OUTPUT "${PROJECT_BINARY_DIR}/${out_file}"
    COMMAND "${gen_target}" "${PROJECT_BINARY_DIR}/${out_file}" "${PROJECT_SOURCE_DIR}/${in_file}"
    DEPENDS "${gen_target}" "${in_file}"
  )
endfunction()
