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


#include <algorithm>

#include <QtCore/QTimer>
#include <QtCore/QUrl>
// TODO
//#include <QtCore/QVariant>

#include <QtNetwork/QNetworkAccessManager>
// TODO
//#include <QtNetwork/QNetworkCookie>
//#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <src/common.hpp>

#include <src/client/reader.hpp>

#include "network_task.hpp"


namespace grov { namespace client { namespace reader {


Network_task::Network_task(Reader* reader, QObject* parent)
:
	Task(parent),

	reader(reader),
	fails_count(0),

	manager(new QNetworkAccessManager(this)),
	current_reply(NULL),
	timeout_timer(new QTimer(this))
{
	this->timeout_timer->setSingleShot(true);
	// TODO
	this->timeout_timer->setInterval(10 * 1000);
	connect(this->timeout_timer, SIGNAL(timeout()),
		this, SLOT(on_timeout()) );
}



void Network_task::get(const QString& url)
{
	MLIB_D("Processing a HTTP GET to '%1'...", url);

	QNetworkRequest request = this->prepare_request(url);
	this->process_reply(manager->get(request));
}



void Network_task::on_data_gotten(qint64 size, qint64 total_size)
{
	MLIB_DV("Data gotten: %1 of %2.", size, total_size);

	// If connection is not timed out yet
	if(this->timeout_timer->isActive())
	{
		// Reset the timeout
		this->timeout_timer->stop();
		this->timeout_timer->start();

		// Request's reply size limit -->
		{
			// TODO
			const qint64 max_size = 10 * 1024 * 1024;
			const qint64 reply_size = std::max(size, total_size);

			if(reply_size > max_size)
			{
				MLIB_D("Request's reply is too big (%1 bytes).", reply_size);
				// TODO: may be format to KB, MB, GB
				this->reply_error = _F(tr("Server returned too big amount of data (%1 bytes)."), reply_size);
				this->current_reply->abort();
			}
		}
		// Request's reply size limit <--
	}
}



void Network_task::on_finished(void)
{
	QNetworkReply* reply = this->current_reply;
	this->current_reply = NULL;

	MLIB_D("Request to '%1' finished.", reply->request().url().toString());
	this->timeout_timer->stop();

	QString reply_error;
	QByteArray reply_data;


	try
	{
		if(!this->reply_error.isEmpty())
			M_THROW(this->reply_error);

		if(reply->error())
			M_THROW(CSF(reply->errorString()));

		// Checking HTTP status code -->
		{
			int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

			if(code != 200)
				M_THROW("Server returned error: %1 (%2).",
					reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), code );
		}
		// Checking HTTP status code <--

// TODO
		// Getting cookies from the reply
//		this->reader->cookies->setCookiesFromUrl(
//			QNetworkCookie::parseCookies(reply->rawHeader("Set-Cookie")),
//			reply->request().url()
//		);

		reply_data = reply->readAll();
		MLIB_DV("Request reply:\n%1", reply_data);
	}
	catch(m::Exception& e)
	{
		this->fails_count++;
		reply_error = EE(e);
	}

	this->request_finished(reply_error, reply_data);
	reply->deleteLater();
}



void Network_task::on_timeout(void)
{
	MLIB_D("Request timed out.");
	this->reply_error = tr("Connection timed out.");
	this->current_reply->abort();
}



void Network_task::post(const QString& url, const QByteArray& data)
{
	MLIB_D("Processing a HTTP POST to '%1'...", url);
	QNetworkRequest request(url);
	// TODO
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	this->process_reply(this->manager->post(request, data));
}



QNetworkRequest Network_task::prepare_request(const QString& url)
{
	QNetworkRequest request(url);

	// TODO
//	request.setHeader(QNetworkRequest::CookieHeader,
//		qVariantFromValue(this->reader->cookies->cookiesForUrl(QUrl(url))) );

	// TODO
	request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.1.8) Gecko/20100214 Ubuntu/9.10 (karmic) Firefox/3.5.8");

	return request;
}



void Network_task::process_reply(QNetworkReply* reply)
{
	MLIB_A(!this->current_reply);

	this->current_reply = reply;
	this->reply_error.clear();
	this->timeout_timer->start();

	connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
		this, SLOT(on_data_gotten(qint64, qint64)) );

	connect(reply, SIGNAL(finished()),
		this, SLOT(on_finished()) );
}



bool Network_task::throw_if_fatal_error(const QString& error)
{
	if(error.isEmpty())
		return false;
	else
	{
		if(this->to_many_tries())
			M_THROW(error);
		else
			return true;
	}
}



bool Network_task::to_many_tries(void)
{
	return this->fails_count >= 3;
}


}}}

