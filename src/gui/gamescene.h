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

class GoEngine;

/**
 * This class provides a graphical representation of the go game using
 * QGraphicsScene.
 *
 * It displays the go board in its current state, receives mouse events,
 * translates them and interacts with the GoEngine. It also drives the game
 * flow, i.e. tells game when to make the next move.
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

    GoEngine * const engine() const;

signals:
    void cursorChanged(QPixmap);

public slots:
    void updateBoard();
    void boardSizeChanged(int size);
    void showMoveHistory(bool show);
    void showLabels(bool show);
    void showHint();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    /**
     * Used to draw the scene background and the Go board.
     *
     * @param painter
     * @param
     * @see QGraphicsScene::drawBackground()
     */
    void drawBackground(QPainter *painter, const QRectF &);

    GoEngine * const m_engine;                      ///< Go engine
    bool m_showLabels;                              ///< Show board labels or not
    QRectF m_boardRect;                             ///< Position of board in the scene
    QRectF m_boardGridRect;                         ///<
    QRectF m_boardMouseRect;
    qreal m_boardGridCellSize;                      ///<
    int m_boardSize;                                ///< Go board size (9, 13, 19, ..)
    QList<QGraphicsPixmapItem *> m_stoneItemList;   ///<
};

} // End of namespace KGo

#endif
