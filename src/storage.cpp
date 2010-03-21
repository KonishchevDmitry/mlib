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


#include <QtCore/QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <src/common.hpp>
#include <src/feed_item.hpp>

#include "storage.hpp"


namespace grov
{


// Scoped_transaction -->
	/// Starts transaction at construction and rollbacks it on destruction if
	/// it has not been committed.
	class Scoped_transaction
	{
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



	Scoped_transaction::Scoped_transaction(const QSqlDatabase& db)
	:
		db(db),
		closed(false)
	{
		MLIB_D("Starting a transaction...");

		if(!this->db.transaction())
			M_THROW(this->db.lastError().databaseText());
	}



	Scoped_transaction::~Scoped_transaction(void)
	{
		if(!this->closed)
		{
			MLIB_D("Rollbacking the transaction...");

			if(!this->db.rollback())
			{
				MLIB_SW(PAM(
					Storage::tr("Unable to rollback a transaction:"),
					this->db.lastError().databaseText()
				));
			}
		}
	}



	void Scoped_transaction::commit(void)
	{
		MLIB_D("Committing the transaction...");

		if(!this->db.commit())
			M_THROW(this->db.lastError().databaseText());

		this->closed = true;
	}
// Scoped_transaction <--



Storage::Storage(QObject* parent)
:
	QObject(parent),
	db(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")))
{
	// Opening database -->
		// TODO
		this->db->setDatabaseName("grov.db");

		MLIB_D("Opening database '%1'...", this->db->databaseName());

		if(!this->db->open())
		{
			M_THROW(PAM(
				_F(tr("Unable to open database '%1':"), this->db->databaseName()),
				this->db->lastError().databaseText()
			));
		}
	// Opening database <--

	// Creating all tables -->
		try
		{
			Scoped_transaction transaction(*this->db);

			this->exec(
				"CREATE TABLE labels("
					"id INTEGER PRIMARY KEY,"
					"name TEXT"
				")"
			);

			this->exec(
				"CREATE TABLE items("
					"id INTEGER PRIMARY KEY,"
					"title TEXT,"
					"summary TEXT"
				")"
			);

			this->exec(
				"CREATE TABLE labels_to_items("
					"label_id INTEGER,"
					"item_id INTEGER"
				")"
			);
			this->exec("CREATE INDEX labels_to_items_label_id ON labels_to_items(label_id)");

			transaction.commit();
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( tr("Unable to create table in the database:"), EE(e) ));
		}
	// Creating all tables <--
}



Storage::~Storage(void)
{
}



// TODO: add test throw
void Storage::add_items(const Feed_items_list& items)
{
	MLIB_D("Adding %1 items to DB...", items.size());

	try
	{
		QHash<QString, size_t> labels;

		// For SQLite it really speeds up many insertions.
		Scoped_transaction transaction(*this->db);

		// Getting all known labels -->
		{
			QSqlQuery query = this->exec(
				"SELECT id, name FROM labels" );

			while(query.next())
				labels[query.value(1).toString()] = query.value(0).toLongLong();
		}
		// Getting all known labels <--

		// Adding items -->
		{
			QSqlQuery insert_item_query = this->prepare(
				"INSERT INTO items (title, summary) "
					"values (:title, :summary)"
			);

			QSqlQuery insert_label_query = this->prepare(
				"INSERT INTO labels (name) values (:name)" );

			QSqlQuery insert_labels_to_items_query = this->prepare(
				"INSERT INTO labels_to_items (label_id, item_id) values (:label_id, :item_id)" );

			Q_FOREACH(const Feed_item& item, items)
			{
				insert_item_query.bindValue(":title", item.title);
				insert_item_query.bindValue(":summary", item.summary);
				this->exec(insert_item_query);
				Big_id item_id = insert_item_query.lastInsertId().toLongLong();

				// Labels -->
					Q_FOREACH(const QString& label, item.labels)
					{
						Big_id label_id;
						MLIB_CONST_ITER_TYPE(labels) iter = labels.find(label);

						if(iter == labels.end())
						{
							insert_label_query.bindValue(":name", label);
							this->exec(insert_label_query);
							label_id = insert_label_query.lastInsertId().toLongLong();

							labels[label] = label_id;
						}
						else
							label_id = iter.value();

						insert_labels_to_items_query.bindValue(":label_id", label_id);
						insert_labels_to_items_query.bindValue(":item_id", item_id);
						this->exec(insert_labels_to_items_query);
					}
				// Labels <--
			}
		}
		// Adding items <--

		transaction.commit();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to add feeds' items to the database:"), EE(e) ));
	}
}



void Storage::create_current_query(void)
{
	MLIB_D("Creating new current query...");

	this->reset();

	try
	{
		this->current_query = std::auto_ptr<QSqlQuery>(new QSqlQuery(
			this->prepare("SELECT title, summary FROM items") ));

		this->exec(*current_query);
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to query a feed's item from the database:"), EE(e) ));
	}
}



void Storage::exec(QSqlQuery& query)
{
	MLIB_D("Executing query '%1'...", query.lastQuery());

	if(!query.exec())
		M_THROW(query.lastError().databaseText());
}



QSqlQuery Storage::exec(const QString& query_string)
{
	QSqlQuery query(*this->db);
	query.prepare(query_string);
	this->exec(query);
	return query;
}



Feed_item Storage::get_item(bool next)
{
	if(!this->current_query.get())
		// Throws m::Exception
		this->create_current_query();

	bool exists;

	if(next)
		exists = this->current_query->next();
	else
	{
		exists = this->current_query->previous();

		if(!exists)
			exists = this->current_query->next();
	}

	if(exists)
	{
		return Feed_item(
			this->current_query->value(0).toString(),
			this->current_query->value(1).toString()
		);
	}
	else
		throw No_more_items();
}



Feed_item Storage::get_next_item(void)
{
	// Throws m::Exception
	return this->get_item(true);
}



Feed_item Storage::get_previous_item(void)
{
	// Throws m::Exception
	return this->get_item(false);
}



QSqlQuery Storage::prepare(const QString& string)
{
	QSqlQuery query(*this->db);

	if(!query.prepare(string))
		M_THROW(query.lastError().databaseText());

	return query;
}



void Storage::reset(void)
{
	MLIB_D("Reseting...");
	this->current_query.reset();
}


}


