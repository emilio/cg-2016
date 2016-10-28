cmake_minimum_required(VERSION 2.8.5)
project(TEST CXX)

# In order to use YouCompleteMe with Vim.
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Whether to build in `Debug` or `Release` mode." FORCE)
  endif()
endif()

# Add this before our local stuff because assimp doesn't build with -Werror
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/assimp)

enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(WILL_RUN_TESTS ON CACHE BOOL "Run tests")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -g3 -Wall -Wextra")
# Add -DGL_DEBUG to debug GL errors.
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Werror -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

set(GENERATED_FILES
  res/vertex.glsl
  res/fragment.glsl
  res/models/Rmk3.obj
  res/models/suzanne.obj
  res/models/QuestionBlock.obj
  res/models/cube.obj
  res/models/box.obj
)

foreach(generated_file ${GENERATED_FILES})
  set_source_files_properties(${generated_file}
                              PROPERTIES GENERATED true HEADER_FILE_ONLY true)
  # TODO: Add a more complex command for this?
  GET_FILENAME_COMPONENT(dir "${CMAKE_CURRENT_BINARY_DIR}/${generated_file}" PATH)

  # In debug, just symlink the shit out of it, so developing live is easier.
  # In release, just copy them.
  IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${generated_file}"
                       COMMAND
                          "mkdir" "-p" "${dir}" "&&"
                          "ln" "-s" "${CMAKE_CURRENT_SOURCE_DIR}/${generated_file}"
                                    "${CMAKE_CURRENT_BINARY_DIR}/${generated_file}")
  ELSE(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${generated_file}"
                       COMMAND
                          "mkdir" "-p" "${dir}" "&&"
                          "cp" "${CMAKE_CURRENT_SOURCE_DIR}/${generated_file}"
                               "${CMAKE_CURRENT_BINARY_DIR}/${generated_file}"
                       DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${generated_file}")
  ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
endforeach(generated_file)

add_custom_target(Generated DEPENDS ${GENERATED_FILES})

link_directories(${LIBRARY_OUTPUT_PATH}/assimp/code)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/lib/glm)
include_directories(${CMAKE_SOURCE_DIR}/lib/assimp/include)

add_library(base OBJECT
  src/base/Program.cpp
  src/base/Scene.cpp
  src/base/Skybox.cpp
  src/base/Terrain.cpp
)

add_library(geometry OBJECT
  src/geometry/Mesh.cpp
  src/geometry/Node.cpp
)
add_executable(main src/main.cpp $<TARGET_OBJECTS:base> $<TARGET_OBJECTS:geometry>)
add_dependencies(main Generated)
add_dependencies(main assimp)

# Ensure we have SFML 2.x
find_package(SFML 2 REQUIRED system window graphics)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(main ${SFML_LIBRARIES})
endif()

target_link_libraries(main -lm -lGL)
IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  target_link_libraries(main -lassimpd)
ELSE(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  target_link_libraries(main -lassimp)
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(JFLAG -j${N})
endif()

add_executable(test-optional src/tests/optional.cpp)
add_test(test-optional ${CMAKE_BINARY_DIR}/bin/test-optional)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose ${JFLAG})
add_custom_target(format COMMAND find ${CMAKE_SOURCE_DIR}/src -regex "'.*\\.\\(cpp\\|h\\)'" -exec clang-format -i {} "\;")
