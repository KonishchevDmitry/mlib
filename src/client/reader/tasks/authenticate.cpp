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


#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>

#include "authenticate.hpp"


namespace grov { namespace client { namespace reader { namespace tasks {


Authenticate::Authenticate(const QNetworkRequest& request_template, const QString& user, const QString& password, QObject* parent)
:
	Network_task(request_template, parent),
	user(user),
	password(password)
{
}



void Authenticate::request_finished(const QString& error, const QByteArray& reply)
{
	MLIB_D("Authentication request finished.");

	try
	{
		// Checking for errors -->
			if(!error.isEmpty())
			{
				if(this->to_many_tries())
					M_THROW(error);
				else
				{
					MLIB_D("Authentication request failed. Trying again...");
					this->process();
					return;
				}
			}
		// Checking for errors <--

		QString sid;

		// Getting Google Reader's SID -->
		{
			char sid_prefix[] = "SID=";

			Q_FOREACH(const QByteArray& line, reply.split('\n'))
			{
				QByteArray entry = line.trimmed();

				if(entry.startsWith(sid_prefix))
				{
					sid = entry.mid(sizeof sid_prefix - 1);
					break;
				}
			}

			if(sid.isEmpty())
				M_THROW(tr("Server did not return Google Reader session id."));
		}
		// Getting Google Reader's SID <--

		emit this->authenticated(sid);
	}
	catch(m::Exception& e)
	{
		MLIB_D("Authentication error. %1", EE(e));
		emit this->error(_F( tr("Unable to login to Google Reader. %1"), EE(e) ));
	}
// TODO: -->
//	Q_FOREACH(QByteArray header, reply->rawHeaderList())
//		MLIB_D("%1: %2", header, reply->rawHeader(header));
//	qDebug() << reply->readAll();
//	QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(reply->rawHeader("Set-Cookie"));
//	qDebug() << cookies;
	//		this->request_template.setHeader(QNetworkRequest::CookieHeader, qVariantFromValue(cookies));
// TODO: <--
}



void Authenticate::process(void)
{
	MLIB_D("Logining to Google Reader...");

	QNetworkRequest request = this->request_template;

	request.setUrl(QUrl("https://www.google.com/accounts/ClientLogin"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	// TODO another fields
	QString post_request =
		"Email=" + QUrl::toPercentEncoding(this->user) + "&"
		"Passwd=" + QUrl::toPercentEncoding(this->password);

	this->post(request, post_request.toAscii());
}


}}}}

