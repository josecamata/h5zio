
include(FindPackageHandleStandardArgs)

SET(SZ_SEARCH_PATHS
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

FIND_PATH(SZ_INCLUDE_DIR sz.h
	HINTS
	$ENV{SZ_ROOT}
	PATH_SUFFIXES include include/sz
	PATHS ${SZ_SEARCH_PATHS}
)

FIND_LIBRARY(SZ_LIBRARY
	NAMES SZ
	HINTS
	$ENV{SZ_ROOT}
	PATH_SUFFIXES lib64 lib
	PATHS ${SZ_SEARCH_PATHS}
)

find_package_handle_standard_args(SZ REQUIRED_VARS SZ_LIBRARY SZ_INCLUDE_DIR)

if (SZ_FOUND)
  mark_as_advanced(SZ_INCLUDE_DIR)
  mark_as_advanced(SZ_LIBRARY)
  message(STATUS "Found SZ: ${SZ_LIBRARY}")
  message(STATUS "SZ include: ${SZ_INCLUDE_DIR}")
else()
  message(STATUS "Could not find SZ")
endif()

# if (SZ_FOUND AND NOT TARGET SZ::SZ)
#   add_library(SZ::SZ IMPORTED)
#   set_property(TARGET SZ::SZ PROPERTY IMPORTED_LOCATION ${SZ_LIBRARY})
#   target_include_directories(SZ::SZ INTERFACE ${SZ_INCLUDE_DIR})
# endif()
