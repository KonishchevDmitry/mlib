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

// TODO:

//#include <src/client.hpp>

#include "feeds_view.hpp"


namespace grov {


Feeds_view::Feeds_view(QWidget *parent)
:
	QTreeView(parent)
{
#if 0
    ui->setupUi(this);
	ui->items_view->setVisible(false);

	// TODO
	this->client = new Client(user, password, this);

	connect(this->client, SIGNAL(mode_changed(Client::Mode)),
		this, SLOT(mode_changed(Client::Mode)) );
#endif
}



#if 0
Feeds_view::~Feeds_view()
{
    delete ui;
}



void Feeds_view::changeEvent(QEvent *e)
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



void Feeds_view::mode_changed(Client::Mode mode)
{
	ui->go_offline_action->setVisible(mode == Client::NONE);
	ui->flush_offline_data_action->setVisible(mode == Client::OFFLINE);

	ui->items_view->setVisible(mode == Client::OFFLINE);

	// TODO
	if(mode == Client::OFFLINE)
		this->on_next_item_action_activated();
}



void Feeds_view::on_go_offline_action_activated(void)
{
	this->client->download();
}



void Feeds_view::on_next_item_action_activated(void)
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



void Feeds_view::on_previous_item_action_activated(void)
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



void Feeds_view::set_current_item(const Feed_item& item)
{
	QString html;

	html += "<html><body>";
	if(!item.title.isEmpty())
		html += "<h1>" + item.title + "</h1>";
	html += item.summary;
	html += "</body></html>";

	ui->items_view->setHtml(html);
}



void Feeds_view::set_no_more_items(void)
{
	QString html;

	html += "<html><body><center>";
	html += tr("You have no unread items.");
	html += "</center></body></html>";

	ui->items_view->setHtml(html);
}
#endif


}

