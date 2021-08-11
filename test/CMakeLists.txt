include_directories(zarks_SOURCE_DIR/include)

add_executable(tests main.cpp)
target_link_libraries(tests LibZarks)