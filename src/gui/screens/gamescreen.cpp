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
 * @file This file is part of KGO and implements the class GameScreen,
 *       which acts as a compound widget to display the game view and
 *       several additional information panels.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#include "gamescreen.h"
#include "game/goengine.h"
#include "gui/graphicsview/gamescene.h"
#include "gui/graphicsview/gameview.h"
#include "preferences.h"

#include <KDebug>

namespace KGo {

GameScreen::GameScreen(GameScene *scene, QWidget *parent)
    : QWidget(parent)
    , m_gameEngine(scene->engine())
{
    if (!m_gameEngine->isRunning())
        kFatal() << "No Go engine is running!";             // Engine should really be running here!

    setupUi(this);
    GameView *gameView = new GameView(scene, this);
    gameView->setInteractive(true);
    gameFrame->setLayout(new QHBoxLayout());
    gameFrame->layout()->addWidget(gameView);
    connect(countButton, SIGNAL(clicked()), this, SLOT(showScoreEstimates()));
    connect(m_gameEngine, SIGNAL(boardChanged()), this, SLOT(updateStatistics()));
}

void GameScreen::showEvent(QShowEvent *)
{
    if (Preferences::whitePlayerHuman())
        whitePlayerName->setText(Preferences::whitePlayerName());
    else
        whitePlayerName->setText(i18n("Computer (Level %1)", Preferences::whitePlayerStrength()));
    if (Preferences::blackPlayerHuman())
        blackPlayerName->setText(Preferences::blackPlayerName());
    else
        blackPlayerName->setText(i18n("Computer (Level %1)", Preferences::blackPlayerStrength()));
    komiSpinBox->setValue(m_gameEngine->komi());
    handicapSpinBox->setValue(m_gameEngine->fixedHandicap());

    updateStatistics();
}

void GameScreen::updateStatistics()
{
    moveSpinBox->setValue(m_gameEngine->moveNumber());

    QPair<GoEngine::Stone, GoEngine::PlayerColor> lastMove = m_gameEngine->lastMove();
    switch (lastMove.second) {
        case GoEngine::WhitePlayer: moveSpinBox->setSuffix(i18n("  (W %1)", lastMove.first.toString())); break;
        case GoEngine::BlackPlayer: moveSpinBox->setSuffix(i18n("  (B %1)", lastMove.first.toString())); break;
        case GoEngine::InvalidPlayer: moveSpinBox->setSuffix(""); break;
    }
    switch (m_gameEngine->currentPlayer()) {
        case GoEngine::WhitePlayer: playerLabel->setText(i18n("White's move")); break;
        case GoEngine::BlackPlayer: playerLabel->setText(i18n("Black's move")); break;
        case GoEngine::InvalidPlayer: playerLabel->setText(""); break;
    }

    whiteCapturesLabel->setText(QString::number(m_gameEngine->captures(GoEngine::WhitePlayer)));
    blackCapturesLabel->setText(QString::number(m_gameEngine->captures(GoEngine::BlackPlayer)));
}

void GameScreen::showScoreEstimates()
{
    kDebug() << "TODO: Show score estimates ...";
}

} // End of namespace KGo

#include "moc_gamescreen.cpp"
