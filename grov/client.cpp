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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#if GROV_DEVELOP_MODE
	#include <QtCore/QFile>
#endif

#include <grov/common.hpp>

#include "client/login_dialog.hpp"
#include "client/reader.hpp"
#include "client/storage.hpp"

#include "client.hpp"


namespace grov
{


Client::Client(QObject* parent)
:
	Storage(parent),
	mode(MODE_NONE)
{
	// Throws m::Exception
	this->mode = this->get_mode();

	this->reader = new client::Reader(this, this);

	connect(this->reader, SIGNAL(cancelled()),
		this, SLOT(reader_cancelled()) );

	connect(this->reader, SIGNAL(error(const QString&)),
		this, SLOT(reader_error(const QString&)) );

	connect(this->reader, SIGNAL(reading_list_gotten()),
		this, SLOT(offline_data_gotten()) );

	connect(this->reader, SIGNAL(offline_data_flushed()),
		this, SLOT(offline_data_flushed()) );
}



void Client::cancel_current_task(void)
{
	this->reader->cancel();
}



void Client::change_mode(Mode mode)
{
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
		this->set_mode(MODE_NONE);
		this->change_mode(MODE_NONE);
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Discarding all offline data failed"), EE(e));
		return;
	}

	try
	{
		this->clear();
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Error while discarding all offline data"), EE(e));
	}
}



void Client::flush_offline_data(void)
{
	MLIB_A(this->mode == MODE_OFFLINE);

	QString login;
	QString password;

	if(this->get_login_data(&login, &password))
	{
		try
		{
			// Throws m::Exception
			this->prepare_to_flush_offline_data();

			this->change_mode(MODE_GOING_NONE);
			this->reader->flush_offline_data(login, password);
		}
		catch(m::Exception& e)
		{
			MLIB_W(tr("Error while flushing offline data"), EE(e));
		}
	}
}



bool Client::get_login_data(QString* login, QString* password)
{
#if GROV_OFFLINE_DEVELOPMENT
	*login = "fake_offline_login";
	*login = "fake_offline_password";
	return true;
#else
	#if GROV_DEVELOP_MODE
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
	#if GROV_DEVELOP_MODE
		}
	#endif
#endif
}



Client::Mode Client::get_mode(void)
{
	try
	{
		QString mode = Storage::get_mode();

		if(mode == "" || mode == "none")
			return MODE_NONE;
		else if(mode == "offline")
			return MODE_OFFLINE;
		else
			M_THROW(tr("Invalid mode '%1'."), mode);
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to get current application mode from the database:"), EE(e) ));
	}
}



void Client::go_offline(void)
{
	MLIB_A(this->mode == MODE_NONE);

	QString login;
	QString password;

	if(this->get_login_data(&login, &password))
	{
		try
		{
			// In case of an error in the past.
			// Throws m::Exception.
			this->clear();

			this->change_mode(MODE_GOING_OFFLINE);
			this->start_editing();
			this->reader->get_reading_list(login, password);
		}
		catch(m::Exception& e)
		{
			MLIB_W(tr("Unable to go offline"), EE(e));
		}
	}
}



void Client::offline_data_flushed(void)
{
	MLIB_A(this->mode == MODE_GOING_NONE);

	try
	{
		try
		{
			this->set_mode(MODE_NONE);
			this->change_mode(MODE_NONE);
		}
		catch(m::Exception& e)
		{
			this->change_mode(MODE_OFFLINE);
			throw;
		}

		this->clear();
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Error while flushing offline data"), EE(e));
	}
}



void Client::offline_data_gotten(void)
{
	MLIB_D("Offline data gotten.");

	MLIB_A(this->mode == MODE_GOING_OFFLINE);

	try
	{
		this->end_editing();
		this->set_mode(MODE_OFFLINE);
		this->change_mode(MODE_OFFLINE);
	}
	catch(m::Exception& e)
	{
		this->change_mode(MODE_NONE);
		MLIB_W(tr("Unable to go offline"), EE(e));
	}
}



void Client::reader_cancelled(void)
{
	MLIB_D("Operation cancelled.");

	switch(this->current_mode())
	{
		case MODE_GOING_OFFLINE:
		{
			try
			{
				this->cancel_editing();
				this->clear();
			}
			catch(m::Exception& e)
			{
				MLIB_SW(tr("Discarding all offline data failed"), EE(e));
			}

			this->change_mode(MODE_NONE);
		}
		break;

		case MODE_GOING_NONE:
			this->change_mode(MODE_OFFLINE);
			break;

		default:
			MLIB_LE();
			break;
	}
}



void Client::reader_error(const QString& message)
{
	MLIB_D("Reader error: '%1'.", message);

	QString title;

	switch(this->current_mode())
	{
		case MODE_GOING_OFFLINE:
			title = tr("Unable to go offline");
			break;

		case MODE_GOING_NONE:
			title = tr("Error while flushing offline data");
			break;

		default:
			MLIB_LE();
			break;
	}

	this->reader_cancelled();

	MLIB_W(title, message);
}



void Client::set_mode(Mode mode)
{
	try
	{
		QString mode_name;

		switch(mode)
		{
			case MODE_NONE:
				mode_name = "none";
				break;

			case MODE_OFFLINE:
				mode_name = "offline";
				break;

			default:
				M_THROW(tr("Logical error."));
				break;
		}

		Storage::set_mode(mode_name);
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to write current application mode to the database:"), EE(e) ));
	}
}


}

