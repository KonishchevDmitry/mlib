TEMPLATE = app

PROJECT_NAME = grov
PREFIX = /usr/local
TARGET = $$PROJECT_NAME

CONFIG *= qt
QT = core network xml sql gui webkit

SRC_DIR = $$_PRO_FILE_PWD_/src
INCLUDEPATH *= $$_PRO_FILE_PWD $$SRC_DIR
DEFINES *= MLIB_DEBUG_MODE MLIB_ENABLE_ALIASES

win32 {
	# The target is a Win32 window application
	CONFIG *= windows

	debug {
		# To get debug output on console
		CONFIG *= console
	}
}

unix {
	# Large files support.
	CONFIG *= largefile
}


debug {
	CONFIG *= warn_on

	contains(QMAKE_CXX, g++) {
		QMAKE_CXXFLAGS_WARN_ON *= -g
		QMAKE_CXXFLAGS_WARN_ON += -O0

		QMAKE_CXXFLAGS_WARN_ON += -Wall

		# Some of this options are not supported by older versions of GCC
		QMAKE_CXXFLAGS_WARN_ON += -fdiagnostics-show-option
		QMAKE_CXXFLAGS_WARN_ON += -Winit-self -Wpointer-arith -Wlogical-op -Wcast-qual -Wcast-align -Wnon-virtual-dtor
		QMAKE_CXXFLAGS_WARN_ON += -Werror=reorder -Werror=main -Werror=missing-braces -Werror=array-bounds -Werror=sequence-point -Werror=return-type

		# GCC forgets about -Wno-* options when -O* flag is specified in CXXFLAGS
		QMAKE_CXXFLAGS_WARN_ON += -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-empty-body

		#QMAKE_CXXFLAGS_WARN_ON += -Winline -Weffc++ -Wold-style-cast -Wsign-conversion -Wconversion -Wfloat-equal -Wstack-protector -Woverloaded-virtual
	}

	DEFINES *= DEBUG_MODE MLIB_DEVELOP_MODE
} else {
	CONFIG *= warn_off
}


FORMS += $$SRC_DIR/main_window.ui


target.path += $$PREFIX/bin
INSTALLS += target


# For future:
#
#documentation.path = /usr/local/program/doc
#documentation.files = docs/*
#
#CONFIG += link_pkgconfig
#PKGCONFIG += library_name
#
#LANG_PATH = lang
#TRANSLATIONS = $$LANG_PATH/grov_ru.ts
#message(Translation files: $$[QT_INSTALL_TRANSLATIONS])


# Automatically generated rules -->
	SOURCES += $$SRC_DIR/mlib/core/exception.cpp
	SOURCES += $$SRC_DIR/mlib/core/messages.cpp
	SOURCES += $$SRC_DIR/mlib/core/format.cpp
	SOURCES += $$SRC_DIR/mlib/core/tools.hh
	SOURCES += $$SRC_DIR/mlib/core/format.hh
	SOURCES += $$SRC_DIR/mlib/core/messages.hh
	SOURCES += $$SRC_DIR/storage.cpp
	SOURCES += $$SRC_DIR/reader_impl.cpp
	SOURCES += $$SRC_DIR/main_window.cpp
	SOURCES += $$SRC_DIR/items_list_parser.cpp
	SOURCES += $$SRC_DIR/reader.cpp
	SOURCES += $$SRC_DIR/client.cpp
	SOURCES += $$SRC_DIR/feed_item.cpp
	SOURCES += $$SRC_DIR/main.cpp

	HEADERS += $$SRC_DIR/reader.hxx
	HEADERS += $$SRC_DIR/mlib/core/tools.hpp
	HEADERS += $$SRC_DIR/mlib/core/types.hpp
	HEADERS += $$SRC_DIR/mlib/core/format.hpp
	HEADERS += $$SRC_DIR/mlib/core/exception.hpp
	HEADERS += $$SRC_DIR/mlib/core/messages.hpp
	HEADERS += $$SRC_DIR/mlib/core.hpp
	HEADERS += $$SRC_DIR/client.hpp
	HEADERS += $$SRC_DIR/storage.hpp
	HEADERS += $$SRC_DIR/client.hxx
	HEADERS += $$SRC_DIR/reader.hpp
	HEADERS += $$SRC_DIR/items_list_parser.hpp
	HEADERS += $$SRC_DIR/feed_item.hxx
	HEADERS += $$SRC_DIR/reader_impl.hpp
	HEADERS += $$SRC_DIR/storage.hxx
	HEADERS += $$SRC_DIR/feed_item.hpp
	HEADERS += $$SRC_DIR/common.hpp
	HEADERS += $$SRC_DIR/main_window.hpp
	HEADERS += $$SRC_DIR/reader_impl.hxx

	DEPENDPATH += $$SRC_DIR/mlib
	DEPENDPATH += $$SRC_DIR/mlib/core
# Automatically generated rules <--
