/**************************************************************************
*                                                                         *
*   MLib - library of some useful things for internal usage               *
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


#ifndef MLIB_HEADER_DB_SCOPED_TRANSACTION
#define MLIB_HEADER_DB_SCOPED_TRANSACTION

#include <QtSql/QSqlDatabase>

#include <mlib/core.hpp>


namespace m { namespace db {


/// Starts transaction at construction and rollbacks it at destruction if it has
/// not been committed.
class Scoped_transaction: public QObject
{
	Q_OBJECT

	public:
		/// @throw m::Exception.
		Scoped_transaction(const QSqlDatabase& db);
		~Scoped_transaction(void);


	private:
		/// Database.
		QSqlDatabase	db;

		/// Is transaction closed.
		bool			closed;


	public:
		/// Commits this transaction.
		///
		/// @throw m::Exception.
		void	commit(void);
};


}}

#endif

