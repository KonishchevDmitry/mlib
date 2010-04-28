# Configures MLib.
#
# Application is not allowed to configure Qt by itself (CMake's Qt module can't
# be called twice).
#
# Application must set:
#  MLIB_APP_MLIB_PARENT_DIR - directory in which MLib is located.
#  MLIB_APP_TRANSLATIONS_DIR - directory for *.qm files (relative to install prefix).
#
# Application may set:
#  MLIB_APP_QT_MIN_VERSION - Minimal Qt version needed for application
#  MLIB_APP_QT_MODULES - Qt modules that application use.
#  MLIB_DEBUG_MODE - enables debug mode.
#  MLIB_DEVELOP_MODE - enables develop mode.
#  MLIB_ENABLE_ALIASES - enables global namespace aliases for common classes
#      and functions.


# Boost -->
	find_package(Boost 1.34 REQUIRED)
	if(Boost_FOUND)
		include_directories(${Boost_INCLUDE_DIRS})
		set(MLIB_LIBRARIES ${MLIB_LIBRARIES} ${Boost_LIBRARIES})
	endif()
# Boost <--


# Qt -->
	find_package(Qt4 ${MLIB_APP_QT_MIN_VERSION} COMPONENTS QtCore ${MLIB_APP_QT_MODULES} REQUIRED)
	include(${QT_USE_FILE})
	set(MLIB_LIBRARIES ${MLIB_LIBRARIES} ${QT_LIBRARIES})
# Qt <--


# Defines -->
	add_definitions(-DMLIB_ENABLE_ALIASES)

	if(MLIB_DEBUG_MODE)
		add_definitions(-DMLIB_DEBUG_MODE)
	endif()

	if(MLIB_DEVELOP_MODE)
		add_definitions(-DMLIB_DEVELOP_MODE)
	endif()
# Defines <--


# Includes
include_directories(${MLIB_APP_MLIB_PARENT_DIR})


# Compiler CFLAGS -->
	# Large files support
	if(UNIX)
		add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)
	endif()

	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

		# Some of this options are not supported by older versions of GCC
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fdiagnostics-show-option")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Winit-self -Wpointer-arith -Wlogical-op -Wcast-qual -Wcast-align -Wnon-virtual-dtor")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror=reorder -Werror=main -Werror=missing-braces -Werror=array-bounds -Werror=sequence-point -Werror=return-type")

		# GCC forgets about -Wno-* options when -O* flag is specified in CXXFLAGS
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-empty-body")

		#-Winline -Weffc++ -Wold-style-cast -Wsign-conversion -Wconversion -Wfloat-equal -Wstack-protector -Woverloaded-virtual
	endif()
# Compiler CFLAGS <--


link_directories(${MLIB_APP_MLIB_PARENT_DIR}/mlib)
set(MLIB_LIBRARIES ${MLIB_LIBRARIES} mlib)
