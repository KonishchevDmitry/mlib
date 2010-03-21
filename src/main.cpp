/**************************************************************************
*                                                                         *
*   grov - Google Reader offline viewer                                   *
*                                                                         *
*   Copyright (C) 2010, Dmitry Konishchev                                 *
*   http://konishchevdmitry.blogspot.com/                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


// TODO: rewrite

#include <QtGui/QApplication>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
// TODO
#include <QtCore/QTextCodec>

#include <mlib/core.hpp>

#include "main_window.hpp"
#include "client.hpp"

using namespace grov;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	m::set_debug_level(m::DEBUG_LEVEL_VERBOSE);
	m::set_debug_level(m::DEBUG_LEVEL_ENABLED);

MLIB_D("Starting application...");

//QLocale::setDefault(QLocale::system());

// TODO
QTextCodec::setCodecForCStrings ( QTextCodec::codecForLocale());
// TODO: http://qt.nokia.com/doc/4.6/internationalization.html
// -->
QTranslator qtTranslator;
qtTranslator.load("qt_" + QLocale::system().name(),
QLibraryInfo::location(QLibraryInfo::TranslationsPath));
app.installTranslator(&qtTranslator);


QTranslator myappTranslator;
myappTranslator.load("lang/grov_" + QLocale::system().name());
app.installTranslator(&myappTranslator);
// <--


	Main_window w(argv[1], argv[2]);
	w.show();
	//w.download();

	//Reader reader(argv[1], argv[2]);
	//Client client(argv[1], argv[2]);
	//client.download();

	return app.exec();
}

