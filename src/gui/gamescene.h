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
 * @file This file is part of KGO and defines the class GameScene,
 *       which displays the go game.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_GAMESCENE_H
#define KGO_GAMESCENE_H

#include <QGraphicsScene>

class QGraphicsPixmapItem;

namespace KGo {

class Gtp;

/**
 * This class provides a graphical representation of the go game using
 * QGraphicsScene.
 *
 * It displays the go board in its current state, receives mouse events,
 * translates them so that KGo::Gtp can understand, send them to it,
 * receives board-changed notificationi and nicely animates them.
 *
 * It also drives the game flow, i.e. tells game when to make the
 * next move.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    GameScene();

    void resizeScene(int width, int height);

    Gtp * const gtp() const;

public slots:
    void updateBoard();
    void showMoveHistory(bool show);
    void hint();

private:
    /**
     * Used to draw the scene background and the Go board.
     *
     * @param painter
     * @param
     * @see QGraphicsScene::drawBackground()
     */
    void drawBackground(QPainter *painter, const QRectF &);

    Gtp * const m_gtp;                              ///< To interface with the go engine
    bool m_showLabels;                              ///< Show board labels or not
    qreal m_currentCellSize;                        ///<
    QRectF m_boardRect;                             ///< Position of board in the scene
    QList<QGraphicsPixmapItem *> m_stoneItemList;   ///<
    QGraphicsPixmapItem *m_cursorItem;              ///<
};

} // End of namespace KGo

#endif
