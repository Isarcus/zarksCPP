add_executable(heightmaps heightmaps.cpp)
target_link_libraries(heightmaps ${ZARKS_LIB_NAME})

add_executable(modern_art modern_art.cpp)
target_link_libraries(modern_art ${ZARKS_LIB_NAME})

add_executable(cool_graph cool_graph.cpp)
target_link_libraries(cool_graph ${ZARKS_LIB_NAME})

add_executable(trippy_noise trippy_noise.cpp)
target_link_libraries(trippy_noise ${ZARKS_LIB_NAME})
