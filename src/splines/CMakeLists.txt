add_custom_target(splines ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/splines.html"
  VERBATIM
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/require.min.js"
  DEPENDS "${CMAKE_SOURCE_DIR}/src/splines/require.min.js"
  COMMAND "cp" "${CMAKE_SOURCE_DIR}/src/splines/require.min.js" "${CMAKE_CURRENT_BINARY_DIR}/require.min.js")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/splines.html"
  DEPENDS "${CMAKE_SOURCE_DIR}/src/splines/splines.html"
          "${CMAKE_CURRENT_BINARY_DIR}/require.min.js"
          "${CMAKE_CURRENT_BINARY_DIR}/splines.js"
  COMMAND "cp" "${CMAKE_SOURCE_DIR}/src/splines/splines.html" "${CMAKE_CURRENT_BINARY_DIR}/splines.html")

file(GLOB TYPESCRIPT_FILES "${CMAKE_SOURCE_DIR}/src/splines/*.ts")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/splines.js"
  DEPENDS "${TYPESCRIPT_FILES}"
  COMMAND "tsc" --module amd "${CMAKE_SOURCE_DIR}/src/splines/splines.ts" --outFile "${CMAKE_CURRENT_BINARY_DIR}/splines.js")
