add_custom_target(splines ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/splines.html" "${CMAKE_CURRENT_BINARY_DIR}/splines.js"
  VERBATIM
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/splines.html"
  DEPENDS "${CMAKE_SOURCE_DIR}/src/splines/splines.html"
  COMMAND "cp" "${CMAKE_SOURCE_DIR}/src/splines/splines.html" "${CMAKE_CURRENT_BINARY_DIR}/splines.html")

file(GLOB TYPESCRIPT_FILES "${CMAKE_SOURCE_DIR}/src/splines/*.ts")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/splines.js"
  DEPENDS "${TYPESCRIPT_FILES}"
  COMMAND "tsc" --module amd "${CMAKE_SOURCE_DIR}/src/splines/splines.ts" --outFile "${CMAKE_CURRENT_BINARY_DIR}/splines.js")
