link_libraries(${ZARKS_LIB_NAME})

add_executable(all_noise all_noise.cpp)
add_executable(compile_mat2d compile_mat2d.cpp)
add_executable(test_gauss test_gauss.cpp)
add_executable(test_image_formats test_image_formats.cpp)
add_executable(test_gif test_gif.cpp)
add_executable(test_kmeans test_kmeans.cpp)
add_executable(unit_tests unit_tests.cpp)
