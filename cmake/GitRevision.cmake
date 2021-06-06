# Detect current Git revision and put it to the following variables. `${PROJECT_NAME}_GIT_REVISION`
# contains the full commit hash, while `${PROJECT_NAME}_GIT_REVISION_SHORT` contains only the short
# commit hash. If we fail to detect Git revision, these variables are set to `unknown`
set(${PROJECT_NAME}_GIT_REVISION "unknown")
set(${PROJECT_NAME}_GIT_REVISION_SHORT "unknown")

if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
  find_package(Git)
  if(Git_FOUND)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      OUTPUT_VARIABLE ${PROJECT_NAME}_GIT_REVISION
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      OUTPUT_VARIABLE ${PROJECT_NAME}_GIT_REVISION_SHORT
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()
endif()
