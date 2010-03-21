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


#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSslError>

#include <src/common.hpp>
#include <src/feed_item.hpp>
#include <src/items_list_parser.hpp>
#include <src/storage.hpp>

#include "reader_impl.hpp"

//TODO
#include <stdio.h>
//TODO: add timeouts


namespace grov
{


/// Maximum attempts to accomplish a task.
const size_t MAX_ATTEMPTS = 3;



bool Reader_impl::Task::fail(void)
{
	return ++this->tries > MAX_ATTEMPTS;
};



Reader_impl::Reader_impl(Storage* storage, const QString& user, const QString& password, QObject* parent)
:
	QObject(parent),

	storage(storage),

	user(user),
	password(password),

	authenticated(false),

	manager(new QNetworkAccessManager(this))
{
}



bool Reader_impl::authenticate(void)
{
	if(this->authenticated)
		return true;

	MLIB_D("Logining to Google Reader...");

	QNetworkRequest request = this->prepare_request();

	request.setUrl(QUrl("https://www.google.com/accounts/ClientLogin"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	// TODO another fields
	QString post_request =
		"Email=" + QUrl::toPercentEncoding(this->user) + "&"
		"Passwd=" + QUrl::toPercentEncoding(this->password);

	QNetworkReply *reply = manager->post(request, post_request.toAscii());
	connect(reply, SIGNAL(finished()), this, SLOT(on_authentication_finished()));
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
		this, SLOT(on_data_gotten(qint64, qint64)) );

	return false;
}



void Reader_impl::check_reply_status(QNetworkReply* reply)
{
	int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if(code == 200)
		return;

	M_THROW("%1 (%2)", reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(), code );
}



void Reader_impl::get_reading_list(void)
{
	MLIB_D("Queuing 'get reading list' task...");
	this->queue(Task(Task::GET_READING_LIST));
}



// TODO: more tries on errors
void Reader_impl::on_authentication_finished(void)
{
	MLIB_D("Authentication request finished.");

	QNetworkReply* reply = m::checked_qobject_cast<QNetworkReply*>(this->sender());

	try
	{
		// Authentication reply error
		if(reply->error())
			M_THROW(tr("Authentication error. %1"), CSF(reply->errorString()));
		// If authentication is successful
		else
		{
			QString reply_text = reply->readAll();

// TODO: enable or delete
			MLIB_DV("Authentication reply:\n%1", reply_text);

			// Checking HTTP status code -->
				try
				{
					this->check_reply_status(reply);
				}
				catch(m::Exception& e)
				{
					M_THROW(tr("Server returned error: %1."), EE(e));
				}
			// Checking HTTP status code <--

			// Getting Google Reader's SID -->
			{
				QString sid_prefix = "SID=";
				QStringList reply_lines = reply_text.split('\n');

				this->sid.clear();

				Q_FOREACH(const QString& string, reply_lines)
				{
					QString entry = string.trimmed();

					if(entry.startsWith(sid_prefix))
					{
						this->sid = entry.mid(sid_prefix.size());
						break;
					}
				}

				if(this->sid.isEmpty())
					M_THROW(tr("Server did not return Google Reader session id."));
			}
			// Getting Google Reader's SID <--

			// Now we are authenticated
			this->authenticated = true;

			// Processing pending tasks
			this->process();
		}
	}
	catch(m::Exception& e)
	{
		MLIB_D("Authentication error. %1", EE(e));

		// Cancelling all tasks -->
			while(!this->tasks.empty())
			{
				this->tasks.dequeue();

				emit error(_F( tr("Unable to login to Google Reader. %1"), EE(e) ));
			}
		// Cancelling all tasks <--
	}

	reply->deleteLater();
}



// TODO: max size limit
void Reader_impl::on_data_gotten(qint64 size, qint64 total_size)
{
	MLIB_D("Data gotten: %1 of %2.", size, total_size);
}



void Reader_impl::on_query_finished(void)
{
	MLIB_D("Task's query finished.");

	bool dequeue_task = true;
	QNetworkReply* reply = m::checked_qobject_cast<QNetworkReply*>(this->sender());

	try
	{
		if(reply->error())
		{
			MLIB_D(PAM( "Query error:", reply->errorString() ));
			// TODO: more descriptive
			M_THROW(PAM( "Query error:", reply->errorString() ));
		}
		else
		{
			QByteArray reply_text = reply->readAll();

// TODO: enable or delete
			MLIB_DV("Query reply:\n%1", reply_text);

			// Checking HTTP status code -->
				try
				{
					this->check_reply_status(reply);
				}
				catch(m::Exception& e)
				{
					M_THROW(tr("Server returned error: %1."), EE(e));
				}
			// Checking HTTP status code <--

			// Getting feeds' items -->
			{
				// TODO: exception
				Feed_items_list items = Items_list_parser().parse(reply_text);
				// TODO: may be move out (not retry task on fail)

				// Throws m::Exception.
				this->storage->add_items(items);

				// TODO
				emit reading_list(items);
			}
			// Getting feeds' items <--
		}
	}
	catch(m::Exception& e)
	{
		MLIB_D("Task's query failed.");

		if(this->tasks.head().fail())
			emit error(EE(e));
		else
		{
			MLIB_D("Trying to process this task again...");
			dequeue_task = false;
		}
	}

	if(dequeue_task)
		this->tasks.dequeue();

	reply->deleteLater();

	this->process();
	// TODO
//	MLIB_D("Exiting...");
//	exit(0);
}



QNetworkRequest Reader_impl::prepare_request(void)
{
	QNetworkRequest request;

	if(this->authenticated)
	{
		QNetworkCookie cookie("SID", this->sid.toAscii());
		cookie.setDomain(".google.com");
		cookie.setPath("/");

		QList<QNetworkCookie> cookies;
		cookies << cookie;
		request.setHeader(QNetworkRequest::CookieHeader, qVariantFromValue(cookies));
	}

	// TODO: timeout
	// TODO: user agent
	//	request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
	return request;
}



void Reader_impl::process(void)
{
	// There is no tasks to process
	if(this->tasks.empty())
		return;

	// If we are not authenticated we must authenticate first
	if(!this->authenticate())
		return;


	Task& task = this->tasks.head();
	MLIB_D("Processing task %1...", task.type());

	QNetworkRequest request = this->prepare_request();

	switch(task.type())
	{
		case Task::GET_READING_LIST:
			request.setUrl(QUrl("http://www.google.com/reader/atom/user/-/state/com.google/reading-list?n=10"));
			break;

		default:
			MLIB_LE();
			break;
	}

	QNetworkReply *reply = manager->get(request);
	connect(reply, SIGNAL(finished()), this, SLOT(on_query_finished()));
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
		this, SLOT(on_data_gotten(qint64, qint64)) );
}



void Reader_impl::queue(const Task& task)
{
	bool no_tasks = this->tasks.empty();

	this->tasks.enqueue(task);

	if(no_tasks)
		this->process();
}


}

