cmake_minimum_required(VERSION 2.6)

project(GROV)
include(CMakeProjectFiles.txt)
configure_file(config.hpp.in config.hpp)

include_directories(${PROJECT_SOURCE_DIR}/.. ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/..)

QT4_WRAP_CPP(GROV_MOC_CPP ${GROV_MOC_SOURCES})
QT4_WRAP_UI(GROV_UI_CPP ${GROV_UIS})

add_executable(grov ${GROV_SOURCES} ${GROV_MOC_CPP} ${GROV_UI_CPP})
target_link_libraries(grov ${MLIB_LIBRARIES})
install(TARGETS grov DESTINATION bin)
