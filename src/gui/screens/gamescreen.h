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
 * @file This file is part of KGO and defines the class GameScreen,
 *       which acts as a compound widget to display the game view and
 *       several additional information panels.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_GAMESCREEN_H
#define KGO_GAMESCREEN_H

#include "ui_gamescreen.h"

#include <QWidget>

namespace KGo {

class GameScene;
class GoEngine;

/**
 * The game screen acts as a compound widget for the game view and
 * some additional information panels. The panels include the move
 * history and game statistics.
 *
 * @see GameView
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class GameScreen : public QWidget, private Ui::GameScreen
{
    Q_OBJECT

public:
    /**
     * Standard constructor. Creates a new game screen and sets up
     * some informational panels as well as the main Go game board
     * view.
     *
     * @param scene A configured, ready to start game scene
     * @param parent The parent widget or none
     */
    explicit GameScreen(GameScene *scene, QWidget *parent = 0);

private slots:
    void updateStatistics();

private:
    GoEngine * const m_gameEngine;  ///<
};

} // End of namespace KGo

#endif
