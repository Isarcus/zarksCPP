add_executable(unit_tests unit_tests.cpp)
target_link_libraries(unit_tests ${ZARKS_LIB_NAME})

add_executable(all_noise all_noise.cpp)
target_link_libraries(all_noise ${ZARKS_LIB_NAME})

add_executable(test_binary test_binary.cpp)
target_link_libraries(test_binary ${ZARKS_LIB_NAME})
