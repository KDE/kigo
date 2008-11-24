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

#include "gamewidget.h"
#include "game/goengine.h"
#include "preferences.h"

#include <KDebug>

namespace Kigo {

GameWidget::GameWidget(GoEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    setupUi(this);
    connect(m_engine, SIGNAL(boardChanged()), this, SLOT(update()));
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

    QPair<GoEngine::Stone, GoEngine::PlayerColor> lastMove = m_engine->lastMove();
    switch (lastMove.second) {
        case GoEngine::WhitePlayer:
            moveSpinBox->setSuffix(i18n("  (W %1)", lastMove.first.toString())); break;
        case GoEngine::BlackPlayer:
            moveSpinBox->setSuffix(i18n("  (B %1)", lastMove.first.toString())); break;
        case GoEngine::InvalidPlayer:
            moveSpinBox->setSuffix(""); break;
    }
    switch (m_engine->currentPlayer()) {
        case GoEngine::WhitePlayer: playerLabel->setText(i18n("White's move")); break;
        case GoEngine::BlackPlayer: playerLabel->setText(i18n("Black's move")); break;
        case GoEngine::InvalidPlayer: playerLabel->setText(""); break;
    }

    whiteCapturesLabel->setText(QString::number(m_engine->captures(GoEngine::WhitePlayer)));
    blackCapturesLabel->setText(QString::number(m_engine->captures(GoEngine::BlackPlayer)));
}

} // End of namespace Kigo

#include "moc_gamewidget.cpp"

