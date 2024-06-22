cmake_minimum_required(VERSION 3.9.0)
project(project-compression VERSION 0.1.0 LANGUAGES C CXX)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# HDF5
find_package(HDF5 REQUIRED)
if(HDF5_FOUND)
    # HDF5
    include_directories(${HDF5_INCLUDE_DIRS})
    link_directories(${HDF5_LIBRARY_DIRS})
    add_definitions(-DHDF5)
    set(H5ZIO_HAS_GZIP 1)
else()
    message(FATAL_ERROR "HDF5 not found")
endif()

find_package(SZ)
if(SZ_FOUND)
    # SZ
    include_directories(${SZ_INCLUDE_DIR})
    link_directories(${SZ_LIBRARY})

    find_package(SZ_HDF5)
    if(SZ_HDF5_FOUND)
    # SZ HDF5
        include_directories(${SZ_HDF5_INCLUDE_DIR})
        link_directories(${SZ_HDF5_LIBRARY})
        add_definitions(-DSZ_HDF5)
        set(H5ZIO_HAS_SZ 1)
    else()
        set(H5ZIO_HAS_SZ 0)
        message("SZ HDF5 not found")
    endif()
endif()

find_package(ZFP_HDF5)
if(ZFP_HDF5_FOUND)
    # ZFP HDF5
    include_directories(${ZFP_HDF5_INCLUDE_DIR})
    link_directories(${ZFP_HDF5_LIBRARY})
    add_definitions(-DZFP_HDF5)
    set(H5ZIO_HAS_ZFP 1)
else()
    message("ZFP HDF5 not found")
endif()


# Directories
include_directories(${PROJECT_SOURCE_DIR}/include)

configure_file(${PROJECT_SOURCE_DIR}/cmake/h5zio_config.h.in ${PROJECT_SOURCE_DIR}/include/h5zio_config.h)

file(GLOB SOURCES "src/*.cpp" 
                  "include/*.h" )

# Library
add_library(h5zio STATIC ${SOURCES})

#add_executable(main main.cpp)
#target_link_libraries(main ${HDF5_LIBRARIES} ${SZ_HDF5_LIBRARY} h5zio)
#target_compile_definitions(main PRIVATE HDF5)

add_subdirectory(test)
enable_testing()
add_test(NAME test_zfp COMMAND test_zfp)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
