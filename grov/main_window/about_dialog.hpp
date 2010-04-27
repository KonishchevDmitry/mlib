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


#ifndef GROV_HEADER_MAIN_WINDOW_ABOUT_DIALOG
#define GROV_HEADER_MAIN_WINDOW_ABOUT_DIALOG

#include <boost/scoped_ptr.hpp>

#include <QtGui/QDialog>

#include <grov/common.hpp>


namespace grov { namespace main_window {
namespace Ui { class About_dialog; }


/// About application dialog.
class About_dialog: public QDialog
{
	Q_OBJECT

	public:
		About_dialog(QWidget *parent = 0);
		~About_dialog(void);


	private:
		// Qt Designer-generated widgets.
		boost::scoped_ptr<Ui::About_dialog>	ui;
};


}}

#endif

