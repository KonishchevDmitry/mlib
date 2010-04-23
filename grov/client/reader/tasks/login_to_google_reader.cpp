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


#include <QtCore/QUrl>

#include <src/common.hpp>
#include <src/main.hpp>

#include "login_to_google_reader.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Login_to_google_reader::Login_to_google_reader(const QString& login, const QString& password, QObject* parent)
:
	Network_task(parent),
	login(login),
	password(password)
{
}



QString Login_to_google_reader::get_auth_id(const QByteArray& reply)
{
	char auth_id_prefix[] = "Auth=";

	Q_FOREACH(const QByteArray& line, reply.split('\n'))
	{
		QByteArray entry = line.trimmed();

		if(entry.startsWith(auth_id_prefix))
			return entry.mid(sizeof auth_id_prefix - 1);
	}

	M_THROW(tr("Server did not return Google Reader's authentication id."));
}



void Login_to_google_reader::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Authentication request finished.");

	try
	{
		// Checking for errors -->
			if(this->throw_if_fatal_error(error))
			{
				MLIB_D("Request failed. Trying again...");
				this->process();
				return;
			}
		// Checking for errors <--

		// Getting Google Reader's authentication id
		// Throws m::Exception.
		QString auth_id = this->get_auth_id(reply);
		MLIB_D("Auth id gotten: '%1'.", auth_id);
		this->finish();
		emit this->authenticated(auth_id);
	}
	catch(m::Exception& e)
	{
		MLIB_D("Authentication error. %1", EE(e));
		this->failed(PAM( tr("Unable to login to Google Reader."), EE(e) ));
	}
}



void Login_to_google_reader::process(void)
{
	MLIB_D("Logining to Google Reader...");

#if GROV_OFFLINE_DEVELOPMENT
	emit this->request_finished("", "Auth=fake_offline_auth_id");
#else
	QString post_data = _F(
		"accountType=GOOGLE&"
		"Email=%1&"
		"Passwd=%2&"
		"service=reader&"
		"source=%3",
		QUrl::toPercentEncoding(this->login),
		QUrl::toPercentEncoding(this->password),
		QUrl::toPercentEncoding(get_user_agent())
	);

	this->post("https://www.google.com/accounts/ClientLogin", post_data);
#endif
}


}}}}

