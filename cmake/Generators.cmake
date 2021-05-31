include_guard(GLOBAL)


# Declare targets for generators
add_library(sof_gen_common STATIC
  gen/common.cpp
)
target_link_libraries(sof_gen_common PUBLIC sof_util)

add_library(sof_gen_main STATIC
  gen/gen_main.cpp
)
target_link_libraries(sof_gen_main PUBLIC sof_gen_common)

add_library(sof_gen_json_main STATIC
  gen/gen_json_main.cpp
)
target_link_libraries(sof_gen_json_main PUBLIC sof_gen_common jsoncpp_lib)



# Creates the directory in which `file` will be located
function(_ensure_dir file)
  string(REGEX REPLACE "^(.*)/[^/]+$" "\\1" dir "${file}")
  file(MAKE_DIRECTORY "${dir}")
endfunction()

# Generates a file with generator target `gen_target`, which is compiled from source `gen_src`.
# The result is stored in `${PROJECT_BINARY_DIR}/out_file`
function(generate_file gen_target gen_src out_file)
  add_executable("${gen_target}" "${gen_src}")
  target_link_libraries("${gen_target}" sof_gen_main)
  _ensure_dir("${PROJECT_BINARY_DIR}/${out_file}")
  add_custom_command(
    OUTPUT "${PROJECT_BINARY_DIR}/${out_file}"
    COMMAND "${gen_target}" "${PROJECT_BINARY_DIR}/${out_file}"
    DEPENDS "${gen_target}"
  )
endfunction()

# Generates a file with generator target `gen_target`, which is compiled from source `gen_src`.
# The generator reads the input from JSON file `${PROJECT_SOURCE_DIR}/in_file` and stores the
# result in `${PROJECT_BINARY_DIR}/out_file`
function(generate_file_json gen_target gen_src out_file in_file)
  add_executable("${gen_target}" "${gen_src}")
  target_link_libraries("${gen_target}" sof_gen_json_main)
  _ensure_dir("${PROJECT_BINARY_DIR}/${out_file}")
  add_custom_command(
    OUTPUT "${PROJECT_BINARY_DIR}/${out_file}"
    COMMAND "${gen_target}" "${PROJECT_BINARY_DIR}/${out_file}" "${PROJECT_SOURCE_DIR}/${in_file}"
    DEPENDS "${gen_target}" "${in_file}"
  )
endfunction()
