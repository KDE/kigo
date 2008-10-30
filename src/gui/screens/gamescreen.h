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

#ifndef KGO_GAMESCREEN_H
#define KGO_GAMESCREEN_H

#include "ui_gamescreen.h"

#include <QWidget>

namespace KGo {

class GameScene;
class OldGoEngine;

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
    void showEvent(QShowEvent *);
    void updateStatistics();
    void handlePass(int);
    void handleResign();
    void scoreEstimates();
    void finishGame();

private:
    OldGoEngine * const m_gameEngine;  ///<
};

} // End of namespace KGo

#endif
