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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#if DEVELOP_MODE
	#include <QtCore/QFile>
#endif

#include <src/common.hpp>

#include "client/login_dialog.hpp"
#include "client/reader.hpp"
#include "client/storage.hpp"

#include "client.hpp"


namespace grov
{

// TODO: task cancelled

Client::Client(QObject* parent)
:
	Storage(parent),
	mode(MODE_NONE)
{
	// Throws m::Exception
	if(this->has_items())
		this->mode = MODE_OFFLINE;

	this->reader = new client::Reader(this, this);

	connect(this->reader, SIGNAL(error(const QString&)),
		this, SLOT(reader_error(const QString&)) );

	connect(this->reader, SIGNAL(reading_list_gotten()),
		this, SLOT(offline_data_gotten()) );

	connect(this->reader, SIGNAL(offline_data_flushed()),
		this, SLOT(offline_data_flushed()) );
}



void Client::change_mode(Mode mode)
{
	// TODO: write current mode to the DB to save as from DB clean errors and call clear before mode changes
	this->mode = mode;
	emit this->mode_changed(mode);
}



Client::Mode Client::current_mode(void)
{
	return this->mode;
}



void Client::discard_offline_data(void)
{
	MLIB_A(this->mode == MODE_OFFLINE);

	try
	{
		this->clear();
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Discarding all offline data failed"), EE(e));
	}

	this->change_mode(MODE_NONE);
}



void Client::flush_offline_data(void)
{
	MLIB_A(this->mode == MODE_OFFLINE);

// reset, etc
	QString login;
	QString password;

	if(this->get_login_data(&login, &password))
	{
		this->change_mode(MODE_GOING_NONE);
		this->reader->flush_offline_data(login, password);
	}
}



bool Client::get_login_data(QString* login, QString* password)
{
#if OFFLINE_DEVELOPMENT
	*login = "fake_offline_login";
	*login = "fake_offline_password";
	return true;
#else
	#if DEVELOP_MODE
		QFile login_data("login_data");

		if(login_data.exists() && login_data.open(QIODevice::ReadOnly))
		{
			*login = login_data.readLine();
			*password = login_data.readLine();
			return true;
		}
		else
		{
	#endif
			client::Login_dialog dialog;

			if(dialog.exec())
			{
				*login = dialog.login();
				*password = dialog.password();
				return true;
			}
			else
				return false;
	#if DEVELOP_MODE
		}
	#endif
#endif
}



void Client::go_offline(void)
{
	MLIB_A(this->mode == MODE_NONE);

	QString login;
	QString password;

	if(this->get_login_data(&login, &password))
	{
		this->change_mode(MODE_GOING_OFFLINE);
		this->reader->get_reading_list(login, password);
	}
}



void Client::offline_data_flushed(void)
{
	MLIB_A(this->mode == MODE_GOING_NONE);
	this->clear(); // TODO: exception
	this->change_mode(MODE_NONE);
	// TODO: clear, etc
}



void Client::offline_data_gotten(void)
{
	MLIB_A(this->mode == MODE_GOING_OFFLINE);
	this->change_mode(MODE_OFFLINE);
}



// TODO
void Client::reader_cancelled(void)
{
}



// TODO
void Client::reader_error(const QString& message)
{
	Mode new_mode;
	QString title;

	switch(this->mode)
	{
		case MODE_GOING_OFFLINE:
		{
			try
			{
				this->clear();
			}
			catch(m::Exception& e)
			{
				MLIB_SW(tr("Discarding all offline data failed"), EE(e));
			}

			new_mode = MODE_NONE;
			title = tr("Unable to go offline");
		}
		break;

		case MODE_GOING_NONE:
		{
			new_mode = MODE_OFFLINE;
			title = tr("Error while flushing offline data");
		}
		break;

		default:
			MLIB_LE();
			break;
	}

	MLIB_W(title, message);
	this->change_mode(new_mode);
}


}

