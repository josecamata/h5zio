
include(FindPackageHandleStandardArgs)

SET(SZ_HDF5_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/opt/local
	/opt/lib
	/opt
	/opt/lib/sz
	$ENV{SZ_ROOT}
)

FIND_PATH(SZ_HDF5_INCLUDE_DIR H5Z_SZ.h
	HINTS
	$ENV{SZ_ROOT}
	PATH_SUFFIXES include include/hdf5_sz
	PATHS ${SZ_HDF5_SEARCH_PATHS}
)

FIND_LIBRARY(SZ_HDF5_LIBRARY
	NAMES hdf5sz
	HINTS
	$ENV{SZ_ROOT}
	PATH_SUFFIXES lib64 lib
	PATHS ${SZ_HDF5_SEARCH_PATHS}
)


find_package_handle_standard_args(SZ_HDF5 REQUIRED_VARS SZ_HDF5_LIBRARY SZ_HDF5_INCLUDE_DIR)

if (SZ_HDF5_FOUND)
  mark_as_advanced(SZ_HDF5_INCLUDE_DIR)
  mark_as_advanced(SZ_HDF5_LIBRARY)
  message(STATUS "Found SZ_HDF5_INCLUDE_DIR: ${SZ_HDF5_INCLUDE_DIR}")
  message(STATUS "SZ_HDF5_LIBRARY: ${SZ_HDF5_LIBRARY}")
else()
  message(STATUS "Could not find SZ_HDF5")
endif()

# if (SZ_FOUND AND NOT TARGET SZ::SZ)
#   add_library(SZ::SZ IMPORTED)
#   set_property(TARGET SZ::SZ PROPERTY IMPORTED_LOCATION ${SZ_LIBRARY})
#   target_include_directories(SZ::SZ INTERFACE ${SZ_INCLUDE_DIR})
# endif()
