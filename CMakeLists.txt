cmake_minimum_required(VERSION 2.6)


# Application's properties -->
	set(APP_NAME grov)
	set(APP_UNIX_NAME grov)

	set(APP_VERSION_MAJOR 0)
	set(APP_VERSION_MINOR 1)
	set(APP_VERSION_PATCH 0)

	set(APP_BIN_DIR bin)
	set(APP_TRANSLATIONS_DIR share/${APP_UNIX_NAME}/translations)
# Application's properties <--


# Build type -->
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)
	endif()

	string(COMPARE EQUAL ${CMAKE_BUILD_TYPE} Debug DEVELOP_BUILD)

	if(DEVELOP_BUILD)
		add_definitions(-DGROV_DEBUG_MODE -DGROV_DEVELOP_MODE)
	endif()
# Build type <--


# Configuring MLib -->
	set(MLIB_APP_MLIB_PARENT_DIR ${PROJECT_SOURCE_DIR})
	set(MLIB_APP_TRANSLATIONS_DIR ${APP_TRANSLATIONS_DIR})

	set(MLIB_APP_QT_MODULES QtCore QtGui QtNetwork QtSql QtWebkit QtXml)
	set(MLIB_ENABLE_ALIASES TRUE)

	set(MLIB_DEBUG_MODE TRUE)

	if(DEVELOP_BUILD)
		set(MLIB_DEVELOP_MODE TRUE)
	endif()

	include(mlib/mlib.cmake)
# Configuring MLib <--


add_subdirectory(mlib)
add_subdirectory(src)
