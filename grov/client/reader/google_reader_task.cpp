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


#include <QtNetwork/QNetworkRequest>

#include <grov/common.hpp>

#include "tasks/get_gr_token.hpp"
#include "tasks/login_to_google_reader.hpp"

#include "google_reader_task.hpp"


namespace grov { namespace client { namespace reader {


Google_reader_task::Google_reader_task(const QString& auth_id, QObject* parent)
:
	Network_task(parent),
	auth_id(auth_id)
{
}



Google_reader_task::Google_reader_task(const QString& login, const QString& password, QObject* parent)
:
	Network_task(parent),
	login(login),
	password(password)
{
}



void Google_reader_task::get_token(void)
{
	tasks::Get_gr_token* task = new tasks::Get_gr_token(this->auth_id, this);

	connect(task, SIGNAL(token_gotten(const QString&)),
		this, SLOT(on_token_gotten(const QString&)), Qt::QueuedConnection );

	this->process_task(task);
}



void Google_reader_task::on_authenticated(const QString& auth_id)
{
	this->auth_id = auth_id;
	this->authenticated();
}



void Google_reader_task::on_token_gotten(const QString& token)
{
	this->token = token;
	this->token_gotten();
}



QNetworkRequest Google_reader_task::prepare_request(const QString& url)
{
	QNetworkRequest request = Network_task::prepare_request(url);
	if(!this->auth_id.isEmpty())
		request.setRawHeader("Authorization", "GoogleLogin auth=" + auth_id.toAscii());
	return request;
}



void Google_reader_task::process(void)
{
	if(this->auth_id.isEmpty())
	{
		tasks::Login_to_google_reader* task =
			new tasks::Login_to_google_reader(this->login, this->password, this);

		connect(task, SIGNAL(authenticated(const QString&)),
			this, SLOT(on_authenticated(const QString&)), Qt::QueuedConnection );

		this->process_task(task);
	}
	else
		this->authenticated();
}


}}}

