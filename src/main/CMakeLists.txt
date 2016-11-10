add_library(mainlib OBJECT
  PhysicsState.cpp
)

add_executable(main main.cpp
  $<TARGET_OBJECTS:mainlib>
  $<TARGET_OBJECTS:base>
  $<TARGET_OBJECTS:geometry>
)

add_dependencies(main Generated assimp)
target_link_libraries(main -lm -lGL ${SFML_LIBRARIES} ${ASSIMP_LIB})

if(LINUX)
  # FIXME: Detect if SFML will use EGL or not to avoid a dumb call to
  # XInitThreads.
  target_link_libraries(main -lX11)
endif()
