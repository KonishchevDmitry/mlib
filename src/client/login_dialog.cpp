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


#include <src/common.hpp>

#include <mlib/gui/messages.hpp>

#include "login_dialog.hpp"
#include "ui_login_dialog.h"


namespace grov { namespace client {


Login_dialog::Login_dialog(QWidget *parent)
:
	QDialog(parent),
	ui(new Ui::Login_dialog)
{
    ui->setupUi(this);
}



Login_dialog::~Login_dialog()
{
    delete ui;
}



void Login_dialog::accept(void)
{
	if(this->login().isEmpty())
		m::gui::show_warning_message(this, tr("Invalid login"), tr("Please enter a valid login."));
	else if(this->password().isEmpty())
		m::gui::show_warning_message(this, tr("Invalid password"), tr("Please enter a valid password."));
	else
		this->done(Accepted);
}



QString Login_dialog::login(void)
{
	return ui->login->text().trimmed();
}



QString Login_dialog::password(void)
{
	return ui->password->text();
}



void Login_dialog::reject(void)
{
	this->done(Rejected);
}


}}

