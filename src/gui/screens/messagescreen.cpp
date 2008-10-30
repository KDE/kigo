/***************************************************************************
 *   Copyright (C) 2008 by Sascha Peilicke <sasch.pe@gmx.de>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "messagescreen.h"

#include <KStandardGuiItem>

namespace KGo {

MessageScreen::MessageScreen(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    KGuiItem configureItem = KStandardGuiItem::configure();
    configureButton->setText(configureItem.text());
    configureButton->setIcon(configureItem.icon());
    configureButton->setToolTip(configureItem.toolTip());
    configureButton->setWhatsThis(configureItem.whatsThis());
    connect(configureButton, SIGNAL(clicked()), this, SIGNAL(configureButtonClicked()));

    KGuiItem quitItem = KStandardGuiItem::quit();
    quitButton->setText(quitItem.text());
    quitButton->setIcon(quitItem.icon());
    quitButton->setToolTip(quitItem.toolTip());
    quitButton->setWhatsThis(quitItem.whatsThis());
    connect(quitButton, SIGNAL(clicked()), this, SIGNAL(quitButtonClicked()));
}

void MessageScreen::setMessageMessage(const QString &errorMessage)
{
    errorMessageLabel->setText(errorMessage);
}

} // End of namespace KGo

#include "moc_messagescreen.cpp"
