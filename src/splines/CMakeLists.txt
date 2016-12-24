add_custom_target(splines ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/splines.html" "${CMAKE_CURRENT_BINARY_DIR}/splines.js"
  VERBATIM
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/splines.html"
  COMMAND "cp" "${CMAKE_SOURCE_DIR}/src/splines/splines.html" "${CMAKE_CURRENT_BINARY_DIR}/splines.html")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/splines.js"
  COMMAND "tsc" "${CMAKE_SOURCE_DIR}/src/splines/splines.ts" --outFile "${CMAKE_CURRENT_BINARY_DIR}/splines.js")
