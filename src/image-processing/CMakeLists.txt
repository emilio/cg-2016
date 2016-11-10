# add_library(image-processinglib OBJECT
#   PhysicsState.cpp
# )

add_executable(image-processing image-processing.cpp
  #  $<TARGET_OBJECTS:image-processinglib>
  $<TARGET_OBJECTS:base>
  $<TARGET_OBJECTS:geometry>
)

add_dependencies(image-processing Generated assimp)
target_link_libraries(image-processing -lm -lGL ${SFML_LIBRARIES} ${ASSIMP_LIB})

if(LINUX)
  # FIXME: Detect if SFML will use EGL or not to avoid a dumb call to
  # XInitThreads.
  target_link_libraries(image-processing -lX11)
endif()
