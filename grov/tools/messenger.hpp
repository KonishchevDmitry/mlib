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


#ifndef GROV_HEADER_TOOLS_MESSENGER
#define GROV_HEADER_TOOLS_MESSENGER

#include <src/common.hpp>

#include "messenger.hxx"


namespace grov { namespace tools {


/// Shows messages to the user.
class Messenger: public QObject
{
	Q_OBJECT

	public:
		Messenger(QObject* parent = NULL);


	public:
		/// Shows a message.
		void	show(const char* file, int line, m::Message_type type, const QString& title, const QString& message);


	signals:
		/// Shows a message.
		void	message(const char* file, int line, m::Message_type type, const QString& title, const QString& message);


	private slots:
		/// Called when we get a message.
		void	on_message(const char* file, int line, m::Message_type type, const QString& title, const QString& message);
};


}}

#endif

