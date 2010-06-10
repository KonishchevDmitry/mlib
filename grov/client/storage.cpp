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
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
*   GNU General Public License for more details.                          *
*                                                                         *
**************************************************************************/


#include <boost/foreach.hpp>

#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <QtCore/QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <mlib/db/core.hpp>
#include <mlib/db/scoped_transaction.hpp>

#include <grov/common.hpp>
#include <grov/common/feed.hpp>
#include <grov/common/feed_item.hpp>
#include <grov/common/feed_tree.hpp>

#include <grov/main.hpp>

#include "web_cache.hpp"

#include "storage.hpp"


namespace grov { namespace client {


/// Version of database format that we use.
const Version CURRENT_DB_FORMAT_VERSION = MLIB_GET_VERSION(0, 2, 0);


Storage::Storage(QObject* parent)
:
	QObject(parent),

	db(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"))),

	root_label_id(0),
	broadcast_feed_id(-1),
	starred_feed_id(-1),
	shared_feed_id(-1),

	current_source(SOURCE_NONE),

	cache_flushing_timer(new QTimer(this))
{
#if !GROV_DEVELOP_MODE
	QString app_home_dir = get_app_home_dir();

	// Application's home directory
	if(!QDir("").mkpath(app_home_dir))
		M_THROW(tr("Can't create directory '%1'."), app_home_dir);
#endif

	// Opening the database -->
	{
	#if GROV_DEVELOP_MODE
		QString db_path = GROV_APP_UNIX_NAME ".db";
	#else
		QString db_path = QDir(app_home_dir).filePath(GROV_APP_UNIX_NAME ".db");
	#endif

		this->db->setDatabaseName(db_path);

		MLIB_D("Opening database '%1'...", db_path);

		if(!this->db->open())
			M_THROW(PAM( _F(tr("Unable to open database '%1':"), db_path), EE(*this->db) ));
	}
	// Opening the database <--

	// Locking the database -->
		try
		{
			this->exec("PRAGMA locking_mode = EXCLUSIVE");

			// We must make some changes to trigger the lock
			m::db::Scoped_transaction transaction(*this->db);
				this->exec("CREATE TABLE lock_table (id INTEGER)");
				this->exec("DROP TABLE lock_table");
			transaction.commit();
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( tr("Error while locking the database."), EE(e) ));
		}
	// Locking the database <--

	// Preparing the database -->
		if(this->db->tables().empty())
			this->create_db_tables();
		else
			this->check_db_format_version();

		this->get_db_info();
	// Preparing the database <--

	// Setting cache flushing timer -->
		this->cache_flushing_timer->setInterval(
			config::cache_flushing_interval * 1000 );

		connect(this->cache_flushing_timer, SIGNAL(timeout()),
			SLOT(cache_flushing_timed_out()) );

		this->cache_flushing_timer->start();
	// Setting cache flushing timer <--
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



void Storage::add_feeds(const QHash<QString, QString>& label_sort_ids, const Gr_feed_list& feeds, const QHash<QString, QString>& orderings)
{
	MLIB_D("Adding %1 feeds to DB...", feeds.size());

	// Just in case
	QHash<QString, const Gr_feed*> uniq_feeds;
	Q_FOREACH(const Gr_feed& feed, feeds)
		uniq_feeds[feed.gr_id] = &feed;

	QHash<QString, Big_id> labels;
	QHash<QString, Big_id> feed_sort_ids;

	try
	{
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

				// Sorting
				feed_sort_ids[feed->sort_id] = feed_id;

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

		// Adding ordering info -->
		{
			QSqlQuery insert_ordering_query = this->prepare(
				"INSERT INTO orderings (parent_id, child_id, order_id) "
					"values (:parent_id, :child_id, :order_id)"
			);

			bool root_target = true;
			const int sort_id_size = 8;
			QString target = "state/com.google/root";
			Big_id target_id = this->root_label_id;

			MLIB_CONST_ITER_TYPE(labels) label_it = labels.constBegin();

			while(true)
			{
				if(!root_target)
				{
					target = "label/" + label_it.key();
					target_id = label_it.value();
				}

				QString orderings_string = orderings[target];
				if(orderings_string.size() % sort_id_size)
					M_THROW(tr("Logical error."));

				for(int i = 0; i < orderings_string.size(); i += sort_id_size)
				{
					QString sort_id = orderings_string.mid(i, sort_id_size);

					MLIB_CONST_ITER_TYPE(feed_sort_ids) feed_id_it;
					MLIB_CONST_ITER_TYPE(label_sort_ids) label_name_it;
					MLIB_CONST_ITER_TYPE(labels) label_id_it;

					if(
						( feed_id_it = feed_sort_ids.find(sort_id) ) != feed_sort_ids.constEnd() ||
						(
							( label_name_it = label_sort_ids.find(sort_id) ) != label_sort_ids.constEnd() &&
							( label_id_it = labels.find(*label_name_it) ) != labels.constEnd()
						)
					)
					{
						insert_ordering_query.bindValue(":parent_id", target_id);

						if(feed_id_it != feed_sort_ids.constEnd())
							insert_ordering_query.bindValue(":child_id", *feed_id_it);
						else
						{
							// Negative sign is to distinguish labels from feeds.
							insert_ordering_query.bindValue(":child_id", -*label_id_it);
						}

						// +1 is because feeds and labels without order id (if
						// such will) has order_id = 0
						insert_ordering_query.bindValue(":order_id", i / sort_id_size + 1);

						this->exec(insert_ordering_query);
					}
				}

				if(root_target)
					root_target = false;
				else
					++label_it;

				if(label_it == labels.constEnd())
					break;
			}
		}
		// Adding ordering info <--

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
					"feed_id, broadcast, read, orig_read, starred, orig_starred, shared,"
					"gr_id, url, title, summary"
				") values ("
					":feed_id, :broadcast, :read, :orig_read, :starred, :orig_starred, :shared,"
					":gr_id, :url, :title, :summary"
				")"
			);

			Q_FOREACH(const Gr_feed_item& item, items)
			{
				Big_id feed_id = feeds.value(item.feed_gr_id, -1);

				if(feed_id < 0 && !item.broadcast && !item.starred)
				{
					// It may be due an error or when user setted an ignore
					// settings for this feed.
					MLIB_D("Gotten item '%1' for unknown feed '%2'. Skipping it.",
						item.gr_id, item.feed_gr_id );
					continue;
				}

				insert_item_query.bindValue(":feed_id", feed_id);
				insert_item_query.bindValue(":broadcast", int(item.broadcast));
				insert_item_query.bindValue(":read", 0);
				insert_item_query.bindValue(":orig_read", 0);
				insert_item_query.bindValue(":starred", int(item.starred));
				insert_item_query.bindValue(":orig_starred", int(item.starred));
				insert_item_query.bindValue(":shared", 0);
				insert_item_query.bindValue(":gr_id", item.gr_id);
				insert_item_query.bindValue(":url", item.url);
				insert_item_query.bindValue(":title", item.title);
				insert_item_query.bindValue(":summary", item.summary);
				this->exec(insert_item_query);
			}
		}
		// Adding items <--

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
	if(entry.data.isEmpty() && entry.location.isEmpty())
	{
		MLIB_D("Skipping adding the empty data with an empty Location header to the DB.");
		return;
	}

	MLIB_D("Adding web cache entry for '%1' to the DB...", entry.url);

	try
	{
		QSqlQuery query = this->prepare(
			"INSERT OR IGNORE INTO web_cache ("
				"url, location, content_type, content_encoding, data"
			") values ("
				":url, :location, :content_type, :content_encoding, :data"
			")"
		);

		query.bindValue(":url", entry.url);
		query.bindValue(":location", entry.location);
		query.bindValue(":content_type", entry.content_type);
		query.bindValue(":content_encoding", entry.content_encoding);
		query.bindValue(":data", entry.data);
		this->exec(query);
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( _F(tr("Error while adding '%1' to the Web cache:"), entry.url), EE(e) ));
	}

	MLIB_D("Web cache entry has been added successfully.");
}



void Storage::cache_flushing_timed_out(void)
{
	MLIB_D("Flushing the cache by timeout.");

	try
	{
		this->flush_cache();
	}
	catch(m::Exception& e)
	{
		MLIB_SW(EE(e));
	}
}



void Storage::cancel_editing(void)
{
	// We must destroy all queries started inside the transaction before
	// rolling back it.
	this->set_current_source_to_none();

	try
	{
		this->exec("ROLLBACK");
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to rollback a transaction:"), EE(e) ));
	}
}



void Storage::check_db_format_version(void)
{
	Version version = 0;

	while(version != CURRENT_DB_FORMAT_VERSION)
	{
		try
		{
			QSqlQuery query = this->exec_and_next("SELECT value FROM config WHERE name = 'version'");

			// An infinite loop
			if(version && version == query.value(0))
				M_THROW(tr("Logical error."));

			version = m::qvariant_to_version(query.value(0));
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( tr("Unable to get the database format version:"), EE(e) ));
		}

		if(!version)
			M_THROW(tr("Application's database '%1' has an invalid format version."),
				this->db->databaseName() );

		try
		{
			switch(version)
			{
				case CURRENT_DB_FORMAT_VERSION:
					// All is OK.
					return;
					break;

				// Upgrading the DB -->
					case MLIB_GET_VERSION(0, 1, 0):
					{
						m::db::Scoped_transaction transaction(*this->db);
							this->exec("INSERT INTO feeds (name) VALUES ('shared')");

							this->exec(
								"CREATE TABLE orderings("
									"parent_id INTEGER,"
									"child_id INTEGER,"
									"order_id INTEGER"
								")"
							);
							this->exec("CREATE INDEX orderings_idx ON orderings(parent_id, child_id)");

							this->exec("ALTER TABLE items ADD COLUMN shared INTEGER");
							this->exec("UPDATE items SET shared = 0");

							this->exec(_F("UPDATE config SET value = %1 WHERE name = 'version'", MLIB_GET_VERSION(0, 2, 0)));
						transaction.commit();
					}
					break;
				// Upgrading the DB <--

				default:
					M_THROW(tr(
						"Application's database '%1' has unsupported format version. "
						"Please delete it by yourself."
						"\n\n"
						"If you have an important unsaved offline data in it, "
						"please flush this offline data by %2."),
						this->db->databaseName(), GROV_APP_NAME " " + m::get_version_string(version)
					);
					break;
			}
		}
		catch(m::Exception& e)
		{
			M_THROW(PAM( tr("Error while upgrading the database to a new version."), EE(e) ));
		}
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
			this->exec("DELETE FROM orderings");
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

// Takes too many time. May be in the future.
#if 0
	try
	{
		MLIB_D("Vacuuming the database...");
		this->exec("VACUUM");
	}
	catch(m::Exception& e)
	{
		MLIB_SW(PAM( tr("Unable to vacuum the database:"), EE(e) ));
	}
#endif

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
				where = "broadcast = 1 AND read = 0";
			else if(this->current_source_id == this->starred_feed_id)
				where = "starred = 1";
			else if(this->current_source_id == this->shared_feed_id)
				where = "shared = 1";
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
			"id, feed_id, url, title, summary, broadcast, read, starred, shared "
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
		this->exec("CREATE INDEX lookup_config_by_name_idx ON config(name)");
		this->exec(_F("INSERT INTO config VALUES ('version', %1)", CURRENT_DB_FORMAT_VERSION));
		this->exec(_F("INSERT INTO config VALUES ('mode', '')"));

		this->exec(
			"CREATE TABLE feeds("
				"id INTEGER PRIMARY KEY,"
				"name TEXT,"
				"gr_id TEXT" // Google Reader's id.
			")"
		);

		this->exec(
			"CREATE TABLE labels("
				"id INTEGER PRIMARY KEY,"
				"name TEXT"
			")"
		);

		// Label -> Feeds mapping.
		this->exec(
			"CREATE TABLE labels_to_feeds("
				"label_id INTEGER,"
				"feed_id INTEGER"
			")"
		);
		this->exec("CREATE INDEX lookup_feeds_by_label_id_idx ON labels_to_feeds(label_id)");

		// Label and feed ordering in the Subscriptions tree.
		this->exec(
			"CREATE TABLE orderings("
				"parent_id INTEGER,"
				"child_id INTEGER,"
				"order_id INTEGER"
			")"
		);
		this->exec("CREATE INDEX orderings_idx ON orderings(parent_id, child_id)");

		this->exec(
			"CREATE TABLE items("
				"id INTEGER PRIMARY KEY,"
				"feed_id INTEGER,"
				"broadcast INTEGER,"
				"read INTEGER,"
				"orig_read INTEGER,"
				"starred INTEGER,"
				"orig_starred INTEGER,"
				"shared INTEGER,"
				"gr_id TEXT," // Google Reader's id.
				"url TEXT,"
				"title TEXT,"
				"summary TEXT"
			")"
		);
		this->exec("CREATE INDEX lookup_feeds_unread_items_idx ON items(feed_id, read, id)");

		this->exec(
			"CREATE TABLE web_cache("
				"url TEXT UNIQUE,"
				"location TEXT,"
				"content_type TEXT,"
				"content_encoding TEXT,"
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



void Storage::end_editing(void)
{
	// We must destroy all queries started inside the transaction before
	// committing it.
	this->set_current_source_to_none();

	try
	{
		this->exec("COMMIT");
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to commit a transaction:"), EE(e) ));
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
	Big_id shared_feed_id;

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

		query.bindValue(":name", "shared");
		this->exec_and_next(query);
		shared_feed_id = m::qvariant_to_big_id(query.value(0));
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Error while getting database's main information:"), EE(e) ));
	}

	this->broadcast_feed_id = broadcast_feed_id;
	this->starred_feed_id = starred_feed_id;
	this->shared_feed_id = shared_feed_id;
}



Feed_tree Storage::get_feed_tree(void)
{
	MLIB_D("Getting feed tree...");

	// Throws m::Exception
	this->flush_cache();

	try
	{
		Feed_tree feed_tree = Feed_tree::create();

		QSqlQuery root_query = this->exec(_F(
			"SELECT "
				"id, name, 1 AS is_label, 1 AS prio_order, -orderings.order_id AS neg_order_id "
			"FROM "
				"labels "
			"LEFT JOIN "
				"orderings "
			"ON "
				// Negative sign is to distinguish labels from feeds.
				"orderings.parent_id = %1 AND labels.id = -orderings.child_id "

			"UNION "

			"SELECT "
				"id, name, 0 AS is_label, (id != %2) AS prio_order, -orderings.order_id AS neg_order_id "
			"FROM "
				"feeds "
			"LEFT JOIN "
				"orderings "
			"ON "
				"orderings.parent_id = %1 AND feeds.id = orderings.child_id "
			"WHERE "
				"feeds.id NOT IN ( "
					"SELECT "
						"feed_id "
					"FROM "
						"labels_to_feeds "
					"GROUP BY "
						"feed_id "
				") "

			"ORDER BY "
				// Negative sign and DESC is to place labels and feeds without
				// ordering to the end.
				"prio_order, neg_order_id DESC",

			this->root_label_id, this->broadcast_feed_id
		));

		QSqlQuery label_feeds_query = this->prepare(
			"SELECT "
				"id, name "
			"FROM "
				"feeds "
			"LEFT JOIN "
				"orderings "
			"ON "
				"orderings.parent_id = :sort_id AND feeds.id = orderings.child_id "
			"WHERE "
				"feeds.id IN ( "
					"SELECT "
						"id "
					"FROM "
						"feeds, labels_to_feeds "
					"WHERE "
						"label_id = :label_id AND feeds.id = feed_id "
				") "
			"ORDER BY "
				// Negative sign and DESC is to place feeds without ordering to
				// the end.
				"-orderings.order_id DESC"
		);

		QSqlQuery feeds_items_query = this->prepare(
			"SELECT "
				"COUNT(*) "
			"FROM "
				"items "
			"WHERE "
				"feed_id = :feed_id AND read = 0"
		);

		while(root_query.next())
		{
			// This is a label
			if(root_query.value(2).toInt())
			{
				Big_id label_id = m::qvariant_to_big_id(root_query.value(0));
				QString label_name = root_query.value(1).toString();

				Feed_tree_item* label = feed_tree.add_label(label_id, label_name);
				label_feeds_query.bindValue(":label_id", label_id);
				label_feeds_query.bindValue(":sort_id", label_id);
				this->exec(label_feeds_query);

				while(label_feeds_query.next())
				{
					Big_id feed_id = m::qvariant_to_big_id(label_feeds_query.value(0));
					QString feed_name = label_feeds_query.value(1).toString();

					feeds_items_query.bindValue(":feed_id", feed_id);
					this->exec_and_next(feeds_items_query);

					Feed_tree_item* feed = label->add_feed(feed_id, feed_name);
					feed->unread_items = m::qvariant_to_big_id(feeds_items_query.value(0));
					label->unread_items += feed->unread_items;
				}
			}
			// This is a feed
			else
			{
				Big_id feed_id = m::qvariant_to_big_id(root_query.value(0));
				QString feed_name = root_query.value(1).toString();

				feeds_items_query.bindValue(":feed_id", feed_id);
				this->exec_and_next(feeds_items_query);

				Feed_tree_item* feed = feed_tree.add_feed(feed_id, feed_name);
				feed->unread_items = m::qvariant_to_big_id(feeds_items_query.value(0));
			}
		}

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
				query->value(4).toString(),
				query->value(5).toBool(),
				this->current_query_read_cache.contains(id) ? true : query->value(6).toBool(),
				this->current_query_star_cache.value(id, query->value(7).toBool()),
				this->current_query_share_cache.value(id, query->value(8).toBool())
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

		query = this->exec("SELECT id, gr_id FROM items WHERE shared = 1");
		while(query.next())
		{
			items << Changed_feed_item(
				m::qvariant_to_big_id(query.value(0)),
				query.value(1).toString(),
				Changed_feed_item::PROPERTY_SHARED,
				true
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
				"location, "
				"content_type, "
				"content_encoding, "
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
				query.value(1).toString(),
				query.value(2).toString(),
				query.value(3).toByteArray()
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
	QSqlQuery query = this->prepare("INSERT INTO feeds (name) VALUES (:name)");

	// This is special feeds that must always exists.
	const char* const feeds[] = {
		"broadcast",
		"starred",
		"shared"
	};

	BOOST_FOREACH(const char* feed, feeds)
	{
		query.bindValue(":name", feed);
		this->exec(query);
	}
}



bool Storage::is_in_web_cache(const QString& url)
{
	try
	{
		QSqlQuery query = this->prepare(
			"SELECT "
				"COUNT(*) "
			"FROM "
				"web_cache "
			"WHERE "
				"url = :url"
		);
		query.bindValue(":url", url);
		this->exec_and_next(query);

		return query.value(0).toBool();
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( _F(tr("Error while getting web cache for '%1':"), url), EE(e) ));
	}
}



void Storage::mark_as_read(const Db_feed_item& item)
{
	MLIB_D("Marking item [%1] as read...", item.id);

	// Starred items showed to user even if they is already read, so we must
	// always check current item's status.
	if(this->current_query_read_cache.contains(item.id) || item.read)
		MLIB_D("Item [%1] is already marked as read.", item.id);
	else
	{
		this->current_query_read_cache.insert(item.id);
		this->readed_items_cache << item.id;

		// Updating items counter -->
		{
			QList<Big_id> feed_ids;

			if(item.feed_id >= 0)
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
			QSqlQuery* query = NULL;

			switch(item.property)
			{
				case Changed_feed_item::PROPERTY_READ:
					query = &read_query;
					break;

				case Changed_feed_item::PROPERTY_STARRED:
					query = &star_query;
					break;

				case Changed_feed_item::PROPERTY_SHARED:
					break;

				default:
					MLIB_LE();
					break;
			}

			if(query)
			{
				query->bindValue(":id", item.id);
				query->bindValue(":value", item.value);
				this->exec(*query);
			}

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
	this->current_query_share_cache.clear();
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


void Storage::share(Big_id id, bool is)
{
	MLIB_D("Sharing(%1) item [%2]...", is, id);

	try
	{
		this->exec(_F(
			"UPDATE "
				"items "
			"SET "
				"shared = %1 "
			"WHERE "
				"id = %2",
			is, id
		));
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Database error:"), EE(e) ));
	}

	this->current_query_share_cache[id] = is;
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



void Storage::start_editing(void)
{
	try
	{
		// This is really speeds up many insertions for SQLite
		this->exec("BEGIN");
	}
	catch(m::Exception& e)
	{
		M_THROW(PAM( tr("Unable to begin a transaction:"), EE(e) ));
	}
}


}}


