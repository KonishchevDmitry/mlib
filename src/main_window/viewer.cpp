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
#include <src/common/feed_item.hpp>

#include <src/client/storage.hpp>

#include "viewer.hpp"
#include "ui_viewer.h"


namespace grov { namespace main_window {


Viewer::Viewer(QWidget *parent)
:
	QWidget(parent),
    ui(new Ui::Viewer),
	storage(NULL),
	current_item_id(-1)
{
	ui->setupUi(this);
	ui->star_check_box->addAction(ui->star_action);
	this->set_no_selected_feed();

}



Viewer::~Viewer(void)
{
	delete ui;
}



void Viewer::connect_to_storage(client::Storage* storage)
{
	MLIB_A(!this->storage);
	this->storage = storage;

	ui->feeds_view->connect_to_storage(this->storage);

	connect(ui->feeds_view, SIGNAL(unselected()),
		this, SLOT(set_no_selected_feed()) );

	connect(ui->feeds_view, SIGNAL(feed_selected(Big_id)),
		this, SLOT(feed_selected(Big_id)) );

	connect(ui->feeds_view, SIGNAL(label_selected(Big_id)),
		this, SLOT(label_selected(Big_id)) );
}



void Viewer::feed_selected(Big_id id)
{
	this->storage->set_current_source_to_feed(id);
	this->reset_current_item();
	this->go_to_next_item();
}



void Viewer::go_to_next_item(void)
{
	try
	{
		if(this->current_item_id >= 0)
			this->storage->mark_as_read(this->current_item_id);
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Unable to mark feed's item as read"), EE(e));
		return;
	}

	try
	{
		Feed_item item = this->storage->get_next_item();
		this->set_current_item(item);
	}
	catch(client::Storage::No_more_items&)
	{
		this->set_no_more_items();
	}
	catch(client::Storage::No_selected_items&)
	{
		this->set_no_selected_feed();
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Unable to fetch feed's item"), EE(e));
	}
}



void Viewer::go_to_previous_item(void)
{
	try
	{
		Feed_item item = this->storage->get_previous_item();
		this->set_current_item(item);
	}
	catch(client::Storage::No_more_items&) { }
	catch(client::Storage::No_selected_items&) { }
	catch(m::Exception& e)
	{
		MLIB_W(tr("Unable to fetch feed's item"), EE(e));
	}
}



void Viewer::label_selected(Big_id id)
{
	this->storage->set_current_source_to_label(id);
	this->reset_current_item();
	this->go_to_next_item();
}



void Viewer::on_star_check_box_stateChanged(int state)
{
	try
	{
		this->storage->star(this->current_item_id, state);
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Unable to star feed's item"), EE(e));
	}
}



void Viewer::reset_current_item(void)
{
	this->current_item_id = -1;
	ui->controls_box->setVisible(false);
}



void Viewer::set_current_item(const Feed_item& item)
{
	QString html;

	html += "<html><body>";
	if(!item.title.isEmpty())
		html += "<h1>" + item.title + "</h1>";
	html += item.summary;
	html += "</body></html>";

	ui->items_view->setHtml(html);

	this->current_item_id = item.id;

	ui->star_check_box->blockSignals(true);
	ui->star_check_box->setChecked(item.starred);
	ui->star_check_box->blockSignals(false);
	ui->controls_box->setVisible(true);
}



void Viewer::set_no_more_items(void)
{
	QString html;

	html += "<html><body><center>";
	html += tr("You have no unread items.");
	html += "</center></body></html>";

	ui->items_view->setHtml(html);

	this->reset_current_item();
}



void Viewer::set_no_selected_feed(void)
{
	QString html;

	html += "<html><body><center>";
	html += tr("Please select a label or a feed to view its items.");
	html += "</center></body></html>";

	ui->items_view->setHtml(html);

	this->reset_current_item();
}


}}

