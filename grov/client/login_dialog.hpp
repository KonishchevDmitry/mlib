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


#ifndef GROV_HEADER_CLIENT_LOGIN_DIALOG
#define GROV_HEADER_CLIENT_LOGIN_DIALOG

#include <QtGui/QDialog>

#include <grov/common.hpp>


namespace grov { namespace client {


namespace Ui {
    class Login_dialog;
}



/// Prompts Google Reader's login and password from user.
class Login_dialog : public QDialog
{
	Q_OBJECT

	public:
		Login_dialog(QWidget *parent = 0);
		~Login_dialog(void);


	private:
		// Qt Designer-generated widgets.
		Ui::Login_dialog	*ui;


	public:
		/// Returns login.
		QString	login(void);

		/// Returns password.
		QString	password(void);


	public slots:
		/// See QDialog::accept().
		virtual void	accept(void);

		/// See QDialog::reject().
		virtual void	reject(void);
};


}}

#endif

