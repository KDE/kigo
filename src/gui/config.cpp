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
 * @file This file is part of KGO and implements the class GeneralConfig,
 *       which displays the general configuration options tab in the
 *       Kgo configuration screen.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#include "config.h" // krazy:exclude=includes
#include "preferences.h"
#include "game/goengine.h"

namespace KGo {

GeneralConfig::GeneralConfig(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    QString engineCommand = Preferences::engineCommand();
    engineExecutable->setUrl(KUrl::fromLocalFile(engineCommand.section(' ', 0, 0)));
    engineParameters->setText(engineCommand.section(' ', 1));
    connect(engineCheckButton, SIGNAL(clicked()), this, SLOT(checkEngine()));
}

GeneralConfig::~GeneralConfig()
{
    QString engineCommand = engineExecutable->url().toLocalFile() + ' ' + engineParameters->text();
    Preferences::setEngineCommand(engineCommand);
    Preferences::self()->writeConfig();
}

void GeneralConfig::checkEngine()
{
    GoEngine engine;
    QString engineCommand = engineExecutable->url().toLocalFile() + ' ' + engineParameters->text();
    kDebug() << "Checking" << engineCommand;
    if(engine.start(engineCommand) && !engine.name().isEmpty())
        engineCheckLabel->setText("<span style=\"color: green;\">" + i18n("Works") + "</span>");
    else
        engineCheckLabel->setText("<span style=\"color: red;\">" + i18n("Error") + "</span>");
}

} // End of namespace KGo

#include "moc_config.cpp"
