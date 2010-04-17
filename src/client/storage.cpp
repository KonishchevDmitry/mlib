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


#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <mlib/db/core.hpp>
#include <mlib/db/scoped_transaction.hpp>

#include <src/common.hpp>
#include <src/common/feed.hpp>
#include <src/common/feed_item.hpp>
#include <src/common/feed_tree.hpp>

#include <src/main.hpp>

#include "web_cache.hpp"

#include "storage.hpp"


namespace grov { namespace client {


// TODO
const int CURRENT_DB_FORMAT_VERSION = 0;


Storage::Storage(QObject* parent)
:
	QObject(parent),

	db(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"))),

	broadcast_feed_id(-1),
	starred_feed_id(-1),

	current_source(SOURCE_NONE)
{
	QString app_home_dir = get_app_home_dir();

	// Application's home directory
	if(!QDir("").mkpath(app_home_dir))
		M_THROW(tr("Can't create directory '%1."), app_home_dir);

	// Opening the database -->
	{
		QString db_path = QDir(app_home_dir).filePath(GROV_APP_UNIX_NAME ".db");

		this->db->setDatabaseName(db_path);

		MLIB_D("Opening database '%1'...", db_path);

		if(!this->db->open())
			M_THROW(PAM( _F(tr("Unable to open database '%1':"), db_path), EE(*this->db) ));
	}
	// Opening the database <--

	// Preparing the database -->
		if(this->db->tables().empty())
			this->create_db_tables();
		else
			this->check_db_format_version();

		this->get_db_info();
	// Preparing the database <--
}



Storage::~Storage(void)
{
	try
	{
		this->flush_cache();
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Error while closing database"), EE(e));
	}

	this->reset();
}



void Storage::add_feeds(const Gr_feed_list& feeds)
{
	MLIB_D("Adding %1 feeds to DB...", feeds.size());

	// Just in case
	QHash<QString, const Gr_feed*> uniq_feeds;
	Q_FOREACH(const Gr_feed& feed, feeds)
		uniq_feeds[feed.gr_id] = &feed;

	QHash<QString, Big_id> labels;

	try
	{
		// For SQLite it really speeds up many insertions.
		m::db::Scoped_transaction transaction(*this->db);

		// Adding feeds -->
		{
			QSqlQuery insert_feed_query = this->prepare(
				"INSERT INTO feeds (gr_id, name) values (:gr_id, :name)" );

			QSqlQuery insert_label_query = this->prepare(
				"INSERT INTO labels (name) values (:name)" );

			QSqlQuery insert_labels_to_feeds_query = this->prepare(
				"INSERT INTO labels_to_feeds (label_id, feed_id) values (:label_id, :feed_id)" );

			Q_FOREACH(const Gr_feed* feed, uniq_feeds)
			{
				Big_id feed_id;
				QSet<Big_id> feed_labels;

				// Feed -->
					insert_feed_query.bindValue(":gr_id", feed->gr_id);
					insert_feed_query.bindValue(":name", feed->name);
					this->exec(insert_feed_query);
					feed_id = m::qvariant_to_big_id(insert_feed_query.lastInsertId());
				// Feed <--

				// Labels -->
					Q_FOREACH(const QString& label, feed->labels)
					{
						Big_id label_id = labels.value(label, -1);

						// Adding label to DB -->
							if(label_id < 0)
							{
								insert_label_query.bindValue(":name", label);
								this->exec(insert_label_query);
								label_id = m::qvariant_to_big_id(insert_label_query.lastInsertId());

								labels[label] = label_id;
							}
						// Adding label to DB <--

						// Labels to feeds -->
							if(!feed_labels.contains(label_id))
							{
								insert_labels_to_feeds_query.bindValue(":label_id", label_id);
								insert_labels_to_feeds_query.bindValue(":feed_id", feed_id);
								this->exec(insert_labels_to_feeds_query);
								feed_labels.insert(label_id);
							}
						// Labels to feeds <--
					}
				// Labels <--
			}
		}
		// Adding feeds <--

		transaction.commit();

		// TODO: odd emits
		emit feed_tree_changed();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to add feeds to the database:"), EE(e) ));
	}

	MLIB_D("Feeds has been successfully added to DB.");
}



void Storage::add_items(const Gr_feed_item_list& items)
{
	MLIB_D("Adding %1 items to DB...", items.size());

	try
	{
		QHash<QString, Big_id> feeds;

		// For SQLite it really speeds up many insertions.
		m::db::Scoped_transaction transaction(*this->db);

		// Getting all known feeds -->
		{
			QSqlQuery query = this->exec(
				"SELECT id, gr_id FROM feeds" );

			while(query.next())
				feeds[query.value(1).toString()] = m::qvariant_to_big_id(query.value(0));
		}
		// Getting all known feeds <--

		// Adding items -->
		{
			QSqlQuery insert_item_query = this->prepare(
				"INSERT INTO items ("
					"feed_id, broadcast, read, orig_read, starred, orig_starred,"
					"gr_id, title, summary"
				") values ("
					":feed_id, :broadcast, :read, :orig_read, :starred, :orig_starred,"
					":gr_id, :title, :summary"
				")"
			);

			Q_FOREACH(const Gr_feed_item& item, items)
			{
				Big_id feed_id = feeds.value(item.feed_gr_id, -1);

				if(feed_id < 0)
				{
					if(item.broadcast)
						// TODO:
						feed_id = -1;
					else
					{
						// It may be due an error or when user mark feed's
						// label by ignore mark.
						MLIB_D("Gotten item '%1' for unknown feed '%2'. Skipping it.",
							item.gr_id, item.feed_gr_id );
						continue;
					}
				}

				insert_item_query.bindValue(":feed_id", feed_id);
				insert_item_query.bindValue(":broadcast", int(item.broadcast));
				insert_item_query.bindValue(":read", 0);
				insert_item_query.bindValue(":orig_read", 0);
				insert_item_query.bindValue(":starred", int(item.starred));
				insert_item_query.bindValue(":orig_starred", int(item.starred));
				insert_item_query.bindValue(":gr_id", item.gr_id);
				insert_item_query.bindValue(":title", item.title);
				insert_item_query.bindValue(":summary", item.summary);
				this->exec(insert_item_query);
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

	MLIB_D("All items successfully added to DB.");
}



void Storage::add_web_cache_entry(const Web_cache_entry& entry)
{
	MLIB_D("Adding web cache entry for '%1' to the DB...", entry.url);

	try
	{
		QSqlQuery query = this->prepare(
			"INSERT OR REPLACE INTO web_cache ("
				"url, content_type, data"
			") values ("
				":url, :content_type, :data"
			")"
		);

		query.bindValue(":url", entry.url);
		query.bindValue(":content_type", entry.content_type);
		// TODO: data written in db in some ugly form. fix this
		query.bindValue(":data", entry.data);
		this->exec(query);
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( _F(tr("Error while adding '%1' to the Web cache:"), entry.url), EE(e) ));
	}

	MLIB_D("Web cache entry has been added successfully.");
}



void Storage::check_db_format_version(void)
{
	int version;

	try
	{
		QSqlQuery query = this->exec_and_next("SELECT value FROM config WHERE name = 'version'");
		version = query.value(0).toInt();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to get the database format version:"), EE(e) ));
	}

	if(version != CURRENT_DB_FORMAT_VERSION)
	{
		M_THROW(tr(
			"Application's database '%1' has unsupported format version. "
			"Please delete it by yourself. "
			"If you have an important unsaved offline data in it, "
			"please flush this offline data by that version of %2, "
			"which you have used to create it."),
			this->db->databaseName(), GROV_APP_NAME
		);
	}
}



void Storage::clear(void)
{
	MLIB_D("Cleaning offline data...");

	try
	{
		m::db::Scoped_transaction transaction(*this->db);
			this->exec("DELETE FROM feeds");
			this->exec("DELETE FROM labels");
			this->exec("DELETE FROM items");
			this->exec("DELETE FROM labels_to_feeds");
			this->exec("DELETE FROM web_cache");
			this->init_empty_database();
			this->get_db_info();
		transaction.commit();

		this->reset();
		this->clear_cache();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Error while deleting data from the database."), EE(e) ));
	}

	try
	{
		MLIB_D("Vacuuming the database...");
		this->exec("VACUUM");
	}
	catch(m::Exception& e)
	{
		MLIB_SW(PAM( tr("Unable to vacuum the database:"), EE(e) ));
	}

	MLIB_D("Offline data cleaned.");

	emit this->feed_tree_changed();
}



void Storage::clear_cache(void)
{
	this->readed_items_cache.clear();
}



void Storage::create_current_query(void)
{
	MLIB_D("Creating new current query for %1:%2...",
		this->current_source, this->current_source_id );

	// Throws m::Exception
	this->flush_cache();

	QString where;
	bool bind_source_id = false;

	switch(this->current_source)
	{
		case SOURCE_ALL:
			break;

		case SOURCE_FEED:
		{
			if(this->current_source_id == this->broadcast_feed_id)
				where = "read = 0 AND broadcast = 1";
			else if(this->current_source_id == this->starred_feed_id)
				where = "starred = 1";
			else
			{
				where = "feed_id = :source_id AND read = 0";
				bind_source_id = true;
			}
		}
		break;

		case SOURCE_LABEL:
		{
			where =
				"feed_id IN ( "
					"SELECT "
						"feed_id "
					"FROM "
						"labels_to_feeds "
					"WHERE label_id = :source_id "
				") AND read = 0";
			bind_source_id = true;
		}
		break;

		default:
			M_THROW(tr("Logical error (invalid item's source type)."));
			break;
	}

	// Throws m::Exception
	QSqlQuery query = this->prepare(_F(
		"SELECT "
			"id, feed_id, title, summary, broadcast, read, starred "
		"FROM "
			"items "
		"%1 "
		"ORDER BY "
			"id", where.isEmpty() ? QString() : "WHERE " + where
	));

	if(bind_source_id)
		query.bindValue(":source_id", this->current_source_id);

	// Throws m::Exception
	this->exec(query);

	this->current_query.reset(new QSqlQuery(query));
}



// TODO: selects and indexes optimization
void Storage::create_db_tables(void)
{
	try
	{
		m::db::Scoped_transaction transaction(*this->db);

		this->exec(
			"CREATE TABLE config("
				"name TEXT,"
				"value TEXT"
			")"
		);
		this->exec(_F("INSERT INTO config VALUES ('version', %1)", CURRENT_DB_FORMAT_VERSION));
		this->exec(_F("INSERT INTO config VALUES ('mode', '')"));

		this->exec(
			"CREATE TABLE feeds("
				"id INTEGER PRIMARY KEY,"
				"name TEXT,"
				"gr_id TEXT"
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
				"feed_id INTEGER,"
				"broadcast INTEGER,"
				"read INTEGER,"
				"orig_read INTEGER,"
				"starred INTEGER,"
				"orig_starred INTEGER,"
				"gr_id TEXT,"
				"title TEXT,"
				"summary TEXT"
			")"
		);
		this->exec("CREATE INDEX items_feed_id_read_idx ON items(feed_id, read)");

		this->exec(
			"CREATE TABLE labels_to_feeds("
				"label_id INTEGER,"
				"feed_id INTEGER"
			")"
		);
		this->exec("CREATE INDEX labels_to_feeds_label_id_idx ON labels_to_feeds(label_id)");

		this->exec(
			"CREATE TABLE web_cache("
				"url TEXT UNIQUE,"
				"content_type TEXT,"
				"data BLOB"
			")"
		);

		this->init_empty_database();

		transaction.commit();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Error while creating tables in the database:"), EE(e) ));
	}
}



void Storage::exec(QSqlQuery& query)
{
	MLIB_DV("Executing query '%1'...", query.lastQuery());

	if(!query.exec())
		M_THROW(EE(query));
}



QSqlQuery Storage::exec(const QString& query_string)
{
	QSqlQuery query = this->prepare(query_string);
	this->exec(query);
	return query;
}



void Storage::exec_and_next(QSqlQuery& query)
{
	this->exec(query);

	if(!query.next())
		M_THROW(tr("query did not return a value"));
}



QSqlQuery Storage::exec_and_next(const QString& query_string)
{
	QSqlQuery query = this->prepare(query_string);
	this->exec_and_next(query);
	return query;
}



/// TODO: by timer
void Storage::flush_cache(void)
{
	if(this->readed_items_cache.empty())
		return;

	QString ids;
	Q_FOREACH(Big_id id, this->readed_items_cache)
		ids += (ids.isEmpty() ? "" : ",") + QString::number(id);

	try
	{
		this->exec(
			"UPDATE "
				"items "
			"SET "
				"read = 1 "
			"WHERE "
				"id IN (" + ids + ")"
		);
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Error while flushing changes' cache to the database:"), EE(e) ));
	}

	this->readed_items_cache.clear();
}



void Storage::get_db_info(void)
{
	Big_id broadcast_feed_id;
	Big_id starred_feed_id;

	try
	{
		QSqlQuery query = this->prepare(
			"SELECT "
				"id "
			"FROM "
				"feeds "
			"WHERE "
				"name = :name "
			"ORDER BY "
				"id "
			"LIMIT 1"
		);

		query.bindValue(":name", "broadcast");
		this->exec_and_next(query);
		broadcast_feed_id = m::qvariant_to_big_id(query.value(0));

		query.bindValue(":name", "starred");
		this->exec_and_next(query);
		starred_feed_id = m::qvariant_to_big_id(query.value(0));
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Error while getting database's main information:"), EE(e) ));
	}

	this->broadcast_feed_id = broadcast_feed_id;
	this->starred_feed_id = starred_feed_id;
}



Feed_tree Storage::get_feed_tree(void)
{
	MLIB_D("Getting feed tree...");

	// Throws m::Exception
	this->flush_cache();

	try
	{
		Feed_tree feed_tree = Feed_tree::create();

		QSqlQuery lonely_feeds_query = this->exec(
			"SELECT "
				"id, name "
			"FROM "
				"feeds "
			"WHERE "
				"id NOT IN ("
					"SELECT "
						"feed_id "
					"FROM "
						"labels_to_feeds "
					"GROUP BY "
						"feed_id"
				")"
		);

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

		QSqlQuery feeds_items_query = this->prepare(
			"SELECT "
				"COUNT(*) "
			"FROM "
				"items "
			"WHERE "
				"feed_id = :feed_id AND read = 0"
		);

		QSqlQuery broadcast_items_query = this->prepare(
			"SELECT "
				"COUNT(*) "
			"FROM "
				"items "
			"WHERE "
				"broadcast = 1"
		);

		// Labeled feeds -->
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

					feeds_items_query.bindValue(":feed_id", feed_id);
					this->exec_and_next(feeds_items_query);

					Feed_tree_item* feed = label->add_feed(feed_id, feed_name);
					feed->unread_items = m::qvariant_to_big_id(feeds_items_query.value(0));
					label->unread_items += feed->unread_items;
				}
			}
		// Labeled feeds <--

		// Lonely feeds -->
			while(lonely_feeds_query.next())
			{
				QSqlQuery query;
				Big_id feed_id = m::qvariant_to_big_id(lonely_feeds_query.value(0));
				QString feed_name = lonely_feeds_query.value(1).toString();

				if(feed_id == this->broadcast_feed_id)
					query = broadcast_items_query;
				else
				{
					query = feeds_items_query;
					feeds_items_query.bindValue(":feed_id", feed_id);
				}

				this->exec_and_next(query);

				Feed_tree_item* feed = feed_tree.add_feed(feed_id, feed_name);
				feed->unread_items = m::qvariant_to_big_id(query.value(0));
			}
		// Lonely feeds <--

		MLIB_D("Feed tree gotten.");

		return feed_tree;
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Error while getting feed tree from the database:"), EE(e) ));
	}
}



Db_feed_item Storage::get_item(bool next)
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

			Db_feed_item item(
				id, m::qvariant_to_big_id(query->value(1)),
				query->value(2).toString(),
				query->value(3).toString(),
				query->value(4).toBool(),
				this->current_query_read_cache.contains(id) ? true : query->value(5).toBool(),
				this->current_query_star_cache.value(id, query->value(6).toBool())
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
		M_THROW(PAM( tr("Unable to query a feed's item from the database."), EE(e) ));
	}
}



QString Storage::get_mode(void)
{
	QSqlQuery query = this->exec_and_next(
		"SELECT "
			"value "
		"FROM "
			"config "
		"WHERE "
			"name = 'mode'"
	);

	return query.value(0).toString();
}



Db_feed_item Storage::get_next_item(void)
{
	// Throws m::Exception
	return this->get_item(true);
}



Db_feed_item Storage::get_previous_item(void)
{
	// Throws m::Exception
	return this->get_item(false);
}



Changed_feed_item_list Storage::get_user_changes(void)
{
	MLIB_D("Getting user changes...");

	Changed_feed_item_list items;

	try
	{
		QSqlQuery query;

		query = this->exec("SELECT id, gr_id, read FROM items WHERE orig_read != read");
		while(query.next())
		{
			items << Changed_feed_item(
				m::qvariant_to_big_id(query.value(0)),
				query.value(1).toString(),
				Changed_feed_item::PROPERTY_READ,
				query.value(2).toBool()
			);
		}

		query = this->exec("SELECT id, gr_id, starred FROM items WHERE orig_starred != starred");
		while(query.next())
		{
			items << Changed_feed_item(
				m::qvariant_to_big_id(query.value(0)),
				query.value(1).toString(),
				Changed_feed_item::PROPERTY_STARRED,
				query.value(2).toBool()
			);
		}
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to get user changes that he made to the items:"), EE(e) ));
	}

	MLIB_D("User changes gotten.");

	return items;
}



Web_cache_entry Storage::get_web_cache_entry(const QString& url)
{
	MLIB_D("Getting web cache for '%1'...", url);

	try
	{
		QSqlQuery query = this->prepare(
			"SELECT "
				"content_type, "
				"data "
			"FROM "
				"web_cache "
			"WHERE "
				"url = :url"
		);
		query.bindValue(":url", url);
		this->exec(query);

		if(query.next())
		{
			return Web_cache_entry(url,
				query.value(0).toString(),
				query.value(1).toByteArray()
			);
		}
		else
			return Web_cache_entry();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( _F(tr("Error while getting web cache for '%1':"), url), EE(e) ));
	}
}



void Storage::init_empty_database(void)
{
	// "broadcast" and "starred" is a special feeds that must always exists.

	QSqlQuery query = this->prepare("INSERT INTO feeds (name) VALUES (:name)");

	query.bindValue(":name", "broadcast");
	this->exec(query);

	query.bindValue(":name", "starred");
	this->exec(query);
}



void Storage::mark_as_read(const Db_feed_item& item)
{
	MLIB_D("Marking item [%1] as read...", item.id);

	if(this->current_query_read_cache.contains(item.id))
		MLIB_D("Item [%1] is already marked as read.", item.id);
	else
	{
		this->current_query_read_cache.insert(item.id);
		this->readed_items_cache << item.id;

		// Updating items counter -->
		{
			QList<Big_id> feed_ids;
			feed_ids << item.feed_id;

			if(item.broadcast)
				feed_ids << this->broadcast_feed_id;

			emit this->item_marked_as_read(feed_ids, true);
		}
		// Updating items counter <--

		if(readed_items_cache.size() > 10)
			// Throws m::Exception
			this->flush_cache();

		MLIB_D("Item [%1] marked as read.", item.id);
	}
}



void Storage::mark_changes_as_flushed(Changed_feed_item_list::const_iterator begin, Changed_feed_item_list::const_iterator end)
{
	MLIB_D("Marking %1 user changes as flushed...", end - begin);

	try
	{
		// For SQLite it really speeds up many updates.
		m::db::Scoped_transaction transaction(*this->db);

		QSqlQuery read_query = this->prepare(
			"UPDATE "
				"items "
			"SET "
				"orig_read = :value "
			"WHERE "
				"id = :id"
		);

		QSqlQuery star_query = this->prepare(
			"UPDATE "
				"items "
			"SET "
				"orig_starred = :value "
			"WHERE "
				"id = :id"
		);

		while(begin != end)
		{
			const Changed_feed_item& item = *begin;
			QSqlQuery* query;

			switch(item.property)
			{
				case Changed_feed_item::PROPERTY_READ:
					query = &read_query;
					break;

				case Changed_feed_item::PROPERTY_STARRED:
					query = &star_query;
					break;

				default:
					MLIB_LE();
					break;
			}

			query->bindValue(":id", item.id);
			query->bindValue(":value", item.value);
			this->exec(*query);

			++begin;
		}

		transaction.commit();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Error while synchronizing flushed to Google Reader changes with the database:"), EE(e) ));
	}

	MLIB_D("Changes had been successfully marked as flushed.");
}



QSqlQuery Storage::prepare(const QString& string)
{
	QSqlQuery query(*this->db);

	MLIB_DV("Preparing SQL query '%1'...", string);
	if(!query.prepare(string))
		M_THROW(EE(query));

	return query;
}



void Storage::prepare_to_flush_offline_data(void)
{
	this->flush_cache();
	this->reset();
}



void Storage::reset(void)
{
	MLIB_D("Reseting...");
	this->current_query.reset();
	this->current_query_read_cache.clear();
	this->current_query_star_cache.clear();
}



void Storage::set_current_source(Current_source source, Big_id id)
{
	this->current_source = source;
	this->current_source_id = id;
	this->reset();
}



void Storage::set_current_source_to_all(void)
{
	this->set_current_source(SOURCE_ALL, -1);
}



void Storage::set_current_source_to_feed(Big_id id)
{
	this->set_current_source(SOURCE_FEED, id);
}



void Storage::set_current_source_to_label(Big_id id)
{
	this->set_current_source(SOURCE_LABEL, id);
}



void Storage::set_current_source_to_none(void)
{
	this->set_current_source(SOURCE_NONE, -1);
}



void Storage::set_mode(const QString& mode)
{
	this->exec(_F(
		"UPDATE "
			"config "
		"SET "
			"value = '%1' "
		"WHERE "
			"name = 'mode'", mode
	));
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


