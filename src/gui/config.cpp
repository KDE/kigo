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
    kcfg_EngineCommand->hide(); // Only used internally

    QString engineCommand = Preferences::engineCommand();
    engineExecutable->setUrl(KUrl::fromLocalFile(engineCommand.section(' ', 0, 0)));
    connect(engineExecutable, SIGNAL(textChanged(QString)), this, SLOT(updateEngineCommand()));
    engineParameters->setText(engineCommand.section(' ', 1));
    connect(engineParameters, SIGNAL(textChanged(QString)), this, SLOT(updateEngineCommand()));

    updateEngineCommand();
}

void GeneralConfig::updateEngineCommand()
{
    kcfg_EngineCommand->setText(engineExecutable->url().toLocalFile() + ' ' + engineParameters->text());

    // Check if the configured Go engine backend actually works and tell the user
    GoEngine engine;
    if(engine.start(kcfg_EngineCommand->text()) && !engine.name().isEmpty())
        engineLed->setState(KLed::On);
    else
        engineLed->setState(KLed::Off);
}

} // End of namespace KGo

#include "moc_config.cpp"
