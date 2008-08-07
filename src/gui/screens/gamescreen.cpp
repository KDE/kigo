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

    connect(m_gameEngine, SIGNAL(boardChanged()), this, SLOT(updateStatistics()));
}

void GameScreen::updateStatistics()
{
    GoEngine::Score score = m_gameEngine->estimateScore();
    if (score.isValid()) {
        QString scoreString;
        score.player() == GoEngine::WhitePlayer ? scoreString.append(i18n("White")) : scoreString.append(i18n("Black"));
        scoreString.append(i18n(" leads (+%1 points)\n", QString::number(score.score())));
        scoreString.append(i18n("Bounds: %1 - %2", QString::number(score.lowerBound()), QString::number(score.upperBound())));
        scoreLabel->setText(scoreString);
    }
    whiteCapturesLabel->setText(QString::number(m_gameEngine->captures(GoEngine::WhitePlayer)));
    blackCapturesLabel->setText(QString::number(m_gameEngine->captures(GoEngine::BlackPlayer)));
}

} // End of namespace KGo

#include "moc_gamescreen.cpp"
