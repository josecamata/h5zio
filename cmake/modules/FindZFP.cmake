
include(FindPackageHandleStandardArgs)

SET(ZFP_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/opt/local
	/opt/lib
	/opt
	/opt/lib/zfp
	$ENV{ZFP_ROOT}
)

FIND_PATH(ZFP_INCLUDE_DIR zfp.h
	HINTS
	$ENV{ZFP_ROOT}
	PATH_SUFFIXES include
	PATHS ${ZFP_SEARCH_PATHS}
)

FIND_LIBRARY(ZFP_LIBRARY
	NAMES ZFP
	HINTS
	$ENV{ZFP_ROOT}
	PATH_SUFFIXES lib64 lib
	PATHS ${ZFP_SEARCH_PATHS}
)

find_package_handle_standard_args(ZFP REQUIRED_VARS ZFP_LIBRARY ZFP_INCLUDE_DIR)

if (ZFP_FOUND)
  mark_as_advanced(ZFP_INCLUDE_DIR)
  mark_as_advanced(ZFP_LIBRARY)
  message(STATUS "Found ZFP: ${ZFP_LIBRARY}")
  message(STATUS "ZFP include: ${ZFP_INCLUDE_DIR}")
else()
  message(STATUS "Could not find ZFP")
endif()

# if (ZFP_FOUND AND NOT TARGET ZFP::ZFP)
#   add_library(ZFP::ZFP IMPORTED)
#   set_property(TARGET ZFP::ZFP PROPERTY IMPORTED_LOCATION ${ZFP_LIBRARY})
#   target_include_directories(ZFP::ZFP INTERFACE ${ZFP_INCLUDE_DIR})
# endif()
