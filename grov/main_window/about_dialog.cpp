/**************************************************************************
*                                                                         *
*   Grov - Google Reader offline viewer                                   *
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


#include <QtCore/QDate>
#include <QtCore/QUrl>

#include <QtGui/QIcon>

#include <grov/common.hpp>

#include "about_dialog.hpp"
#include "ui_about_dialog.h"


namespace grov { namespace main_window {


About_dialog::About_dialog(QWidget *parent)
:
	QDialog(parent),
	ui(new Ui::About_dialog)
{
    ui->setupUi(this);

	// About page -->
	{
		ui->app_icon->setPixmap(this->windowIcon().pixmap(QSize(48, 48)));
		ui->app_info->setText(_F("<b><big>%1</big><br/>%2</b>", GROV_APP_NAME, GROV_VERSION));


		QString copyright_date;

		// Getting copyright date string -->
		{
			int start_year = 2010;
			int current_year = QDate::currentDate().year();
			copyright_date = QString::number(start_year);

			if(start_year != current_year)
				copyright_date += "-" +QString::number(current_year);
		}
		// Getting copyright date string <--

		ui->about->setText(
			_F( tr("%1 - Google Reader offline viewer"), GROV_APP_NAME ) +
			_F( "<p>Copyright © %1, %2</p>", copyright_date, tr("Dmitry Konishchev") ) +
			_F( "<p><a href='%1'>%1</a></p>", "https://sourceforge.net/projects/grov/" )
		);
	}
	// About page <--

	// Authors page -->
		ui->authors->setText(_F(
			"%1"
			"<p style='margin-top: 0px; margin-left: 1em'>"
				"<a href='mailto:%2?subject=%3'>%4</a><br/>"
				"<a href='%5'>%5</a>"
			"</p>",
			tr("Dmitry Konishchev"),
			QUrl::toPercentEncoding("Dmitry Konishchev <konishchev@gmail.com>"),
			QUrl::toPercentEncoding(GROV_APP_NAME),
			"konishchev@gmail.com",
			"http://konishchevdmitry.blogspot.com/"
		));
	// Authors page <--

	// Thanks To page -->
		ui->thanks_to->setText(_F(
			tr("Thanks to %1 for the application's icons."),
			"<a href='http://gnome-look.org/content/show.php/Tango+Google+Reader+Icon?content=84565'>Tango Google Reader Icon</a>"
		));
	// Thanks To page <--
}



About_dialog::~About_dialog()
{
}


}}

