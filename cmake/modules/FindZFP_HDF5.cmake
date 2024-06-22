
include(FindPackageHandleStandardArgs)

SET(ZFP_HDF5_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/opt/local
	/opt/lib
	/opt
	$ENV{H5Z_ZFP_ROOT}
)

FIND_PATH(ZFP_HDF5_INCLUDE_DIR H5Zzfp.h
	HINTS
	$ENV{H5Z_ZFP_ROOT}
	PATH_SUFFIXES include
	PATHS ${ZFP_HDF5_SEARCH_PATHS}
)

FIND_LIBRARY(ZFP_HDF5_LIBRARY
	NAMES h5zzfp
	HINTS
	$ENV{H5Z_ZFP_ROOT}
	PATH_SUFFIXES lib64 lib
	PATHS ${ZFP_HDF5_SEARCH_PATHS}
)

find_package_handle_standard_args(ZFP_HDF5 REQUIRED_VARS ZFP_HDF5_LIBRARY ZFP_HDF5_INCLUDE_DIR)

if (ZFP_HDF5_FOUND)
  mark_as_advanced(ZFP_HDF5_INCLUDE_DIR)
  mark_as_advanced(ZFP_HDF5_LIBRARY)
  message(STATUS "Found ZFP_HDF5: ${ZFP_HDF5_LIBRARY}")
  message(STATUS "ZFP_HDF5 include: ${ZFP_HDF5_INCLUDE_DIR}")
else()
  message(STATUS "Could not find ZFP_HDF5")
endif()

# if (ZFP_HDF5_FOUND AND NOT TARGET ZFP_HDF5::ZFP_HDF5)
#   add_library(ZFP_HDF5::ZFP_HDF5 IMPORTED)
#   set_property(TARGET ZFP_HDF5::ZFP_HDF5 PROPERTY IMPORTED_LOCATION ${ZFP_HDF5_LIBRARY})
#   target_include_directories(ZFP_HDF5::ZFP_HDF5 INTERFACE ${ZFP_HDF5_INCLUDE_DIR})
# endif()
