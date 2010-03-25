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

#include "main_window.hpp"
#include "ui_main_window.h"


namespace grov {


Main_window::Main_window(const QString user, const QString password, QWidget *parent)
:
	QMainWindow(parent),
	ui(new Ui::Main_window)
{
    ui->setupUi(this);

	// Client -->
		// TODO
		this->client = new Client(user, password, this);

		connect(this->client, SIGNAL(mode_changed(Client::Mode)),
			this, SLOT(mode_changed(Client::Mode)) );
	// Client <--

	// Viewer -->
		ui->viewer->connect_to_storage(this->client);

		connect(ui->next_item_action, SIGNAL(activated()),
			ui->viewer, SLOT(go_to_next_item()) );

		connect(ui->previous_item_action, SIGNAL(activated()),
			ui->viewer, SLOT(go_to_previous_item()) );
	// Viewer <--

	this->mode_changed(Client::MODE_NONE);
}



Main_window::~Main_window()
{
    delete ui;
}



void Main_window::mode_changed(Client::Mode mode)
{
	ui->go_offline_action->setVisible(mode == Client::MODE_NONE);
	ui->flush_offline_data_action->setVisible(mode == Client::MODE_OFFLINE);

	ui->viewer->setVisible(mode == Client::MODE_OFFLINE);

	// TODO may be reset Viewer
}



void Main_window::on_go_offline_action_activated(void)
{
	this->client->download();
}


}

