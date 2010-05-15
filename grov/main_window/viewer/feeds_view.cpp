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


#include <QtCore/QModelIndex>

#include <QtGui/QHeaderView>
#include <QtGui/QItemSelection>
#include <QtGui/QItemSelectionModel>

#include <grov/common.hpp>

#include "feeds_model.hpp"

#include "feeds_view.hpp"


namespace grov { namespace main_window { namespace viewer {


Feeds_view::Feeds_view(QWidget *parent)
:
	QTreeView(parent),
	storage(NULL)
{
}



void Feeds_view::connect_to_storage(client::Storage* storage)
{
	MLIB_A(!this->storage);

	this->storage = storage;
	this->model = new Feeds_model(this->storage, this);
	this->setModel(this->model);
	this->selection = this->selectionModel();

	connect(this->selection, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		SLOT(selection_changed(const QItemSelection&, const QItemSelection&)) );
}



void Feeds_view::go_to_next_unread(void)
{
	this->go_to_unread(true);
}



void Feeds_view::go_to_previous_unread(void)
{
	this->go_to_unread(false);
}



void Feeds_view::go_to_unread(bool next)
{
	QModelIndex index;
	bool include_start_pos = false;

	// Getting start position -->
	{
		QModelIndexList selected = this->selection->selectedIndexes();

		if(selected.isEmpty())
		{
			int rows = this->model->rowCount(index);

			if(rows)
			{
				index = this->model->index(next ? 0 : rows - 1, 0, index);
				include_start_pos = true;
			}
		}
		else
			index = selected.at(0);
	}
	// Getting start position <--

	// There is no feeds at all
	if(!index.isValid())
		return;

	// Going to next feed/label with unread items -->
		index = this->model->get_next_unread(index, next, include_start_pos);

		if(index.isValid())
		{
			this->selection->select(index, QItemSelectionModel::SelectCurrent);
			this->selection->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
		}
		else
			this->select_no_items();
	// Going to next feed/label with unread items <--
}



void Feeds_view::select_no_items(void)
{
	this->selection->clear();
}



void Feeds_view::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
{
	QModelIndexList selected_indexes = selected.indexes();

	if(selected_indexes.isEmpty())
		emit unselected();
	else
	{
		const QModelIndex& index = selected_indexes.at(0);

		if(index.data(Feeds_model::ROLE_IS_FEED).toBool())
			emit feed_selected(m::qvariant_to_big_id(index.data(Feeds_model::ROLE_ID)));
		else
			emit label_selected(m::qvariant_to_big_id(index.data(Feeds_model::ROLE_ID)));
	}
}


}}}


