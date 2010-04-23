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


#ifndef GROV_HEADER_COMMON_FEED_TREE
#define GROV_HEADER_COMMON_FEED_TREE

#include <boost/shared_ptr.hpp>

#include <grov/common.hpp>

#include "feed_tree.hxx"


namespace grov {


/// Represents a tree of labels and feeds.
class Feed_tree_item
{
	private:
		/// Type of tree's item.
		enum Type {
			/// Root node.
			TYPE_ROOT,

			/// Label that contains feeds.
			TYPE_LABEL,

			/// Feed.
			TYPE_FEED
		};

	private:
		typedef boost::shared_ptr<Feed_tree_item> Feed_tree_item_ptr;


	public:
		Feed_tree_item(const Feed_tree_item& other);

	private:
		Feed_tree_item(Feed_tree_item* parent, Type type, Big_id id, const QString& name);


	public:
		/// Number of unread items.
		Big_id			unread_items;

	private:
		/// Parent item.
		Feed_tree_item*	parent;


		/// Type of this item.
		Type			type;

		/// Label's or feed's id.
		Big_id			id;

		/// Label's or feed's name.
		QString			name;


		/// Item's children.
		QList<Feed_tree_item_ptr>	children;


	public:
		/// Creates a new feed tree.
		static Feed_tree	create(void);


	public:
		/// Adds a feed to the label item or the feed tree.
		Feed_tree_item*			add_feed(Big_id id, const QString& name);

		/// Adds a label to the feed tree.
		Feed_tree_item*			add_label(Big_id id, const QString& name);

		/// Return number of item's children.
		size_t					count(void) const;

		/// Returns item's child.
		Feed_tree_item*			get_child(size_t id);

		/// Returns item's child.
		const Feed_tree_item*	get_child(size_t id) const;

		/// Returns item's child's id.
		size_t					get_child_id(const Feed_tree_item* child) const;

		/// Returns item's id.
		Big_id		 			get_id(void) const;

		/// Returns item's name.
		const QString&			get_name(void) const;

		/// Returns a parent item.
		Feed_tree_item*			get_parent(void);

		/// Returns a parent item.
		const Feed_tree_item*	get_parent(void) const;

		/// Is this item a feed?
		bool					is_feed(void) const;

		/// Is this item a label?
		bool					is_label(void) const;

		/// Is this item a root?
		bool					is_root(void) const;

	private:
		/// Adds an item to this item.
		Feed_tree_item*	add(Feed_tree_item* item);


	public:
		Feed_tree_item&			operator=(const Feed_tree_item& other);
};


}

#endif

