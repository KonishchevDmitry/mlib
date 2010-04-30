cmake_minimum_required(VERSION 2.6)

project(GROV)
include(CMakeProjectFiles.txt)


configure_file(config.hpp.in config.hpp)


include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/.. ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/..)


QT4_WRAP_CPP(GROV_MOC_CPP ${GROV_MOC_SOURCES})
QT4_WRAP_UI(GROV_UI_CPP ${GROV_UIS})


if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
	configure_file(${APP_UNIX_NAME}.desktop.in ${APP_UNIX_NAME}.desktop)
	install(FILES ${PROJECT_BINARY_DIR}/${APP_UNIX_NAME}.desktop DESTINATION share/applications)

	install(FILES ${PROJECT_SOURCE_DIR}/mans/${APP_UNIX_NAME}.1.gz DESTINATION share/man/man1)
	install(FILES ${PROJECT_SOURCE_DIR}/mans/ru/${APP_UNIX_NAME}.1.gz DESTINATION share/man/ru/man1)
endif()


install(DIRECTORY icons/ DESTINATION ${APP_ICONS_DIR})


QT4_ADD_TRANSLATION(GROV_QM ${GROV_TRANSLATIONS})
install(FILES ${GROV_QM} DESTINATION ${APP_TRANSLATIONS_DIR})


add_executable(${APP_UNIX_NAME} ${GROV_SOURCES} ${GROV_MOC_CPP} ${GROV_QM} ${GROV_UI_CPP})
target_link_libraries(${APP_UNIX_NAME} ${MLIB_LIBRARIES})
install(TARGETS ${APP_UNIX_NAME} DESTINATION ${APP_BIN_DIR})
