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


#include <src/common.hpp>

#include "feed_tree.hpp"


namespace grov {


Feed_tree_item::Feed_tree_item(Feed_tree_item* parent, Type type, Big_id id, const QString& name)
:
	unread_items(0),
	parent(parent),
	type(type),
	id(id),
	name(name)
{
}



Feed_tree_item::Feed_tree_item(const Feed_tree_item& other)
{
	*this = other;
}



Feed_tree_item* Feed_tree_item::add(Feed_tree_item* item)
{
	this->children << Feed_tree_item_ptr(item);
	return item;
}



Feed_tree_item* Feed_tree_item::add_feed(Big_id id, const QString& name)
{
	MLIB_A(this->type == TYPE_ROOT || this->type == TYPE_LABEL);
	return this->add(new Feed_tree_item(this, TYPE_FEED, id, name));
}



Feed_tree_item* Feed_tree_item::add_label(Big_id id, const QString& name)
{
	MLIB_A(this->type == TYPE_ROOT);
	return this->add(new Feed_tree_item(this, TYPE_LABEL, id, name));
}



size_t Feed_tree_item::count(void) const
{
	return this->children.size();
}



Feed_tree Feed_tree::create(void)
{
	return Feed_tree_item(NULL, TYPE_ROOT, -1, "");
}



Feed_tree_item* Feed_tree_item::get_child(size_t id)
{
	return this->children[id].get();
}



const Feed_tree_item* Feed_tree_item::get_child(size_t id) const
{
	return this->children[id].get();
}



size_t Feed_tree_item::get_child_id(const Feed_tree_item* child) const
{
	size_t id = 0;

	Q_FOREACH(const Feed_tree_item_ptr& item, this->children)
	{
		if(item.get() == child)
			return id;

		id++;
	}

	MLIB_LE();
}



Big_id Feed_tree_item::get_id(void) const
{
	return this->id;
}



const QString& Feed_tree_item::get_name(void) const
{
	return this->name;
}



Feed_tree_item* Feed_tree_item::get_parent(void)
{
	return this->parent;
}



const Feed_tree_item* Feed_tree_item::get_parent(void) const
{
	return this->parent;
}



bool Feed_tree_item::is_feed(void) const
{
	return this->type == TYPE_FEED;
}



bool Feed_tree_item::is_label(void) const
{
	return this->type == TYPE_LABEL;
}



bool Feed_tree_item::is_root(void) const
{
	return this->type == TYPE_ROOT;
}



Feed_tree_item& Feed_tree_item::operator=(const Feed_tree_item& other)
{
	if(this == &other)
		return *this;

	QList<Feed_tree_item_ptr> children;

	Q_FOREACH(const Feed_tree_item_ptr& item, other.children)
	{
		Feed_tree_item_ptr item_copy(new Feed_tree_item(*item));
		item_copy->parent = this;
		children << item_copy;
	}

	this->type = other.type;
	this->id = other.id;
	this->name = other.name;
	this->unread_items = other.unread_items;
	this->children = children;

	return *this;
}


}

