
add_executable(test_zfp test_zfp.cpp data.cpp data.h)
target_link_libraries(test_zfp h5zio ${HDF5_LIBRARIES} ${SZ_HDF5_LIBRARY})
target_compile_definitions(test_zfp PRIVATE HDF5)

