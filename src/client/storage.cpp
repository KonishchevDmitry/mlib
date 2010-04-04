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


#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <src/common.hpp>
#include <src/common/feed_item.hpp>
#include <src/common/feed_tree.hpp>

#include "storage.hpp"


namespace grov { namespace client {


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
			MLIB_D("Rolling back the transaction...");

			if(!this->db.rollback())
			{
				MLIB_SW(PAM(
					grov::client::Storage::tr("Unable to rollback a transaction:"),
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



const Big_id Storage::NO_LABEL_ID;


Storage::Storage(QObject* parent)
:
	QObject(parent),
	db(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"))),
	current_source(SOURCE_NONE)
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
		if(this->db->tables().empty())
		{
		// TODO: selects and indexes optimization
			try
			{
				Scoped_transaction transaction(*this->db);

				this->exec(
					"CREATE TABLE config("
						"name TEXT,"
						"value TEXT"
					")"
				);
				this->exec("INSERT INTO config VALUES ('version', 1)");

				this->exec(
					"CREATE TABLE feeds("
						"id INTEGER PRIMARY KEY,"
						"name TEXT,"
						"uri TEXT"
					")"
				);

				this->exec(
					"CREATE TABLE labels("
						"id INTEGER PRIMARY KEY,"
						"name TEXT"
					")"
				);

				this->exec(
					"CREATE TABLE items("
						"id INTEGER PRIMARY KEY,"
						"gr_id TEXT,"
						"feed_id INTEGER,"
						"title TEXT,"
						"summary TEXT,"
						"read DEFAULT 0,"
						"starred DEFAULT 0," // TODO
						"changes DEFAULT 0" // TODO
					")"
				);
				this->exec("CREATE INDEX items_feed_id_read_idx ON items(feed_id, read)");

				this->exec(
					"CREATE TABLE labels_to_items("
						"label_id INTEGER,"
						"item_id INTEGER"
					")"
				);
				this->exec("CREATE INDEX labels_to_items_label_id_idx ON labels_to_items(label_id)");

				this->exec(
					"CREATE TABLE labels_to_feeds("
						"label_id INTEGER,"
						"feed_id INTEGER"
					")"
				);
				this->exec("CREATE INDEX labels_to_feeds_label_id_idx ON labels_to_feeds(label_id)");

				transaction.commit();
			}
			catch(m::Exception& e)
			{
				M_THROW(PAM( tr("Unable to create table in the database:"), EE(e) ));
			}
		}
	// Creating all tables <--
}



Storage::~Storage(void)
{
	// TODO: flush cache, etc
}



// TODO: add test throw
void Storage::add_items(const Feed_items_list& items)
{
	MLIB_D("Adding %1 items to DB...", items.size());

	try
	{
		#warning QMap?
		QHash<QString, Big_id> feeds;
		QHash<QString, Big_id> labels;
		QHash< Big_id, QSet<Big_id> > labels_to_feeds;

		// For SQLite it really speeds up many insertions.
		Scoped_transaction transaction(*this->db);

		// Getting all known feeds -->
		{
			QSqlQuery query = this->exec(
				"SELECT id, uri FROM feeds" );

			while(query.next())
				feeds[query.value(1).toString()] = m::qvariant_to_big_id(query.value(0));
		}
		// Getting all known feeds <--

		// Getting all known labels -->
		{
			QSqlQuery query = this->exec(
				"SELECT id, name FROM labels" );

			while(query.next())
				labels[query.value(1).toString()] = m::qvariant_to_big_id(query.value(0));
		}
		// Getting all known labels <--

		// Adding items -->
		{
			QSqlQuery insert_item_query = this->prepare(
				"INSERT INTO items (gr_id, feed_id, title, summary, starred) "
					"values (:gr_id, :feed_id, :title, :summary, :starred)"
			);


			QSqlQuery insert_feed_query = this->prepare(
				"INSERT INTO feeds (name, uri) values (:name, :uri)" );

			QSqlQuery insert_labels_to_feeds_query = this->prepare(
				"INSERT INTO labels_to_feeds (label_id, feed_id) values (:label_id, :feed_id)" );


			QSqlQuery insert_label_query = this->prepare(
				"INSERT INTO labels (name) values (:name)" );

			QSqlQuery insert_labels_to_items_query = this->prepare(
				"INSERT INTO labels_to_items (label_id, item_id) values (:label_id, :item_id)" );


			Q_FOREACH(const Feed_item& item, items)
			{
				Big_id feed_id;

				// Feed -->
				{
					MLIB_CONST_ITER_TYPE(feeds) iter = feeds.find(item.feed_uri);

					if(iter == feeds.end())
					{
						insert_feed_query.bindValue(":name", item.feed_name);
						insert_feed_query.bindValue(":uri", item.feed_uri);
						this->exec(insert_feed_query);
						feed_id = m::qvariant_to_big_id(insert_feed_query.lastInsertId());

						feeds[item.feed_uri] = feed_id;
					}
					else
						feed_id = iter.value();
				}
				// Feed <--

				insert_item_query.bindValue(":gr_id", item.gr_id);
				insert_item_query.bindValue(":feed_id", feed_id);
				insert_item_query.bindValue(":title", item.title);
				insert_item_query.bindValue(":summary", item.summary);
				insert_item_query.bindValue(":starred", item.labels.contains("starred") == true);
				this->exec(insert_item_query);

				Big_id item_id = m::qvariant_to_big_id(insert_item_query.lastInsertId());

				// Labels -->
					Q_FOREACH(const QString& label, item.labels)
					{
						Big_id label_id;
						MLIB_CONST_ITER_TYPE(labels) iter = labels.find(label);

						// Labels -->
							if(iter == labels.end())
							{
								insert_label_query.bindValue(":name", label);
								this->exec(insert_label_query);
								label_id = m::qvariant_to_big_id(insert_label_query.lastInsertId());

								labels[label] = label_id;
							}
							else
								label_id = iter.value();
						// Labels <--

						// Labels to items -->
							insert_labels_to_items_query.bindValue(":label_id", label_id);
							insert_labels_to_items_query.bindValue(":item_id", item_id);
							this->exec(insert_labels_to_items_query);
						// Labels to items <--

						// Labels to feeds -->
						{
							MLIB_CONST_ITER_TYPE(labels_to_feeds) iter = labels_to_feeds.find(label_id);

							if(iter == labels_to_feeds.end() || !iter->contains(feed_id))
							{
								insert_labels_to_feeds_query.bindValue(":label_id", label_id);
								insert_labels_to_feeds_query.bindValue(":feed_id", feed_id);
								this->exec(insert_labels_to_feeds_query);

								labels_to_feeds[label_id].insert(feed_id);
							}
						}
						// Labels to feeds <--
					}
				// Labels <--
			}
		}
		// Adding items <--

		transaction.commit();

		// TODO: odd emits
		emit feed_tree_changed();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to add feeds' items to the database:"), EE(e) ));
	}
}



void Storage::clear(void)
// TODO: VACUUM
{
	this->reset();
	this->clear_cache();

	try
	{
		Scoped_transaction transaction(*this->db);

		this->exec("DELETE FROM feeds");
		this->exec("DELETE FROM labels");
		this->exec("DELETE FROM items");
		this->exec("DELETE FROM labels_to_items");
		this->exec("DELETE FROM labels_to_feeds");

		// Starred items is a special label - it must always exists.
		this->exec("INSERT INTO labels (name) VALUES ('starred')");

		transaction.commit();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to delete data from the database:"), EE(e) ));
	}

	emit this->feed_tree_changed();
}



void Storage::clear_cache(void)
{
	this->readed_items_cache.clear();
}



void Storage::create_current_query(void)
{
	MLIB_D("Creating new current query...");

	// Throws m::Exception
	this->flush_cache();


	QSqlQuery query;

	switch(this->current_source)
	{
		case SOURCE_FEED:
			query = this->prepare(
				"SELECT "
					"id, feed_id, title, summary, starred "
				"FROM "
					"items "
				"WHERE "
					"feed_id = :source_id AND "
					"read = 0 "
				"ORDER BY "
					"id"
			);
			break;

		case SOURCE_LABEL:
			query = this->prepare(
				"SELECT "
					"id, feed_id, title, summary, starred "
				"FROM "
					"items, labels_to_items "
				"WHERE "
					"label_id = :source_id AND "
					"items.id = item_id AND "
					"read = 0 "
				"ORDER BY "
					"id"
			);
			break;

		default:
			M_THROW(tr("Logical error (invalid item's source type)."));
			break;
	}

	query.bindValue(":source_id", this->current_source_id);

	// Throws m::Exception
	this->exec(query);

	this->current_query.reset(new QSqlQuery(query));
}



void Storage::exec(QSqlQuery& query)
{
	MLIB_DV("Executing query '%1'...", query.lastQuery());

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



void Storage::flush_cache(void)
{
	if(this->readed_items_cache.empty())
		return;

	try
	{
		Scoped_transaction transaction(*this->db);

		QSqlQuery query = this->prepare(
			"UPDATE "
				"items "
			"SET "
				"read = 1 "
			"WHERE "
				"id = :id"
		);

		Q_FOREACH(Big_id id, this->readed_items_cache)
		{
			query.bindValue(":id", id);
			this->exec(query);
		}

		transaction.commit();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to mark feed's item(s) as read:", "", !this->readed_items_cache.isEmpty()), EE(e) ));
	}

	this->readed_items_cache.clear();
}



// TODO: feeds without labels
Feed_tree Storage::get_feed_tree(void)
{
	MLIB_D("Getting feed tree...");

	// Throws m::Exception
	this->flush_cache();

	try
	{
		Feed_tree feed_tree = Feed_tree::create();

		QSqlQuery labels_query = this->exec(
			"SELECT id, name FROM labels" );

		QSqlQuery labels_feeds_query = this->prepare(
			"SELECT "
				"feeds.id, feeds.name "
			"FROM "
				"feeds, labels_to_feeds "
			"WHERE "
				"label_id = :label_id AND feeds.id = feed_id"
		);

		QSqlQuery labels_feeds_items_query = this->prepare(
			"SELECT "
				"COUNT(*) "
			"FROM "
				"labels_to_items, items "
			"WHERE "
				"item_id = items.id AND feed_id = :feed_id AND read = 0"
		);

		while(labels_query.next())
		{
			Big_id label_id = m::qvariant_to_big_id(labels_query.value(0));
			QString label_name = labels_query.value(1).toString();

			Feed_tree_item* label = feed_tree.add_label(label_id, label_name);
			labels_feeds_query.bindValue(":label_id", label_id);
			this->exec(labels_feeds_query);

			while(labels_feeds_query.next())
			{
				Big_id feed_id = m::qvariant_to_big_id(labels_feeds_query.value(0));
				QString feed_name = labels_feeds_query.value(1).toString();

				labels_feeds_items_query.bindValue(":feed_id", feed_id);
				this->exec(labels_feeds_items_query);
				if(!labels_feeds_items_query.next())
					M_THROW(tr("Query did not return a value."));

				Feed_tree_item* feed = label->add_feed(feed_id, feed_name);
				feed->unread_items += m::qvariant_to_big_id(labels_feeds_items_query.value(0));
				label->unread_items += feed->unread_items;
			}
		}

		MLIB_D("Feed tree gotten.");

		return feed_tree;
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to get feed tree from the database:"), EE(e) ));
	}
}



Feed_item Storage::get_item(bool next)
{
	MLIB_D("Getting next(%1) item...", next);

	if(this->current_source == SOURCE_NONE)
		throw No_selected_items();

	try
	{
		if(!this->current_query.get())
			this->create_current_query();

		bool exists;
		QSqlQuery* query = this->current_query.get();

		if(next)
			exists = query->next();
		else
		{
			exists = query->previous();

			if(!exists)
				exists = query->next();
		}

		if(exists)
		{
			Big_id id = m::qvariant_to_big_id(query->value(0));

			Feed_item item(
				id, m::qvariant_to_big_id(query->value(1)),
				query->value(2).toString(),
				query->value(3).toString(),
				this->current_query_star_cache.value(id, query->value(4).toBool())
			);

			MLIB_D("Item gotten.");

			return item;
		}
		else
			throw No_more_items();
	}
	catch(m::Exception& e)
	{
		this->reset();
		M_THROW(PAM( tr("Unable to query a feed's item from the database:"), EE(e) ));
	}
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



bool Storage::has_items(void)
{
	try
	{
		QSqlQuery query = this->exec("SELECT COUNT(*) FROM items");
		return query.next() && m::qvariant_to_big_id(query.value(0));
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to query feed's items from the database:"), EE(e) ));
	}
}



void Storage::mark_as_read(const Feed_item& item)
{
	MLIB_D("Marking item [%1] as read...", item.id);

	this->readed_items_cache << item.id;

	if(readed_items_cache.size() > 10)
		// Throws m::Exception
		this->flush_cache();

	// TODO
	emit this->item_marked_as_read(QList<Big_id>(), item.feed_id, true);

	MLIB_D("Item [%1] marked as read.", item.id);
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
	this->current_query_star_cache.clear();
}



void Storage::set_current_source_to_feed(Big_id id)
{
	this->current_source = SOURCE_FEED;
	this->current_source_id = id;
	this->reset();
}



void Storage::set_current_source_to_label(Big_id id)
{
	this->current_source = SOURCE_LABEL;
	this->current_source_id = id;
	this->reset();
}



void Storage::star(Big_id id, bool is)
{
	MLIB_D("Starring(%1) item [%2]...", is, id);

	try
	{
		this->exec(_F(
			"UPDATE "
				"items "
			"SET "
				"starred = %1 "
			"WHERE "
				"id = %2",
			is, id
		));
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Database error:"), EE(e) ));
	}

	this->current_query_star_cache[id] = is;
}


}}


