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
	connect(this->storage, SIGNAL(feed_tree_changed(const Feed_tree&)),
		this, SLOT(on_feed_tree_changed(const Feed_tree&)) );
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
			return item->get_name();
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



void Feeds_model::on_feed_tree_changed(const Feed_tree& feed_tree)
{
	MLIB_D("Feed tree changed. Updating it...");

	if(this->feed_tree.count())
	{
		this->beginRemoveRows(QModelIndex(), 0, this->feed_tree.count() - 1);
		this->endRemoveRows();
	}

	this->feed_tree = feed_tree;

	if(this->feed_tree.count())
	{
		this->beginInsertRows(QModelIndex(), 0, this->feed_tree.count() - 1);
		this->endInsertRows();
	}
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


}}}

