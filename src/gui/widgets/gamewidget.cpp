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

#include "gamewidget.h"
#include "game/engine.h"
#include "game/stone.h"
#include "preferences.h"

#include <KDebug>

namespace Kigo {

GameWidget::GameWidget(Engine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    setupUi(this);
    connect(m_engine, SIGNAL(changed()), this, SLOT(update()));
}

void GameWidget::init()
{
    if (!m_engine->isRunning()) {
        kDebug() << "Engine is not running, no information update";
        return;
    }

    if (Preferences::whitePlayerHuman())
        whitePlayerName->setText(Preferences::whitePlayerName());
    else
        whitePlayerName->setText(i18n("Computer (Level %1)", Preferences::whitePlayerStrength()));
    if (Preferences::blackPlayerHuman())
        blackPlayerName->setText(Preferences::blackPlayerName());
    else
        blackPlayerName->setText(i18n("Computer (Level %1)", Preferences::blackPlayerStrength()));
    komiSpinBox->setValue(m_engine->komi());
    handicapSpinBox->setValue(m_engine->fixedHandicap());

    update();
}

void GameWidget::update()
{
    moveSpinBox->setValue(m_engine->currentMoveNumber());

    if (m_engine->moves().size() > 0) {
        Move last = m_engine->lastMove();
        if (last.player().isWhite())
            moveSpinBox->setSuffix(i18n(" (White %1)", last.stone().toString()));
        else if (last.player().isBlack())
            moveSpinBox->setSuffix(i18n(" (Black %1)", last.stone().toString()));
        else
            moveSpinBox->setSuffix("");
    }

    if (m_engine->currentPlayer().isWhite())
        playerLabel->setText(i18n("White's move"));
    else if (m_engine->currentPlayer().isBlack())
        playerLabel->setText(i18n("Black's move"));
    else
        playerLabel->setText("");

    whiteCapturesLabel->setText(QString::number(m_engine->captures(m_engine->whitePlayer())));
    blackCapturesLabel->setText(QString::number(m_engine->captures(m_engine->blackPlayer())));
}

} // End of namespace Kigo

#include "moc_gamewidget.cpp"

