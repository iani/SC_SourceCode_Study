# - Find sndfile
# Find the native sndfile includes and libraries
#
#  SNDFILE_INCLUDE_DIR - where to find sndfile.h, etc.
#  SNDFILE_LIBRARIES   - List of libraries when using libsndfile.
#  SNDFILE_FOUND       - True if libsndfile found.

if(SNDFILE_INCLUDE_DIR)
    # Already in cache, be silent
    set(SNDFILE_FIND_QUIETLY TRUE)
endif(SNDFILE_INCLUDE_DIR)

if(NO_LIBSNDFILE)
	#if(NOT SNDFILE_FIND_QUIETLY)
	#	message(STATUS "FindSndfile: sndfile deactivated (NO_LIBSNDFILE)")
	#endif()
	set(SNDFILE_FOUND False)
	set(SNDFILE_INCLUDE_DIR "nowhere")  # for onceonly check above
	set(SNDFILE_LIBRARIES "")
	add_definitions("-DNO_LIBSNDFILE")
elseif (APPLE)
	set(SNDFILE_FOUND TRUE)
	set(SNDFILE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external_libraries/libsndfile/)
	set(SNDFILE_LIBRARIES ${CMAKE_SOURCE_DIR}/platform/mac/lib/scUBlibsndfile.a)
	add_definitions("-isystem ${CMAKE_SOURCE_DIR}/external_libraries/libsndfile")

	# TODO on non-apple platforms, we need to be able to test for >=1018.
	# (On apple it is known true, because we bundle a later version.)
	add_definitions("-DLIBSNDFILE_1018")

else()
	find_path(SNDFILE_INCLUDE_DIR sndfile.h)

	find_library(SNDFILE_LIBRARY NAMES sndfile)

	# Handle the QUIETLY and REQUIRED arguments and set SNDFILE_FOUND to TRUE if
	# all listed variables are TRUE.
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(SNDFILE DEFAULT_MSG
		SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)

	if(SNDFILE_FOUND)
		set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARY})
	else(SNDFILE_FOUND)
		set(SNDFILE_LIBRARIES)
	endif(SNDFILE_FOUND)

endif()
mark_as_advanced(SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)
