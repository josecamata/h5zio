
add_executable(test_zfp test_zfp.cpp data.cpp data.h)
target_link_libraries(test_zfp h5zio ${HDF5_LIBRARIES} ${SZ_HDF5_LIBRARY})
target_compile_definitions(test_zfp PRIVATE HDF5)

add_executable(test_sz test_sz.cpp data.cpp data.h)
target_link_libraries(test_sz h5zio ${HDF5_LIBRARIES} ${SZ_HDF5_LIBRARY})
target_compile_definitions(test_sz PRIVATE HDF5)

add_executable(test_h5 test_h5.cpp)
target_link_libraries(test_h5 h5zio ${HDF5_LIBRARIES} ${SZ_HDF5_LIBRARY})
target_compile_definitions(test_h5 PRIVATE HDF5)

