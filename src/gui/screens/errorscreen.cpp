/*******************************************************************
 *
 * Copyright 2008 Sascha Peilicke <sasch.pe@gmx.de>
 *
 * This file is part of the KDE project "KGo"
 *
 * KGo is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KGo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KReversi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *******************************************************************/

/**
 * @file This file is part of KGO and implements the class ErrorScreen,
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#include "errorscreen.h"

#include <KStandardGuiItem>

namespace KGo {

ErrorScreen::ErrorScreen(QWidget *parent)
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

void ErrorScreen::setErrorMessage(const QString &errorMessage)
{
    errorMessageLabel->setText(errorMessage);
}

} // End of namespace KGo

#include "moc_errorscreen.cpp"
