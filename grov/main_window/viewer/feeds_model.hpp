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


#ifndef GROV_HEADER_MAIN_WINDOW_VIEWER_FEEDS_MODEL
#define GROV_HEADER_MAIN_WINDOW_VIEWER_FEEDS_MODEL

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMultiMap>

#include <grov/common.hpp>
#include <grov/common/feed_tree.hpp>

#include <grov/client/storage.hxx>


namespace grov { namespace main_window { namespace viewer {


/// Model that represents feeds and labels tree.
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
		Feeds_model(client::Storage* storage, QObject *parent = 0);


	private:
		/// All offline data.
		client::Storage*					storage;

		/// Current feed tree.
		Feed_tree							feed_tree;

		/// Contains pointers to Feed_tree_items that represents feeds.
		QMultiMap<Big_id, Feed_tree_item*>	feeds;


	// QAbstractItemModel interface.
	public:
		virtual int				columnCount(const QModelIndex& parent) const;
		virtual QVariant		data(const QModelIndex&, int role) const;
		virtual Qt::ItemFlags	flags(const QModelIndex& index) const;
		virtual QVariant		headerData(int section, Qt::Orientation orientation, int role) const;
		virtual QModelIndex		index(int row, int column, const QModelIndex& parent) const;
		virtual QModelIndex		parent(const QModelIndex& index) const;
		virtual int				rowCount(const QModelIndex& parent) const;

	public:
		/// Returns a next or a previous feed or label with unread items if
		/// such exists.
		///
		/// @param index - index from which start search.
		/// @param next - find next or previous feed/label.
		/// @param include_star_pos - does we need return index if it has unread items.
		QModelIndex				get_next_unread(const QModelIndex& index, bool next, bool include_star_pos);

	private:
		// Returns Feed_tree_item by QModelIndex.
		const Feed_tree_item*	get(const QModelIndex& index) const;

		/// Updates pointers to feeds and labels.
		void					update_feed_tree_map(void);

		/// Updates pointers to feeds and labels for item.
		void					update_feed_tree_item_map(Feed_tree_item* item);


	private slots:
		/// Called when feeds tree changed in DB.
		void	feed_tree_changed(void);

		/// Called when an item is marked as read/unread.
		void	item_marked_as_read(const QList<Big_id>& feed_ids, bool read);
};


}}}

#endif

