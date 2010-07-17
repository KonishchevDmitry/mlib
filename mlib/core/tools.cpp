/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#include <QtGui/QApplication>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QMap>
#include <QtCore/QProcess>
#include <QtCore/QTranslator>

#include "messages.hpp"
#include "exception.hpp"
#include "tools.hpp"


namespace m {


QString get_app_install_dir(const QString& app_bin_dir)
{
	size_t bin_dir_depth = 0;

	// Getting binaries' directory depth relative to installation
	// directory.
	// -->
	{
		QDir bin_dir = QDir::cleanPath(app_bin_dir);

		while(bin_dir.dirName() != "." && bin_dir.dirName() != "..")
		{
			bin_dir_depth++;
			bin_dir = QDir::cleanPath(bin_dir.filePath(".."));

			// If something goes wrong
			if(bin_dir_depth > 1000)
				M_THROW(QApplication::tr("Invalid application's binaries' directory path '%1'."), app_bin_dir);
		}
	}
	// <--

	// Getting installation directory path -->
	{
		QString app_binary_path = QDir(QCoreApplication::applicationFilePath()).absolutePath();
		QDir install_dir(app_binary_path);

		// One additional operation for the binary file name
		do {
			if(!install_dir.cdUp())
				M_THROW(QApplication::tr("Invalid application's binaries' directory path '%1' or gotten application's binary path '%2'."), app_bin_dir, app_binary_path);
		} while(bin_dir_depth--);

		return install_dir.path();
	}
	// Getting installation directory path <--
}



Version get_major_version(Version version)
{
	return version / 100 / 100;
}



Version get_minor_version(Version version)
{
	return version % ( 100 * 100 ) / 100;
}



Version get_patch_version(Version version)
{
	return version % 100;
}



QLocale get_system_locale(void)
{
	QLocale locale = QLocale::system();

#ifdef Q_OS_UNIX
	// Qt's locale detection under UNIX works wrong -->
	{
		QMap<QString,QString> env_vars;

		Q_FOREACH(const QString& var, QProcess::systemEnvironment())
		{
			int pos = var.indexOf('=');
			QString name = var.mid(0, pos);
			QString value = var.mid(pos + 1);
			env_vars[name] = value;
			MLIB_DV("Gotten an environment variable: '%1'='%2'.", name, value);
		}

		if(!env_vars["LC_ALL"].isEmpty())
		{
			locale = QLocale(env_vars["LC_ALL"]);
			MLIB_D("Setting the locale to LC_ALL's value '%1'.", locale.name());
		}
		else if(!env_vars["LC_MESSAGES"].isEmpty())
		{
			locale = QLocale(env_vars["LC_MESSAGES"]);
			MLIB_D("Setting the locale to LC_MESSAGES's value '%1'.", locale.name());
		}
		else if(!env_vars["LANG"].isEmpty())
		{
			locale = QLocale(env_vars["LANG"]);
			MLIB_D("Setting the locale to LANG's value '%1'.", locale.name());
		}
	}
	// Qt's locale detection under UNIX works wrong <--
#endif

	return locale;
}



Version get_version(Version major, Version minor, Version patch)
{
	return MLIB_GET_VERSION(major, minor, patch);
}



QString get_version_string(Version version)
{
	Version patch_version = get_patch_version(version);

	return
		QString::number(get_major_version(version)) +
		"." + QString::number(get_minor_version(version)) +
		( patch_version ? "." + QString::number(patch_version) : QString() );
}



void load_translations(const QString& translations_dir, const QString app_name)
{
	QLocale locale = get_system_locale();

	static QTranslator qt_translator;
	static QTranslator app_translator;
	static QTranslator mlib_translator;

	if(qt_translator.load("qt_" + locale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
		QCoreApplication::installTranslator(&qt_translator);
	else
		MLIB_D("Qt's translations for '%1' did not found.", locale.name());

	if(!translations_dir.isEmpty())
	{
		if(app_translator.load(app_name + "_" + locale.name(), translations_dir))
			QCoreApplication::installTranslator(&app_translator);
		else
			MLIB_D("Application's translations for '%1' did not found.", locale.name());

		if(mlib_translator.load("mlib_" + locale.name(), translations_dir))
			QCoreApplication::installTranslator(&mlib_translator);
		else
			MLIB_D("MLib's translations for '%1' did not found.", locale.name());
	}
}

}

