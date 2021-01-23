/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "generalconfig.h"
#include "preferences.h"
#include "game/game.h"
#include <QUrl>
#include <QLineEdit>

namespace Kigo {

GeneralConfig::GeneralConfig(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    kcfg_EngineCommand->hide(); // Only used internally

    const QString engineCommand = Preferences::engineCommand();
    engineExecutable->setUrl(QUrl::fromLocalFile(engineCommand.section(' ', 0, 0)));
    connect(engineExecutable, &KUrlRequester::textChanged, this, &GeneralConfig::updateEngineCommand);
    engineParameters->setText(engineCommand.section(' ', 1));
    connect(engineParameters, &QLineEdit::textChanged, this, &GeneralConfig::updateEngineCommand);

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
