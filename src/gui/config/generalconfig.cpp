/*
    Copyright 2008 Sascha Peilicke <sasch.pe@gmx.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "generalconfig.h"
#include "preferences.h"
#include "game/game.h"

namespace Kigo {

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
    Game game;
    if (game.start(kcfg_EngineCommand->text())) {
        engineLed->setState(KLed::On);
    } else {
        engineLed->setState(KLed::Off);
    }
}

} // End of namespace Kigo

#include "moc_generalconfig.cpp"
