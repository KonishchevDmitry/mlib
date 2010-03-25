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

#ifndef GROV_HEADER_FEEDS_MODEL
#define GROV_HEADER_FEEDS_MODEL

#include <QtCore/QAbstractItemModel>

#include <src/common.hpp>
#include <src/feed_tree.hpp>
#include <src/storage.hxx>


namespace grov {

class Feeds_model: public QAbstractItemModel
{
	Q_OBJECT

	public:
		enum {
			/// Is tree item a feed or a label.
			ROLE_IS_FEED = Qt::UserRole,

			/// Feed's or label's id.
			ROLE_ID
		};


	public:
		Feeds_model(Storage* storage, QObject *parent = 0);


	private:
		/// All offline data.
		Storage*	storage;

		/// Current feed tree.
		Feed_tree	feed_tree;


	public:
		virtual int				columnCount(const QModelIndex& parent) const;
		virtual QVariant		data(const QModelIndex&, int role) const;
		virtual Qt::ItemFlags	flags(const QModelIndex& index) const;
		virtual QVariant		headerData(int section, Qt::Orientation orientation, int role) const;
		virtual QModelIndex		index(int row, int column, const QModelIndex& parent) const;
		virtual QModelIndex		parent(const QModelIndex& index) const;
		virtual int				rowCount(const QModelIndex& parent) const;

	private:
		// Returns Feed_tree_item by QModelIndex.
		const Feed_tree_item*	get(const QModelIndex& index) const;


	private slots:
		/// Emitted when feeds tree changed in DB.
		void	on_feed_tree_changed(const Feed_tree& feed_tree);
};

}

#endif

