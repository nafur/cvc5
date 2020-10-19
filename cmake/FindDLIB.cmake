# Find dlib
# DLIB_FOUND - system has dlib
# DLIB_INCLUDE_DIR - the dlib include directory
# DLIB_LIBRARIES - Libraries needed to use dlib

# Note: contrib/get-dlib copies header files to deps/install/include/dlib.
find_path(DLIB_INCLUDE_DIR NAMES dlib/config.h)
find_library(DLIB_LIBRARIES NAMES dlib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dlib
  DEFAULT_MSG
  DLIB_INCLUDE_DIR DLIB_LIBRARIES)

mark_as_advanced(DLIB_INCLUDE_DIR DLIB_LIBRARIES)
if(DLIB_LIBRARIES)
  message(STATUS "Found dlib: ${DLIB_LIBRARIES}")
endif()
