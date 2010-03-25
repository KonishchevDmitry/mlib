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


#include <src/client.hpp>
#include <src/common.hpp>
#include <src/feeds_model.hpp>

#include "main_window.hpp"
#include "ui_main_window.h"


namespace grov {


Main_window::Main_window(const QString user, const QString password, QWidget *parent)
:
	QMainWindow(parent),
	ui(new Ui::Main_window)
{
    ui->setupUi(this);
	ui->items_view->setVisible(false);


	// TODO
	this->client = new Client(user, password, this);

	connect(this->client, SIGNAL(mode_changed(Client::Mode)),
		this, SLOT(mode_changed(Client::Mode)) );


	// TODO
	ui->feeds_view->connect_to_storage(this->client);

	connect(ui->feeds_view, SIGNAL(unselected()),
		this, SLOT(set_no_selected_feed()) );

	connect(ui->feeds_view, SIGNAL(feed_selected(Big_id)),
		this, SLOT(feed_selected(Big_id)) );

	connect(ui->feeds_view, SIGNAL(label_selected(Big_id)),
		this, SLOT(label_selected(Big_id)) );
}



Main_window::~Main_window()
{
    delete ui;
}



void Main_window::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);

	switch(e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;

		default:
			break;
	}
}



void Main_window::feed_selected(Big_id id)
{
	client->set_current_source_to_feed(id);
	this->on_next_item_action_activated();
}



void Main_window::label_selected(Big_id id)
{
	client->set_current_source_to_label(id);
	this->on_next_item_action_activated();
}



void Main_window::mode_changed(Client::Mode mode)
{
	ui->go_offline_action->setVisible(mode == Client::NONE);
	ui->flush_offline_data_action->setVisible(mode == Client::OFFLINE);

	ui->items_view->setVisible(mode == Client::OFFLINE);

	// TODO
	if(mode == Client::OFFLINE)
		this->on_next_item_action_activated();
}



void Main_window::on_go_offline_action_activated(void)
{
	this->client->download();
}



void Main_window::on_next_item_action_activated(void)
{
	try
	{
		Feed_item item = this->client->get_next_item();
		this->set_current_item(item);
	}
	catch(Storage::No_more_items&)
	{
		this->set_no_more_items();
	}
	catch(m::Exception& e)
	{
		MLIB_W(tr("Unable to fetch feed's item"), EE(e));
	}
}



void Main_window::on_previous_item_action_activated(void)
{
	try
	{
		Feed_item item = this->client->get_previous_item();
		this->set_current_item(item);
	}
	catch(Storage::No_more_items&) { }
	catch(m::Exception& e)
	{
		MLIB_W(tr("Unable to fetch feed's item"), EE(e));
	}
}



void Main_window::set_current_item(const Feed_item& item)
{
	QString html;

	html += "<html><body>";
	if(!item.title.isEmpty())
		html += "<h1>" + item.title + "</h1>";
	html += item.summary;
	html += "</body></html>";

	ui->items_view->setHtml(html);
}



void Main_window::set_no_more_items(void)
{
	QString html;

	html += "<html><body><center>";
	html += tr("You have no unread items.");
	html += "</center></body></html>";

	ui->items_view->setHtml(html);
}



void Main_window::set_no_selected_feed(void)
{
	QString html;

	html += "<html><body><center>";
	html += tr("Please select a label or a feed to view its items.");
	html += "</center></body></html>";

	ui->items_view->setHtml(html);
}


}

