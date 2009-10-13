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

#ifndef KIGO_GAMESCENE_H
#define KIGO_GAMESCENE_H

#include <KGamePopupItem>

#include <QGraphicsScene>

namespace Kigo {

class Game;

/**
 * This class provides a graphical representation of the go game using
 * QGraphicsScene.
 *
 * It displays the go board in its current state, receives mouse events,
 * translates them and interacts with the Game. It also drives the game
 * flow, i.e. tells game when to make the next move.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    GameScene(Game *game, QObject *parent = 0);

signals:
    void cursorPixmapChanged(const QPixmap &);

public slots:
    void resizeScene(int width, int height);
    void showLabels(bool show);
    void showHint(bool show);
    void showMoveNumbers(bool show);
    void showMessage(const QString &message, int msecs = 2000);
    void showTerritory(bool show);

private slots:
    void updateStoneItems();
    void updateHintItems();
    void changeBoardSize(int size);
    void hideHint() { showHint(false); }
    void themeChanged();

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void drawBackground(QPainter *painter, const QRectF &);

    Game *m_game;                           ///< Go game

    KGamePopupItem m_gamePopup;
    bool m_showLabels;                      ///< Show board labels or not
    bool m_showHint;
    bool m_showMoveNumbers;
    bool m_showTerritory;
    QRectF m_boardRect;                     ///< Position of board in the scene
    QRectF m_mouseRect;                     ///< Board mouse interaction rect
    QRectF m_gridRect;                      ///< Board grid rect
    qreal m_cellSize;                       ///< Width of board grid cell
    QSize m_stonePixmapSize;                ///< Size of Go stone pixmap
    QSize m_placementMarkerPixmapSize;
    int m_boardSize;                        ///< Go board size (9, 13, 19, ..)
    QGraphicsPixmapItem *m_placementMarkerItem;

    QList<QGraphicsPixmapItem *> m_stoneItems;
    QList<QGraphicsPixmapItem *> m_hintItems;
};

} // End of namespace Kigo

#endif
