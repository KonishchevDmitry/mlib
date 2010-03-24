cmake_minimum_required(VERSION 2.6)

project(MLIB)
include(CMakeProjectFiles.txt)

QT4_WRAP_CPP(MLIB_MOC_CPP ${MLIB_MOC_SOURCES})
QT4_WRAP_UI(MLIB_UI_CPP ${MLIB_UIS})

add_library(mlib STATIC
	${MLIB_SOURCES}
	${MLIB_MOC_CPP}
	${MLIB_UI_CPP}
)
