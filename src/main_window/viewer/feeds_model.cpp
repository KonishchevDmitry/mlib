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

// TODO

#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>

#include <src/common.hpp>
#include <src/common/feed_tree.hpp>

#include <src/client/storage.hpp>

#include "feeds_model.hpp"


namespace grov { namespace main_window { namespace viewer {


Feeds_model::Feeds_model(client::Storage* storage, QObject *parent)
:
	QAbstractItemModel(parent),
	storage(storage),
	feed_tree(Feed_tree::create())
{
	connect(this->storage, SIGNAL(feed_tree_changed()),
		this, SLOT(feed_tree_changed()) );

	connect(this->storage, SIGNAL(item_marked_as_read(const QList<Big_id>&, Big_id, bool)),
		this, SLOT(item_marked_as_read(const QList<Big_id>&, Big_id, bool)) );

	this->feed_tree_changed();
}



int Feeds_model::columnCount(const QModelIndex& parent) const
{
	return 1;
}



QVariant Feeds_model::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
		return QVariant();

	const Feed_tree_item* item = this->get(index);

	switch(role)
	{
		case Qt::DisplayRole:
			return item->get_name() + " (" + QString::number(item->unread_items) + ')';
			break;

		case ROLE_IS_FEED:
			return item->is_feed();
			break;

		case ROLE_ID:
			return item->get_id();
			break;

		default:
			return QVariant();
	}
}



void Feeds_model::feed_tree_changed(void)
{
	MLIB_D("Feed tree changed. Updating it...");

	if(this->feed_tree.count())
	{
		this->beginRemoveRows(QModelIndex(), 0, this->feed_tree.count() - 1);
		this->endRemoveRows();
	}

	try
	{
		this->feed_tree = this->storage->get_feed_tree();
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Error while updating subscription list"), EE(e) );
	}

	this->update_feed_tree_map();

	if(this->feed_tree.count())
	{
		this->beginInsertRows(QModelIndex(), 0, this->feed_tree.count() - 1);
		this->endInsertRows();
	}

	MLIB_D("Feed tree updated.");
}



Qt::ItemFlags Feeds_model::flags(const QModelIndex& index) const
{
	if(!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}



const Feed_tree_item* Feeds_model::get(const QModelIndex& index) const
{
	return reinterpret_cast<const Feed_tree_item*>(index.internalPointer());
}



QVariant Feeds_model::headerData(int section, Qt::Orientation orientation, int role) const
{
// TODO
// if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
//	 return rootItem->data(section);
//
	return tr("Subscriptions");
}



QModelIndex Feeds_model::index(int row, int column, const QModelIndex& parent) const
{
	if(!this->hasIndex(row, column, parent))
		return QModelIndex();

	const Feed_tree* tree;

	if(parent.isValid())
		tree = this->get(parent);
	else
		tree = &this->feed_tree;

	return createIndex(row, column,
		static_cast<void*>(const_cast<Feed_tree_item*>( tree->get_child(row) )) );
}



void Feeds_model::item_marked_as_read(const QList<Big_id>& label_ids, Big_id feed_id, bool read)
{
	MLIB_D("Feed's [%1] item's read status changed. Updating model...", feed_id);

	Feed_tree_item* feed_item;

	if(( feed_item = this->lonely_feeds.value(feed_id, NULL) ))
	{
		feed_item->unread_items += ( read ? -1 : 1 );

		Feed_tree_item* parent = feed_item->get_parent();
		QModelIndex index = this->index(parent->get_child_id(feed_item), 0, QModelIndex());
		this->changePersistentIndex(index, index);
	}

	// TODO
	typedef QHash<Big_id, Feed_tree_item*> Label;
	Q_FOREACH(const Label& label, this->labels_feeds)
	{
		if(label.contains(feed_id))
			if(( feed_item = label.value(feed_id, NULL) ))
			{
			MLIB_D("Updating feed [%1]...", feed_id);
				feed_item->unread_items += ( read ? -1 : 1 );

				Feed_tree_item* parent = feed_item->get_parent();
				QModelIndex index = this->index(
					parent->get_child_id(feed_item), 0,
					this->index(parent->get_parent()->get_child_id(parent), 0, QModelIndex())
				);
				emit this->dataChanged(index, index);
//				this->changePersistentIndex(index, index);
//				this->changePersistentIndex(
//					this->index(parent->get_parent()->get_child_id(parent), 0, QModelIndex()),
//					this->index(parent->get_parent()->get_child_id(parent), 0, QModelIndex())
//				);
			}
	}

	MLIB_D("Model updated.");
}



QModelIndex Feeds_model::parent(const QModelIndex& index) const
{
	if(index.isValid())
	{
		const Feed_tree* parent = this->get(index)->get_parent();

		if(parent != &this->feed_tree)
			return createIndex(parent->get_parent()->get_child_id(parent), 0,
				static_cast<void*>(const_cast<Feed_tree_item*>( parent )) );
		else
			return QModelIndex();
	}
	else
		return QModelIndex();
}



int Feeds_model::rowCount(const QModelIndex& parent) const
{
	const Feed_tree* tree;

	if(parent.isValid())
		tree = this->get(parent);
	else
		tree = &this->feed_tree;

	return tree->count();
}



void Feeds_model::update_feed_tree_map(void)
{
	this->labels.clear();
	this->labels_feeds.clear();
	this->lonely_feeds.clear();
	this->update_feed_tree_item_map(&this->feed_tree);
}



void Feeds_model::update_feed_tree_item_map(Feed_tree_item* item)
{
	Big_id item_id = item->get_id();
	size_t children_count = item->count();

	if(item->is_label())
		this->labels[item_id] = item;

	for(size_t child_id = 0; child_id < children_count; child_id++)
	{
		Feed_tree_item* child = item->get_child(child_id);

		if(item->is_label())
			this->labels_feeds[item_id][child->get_id()] = child;
		else if(item->is_root())
		{
			if(child->is_feed())
				this->lonely_feeds[child->get_id()] = child;
			else
				this->update_feed_tree_item_map(child);
		}
		else
			MLIB_LE();
	}
}


}}}

