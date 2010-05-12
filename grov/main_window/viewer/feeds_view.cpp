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
	this->setWordWrap(true);
	this->setTextElideMode(Qt::ElideMiddle);
}



void Feeds_view::connect_to_storage(client::Storage* storage)
{
	MLIB_A(!this->storage);

	this->storage = storage;
	this->model = new Feeds_model(this->storage, this);
	this->setModel(this->model);
	this->selection = this->selectionModel();

	connect(this->selection, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this, SLOT(selection_changed(const QItemSelection&, const QItemSelection&)) );
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


