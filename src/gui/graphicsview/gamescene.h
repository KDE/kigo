/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_GAMESCENE_H
#define KIGO_GAMESCENE_H

#include <KGamePopupItem>

#include <QGraphicsScene>

class QTimer;

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
    explicit GameScene(Game *game, QObject *parent = nullptr);

Q_SIGNALS:
    void cursorPixmapChanged(const QPixmap &);

public Q_SLOTS:
    void resizeScene(int width, int height);
    void showLabels(bool show);
    void showHint(bool show);
    void showMoveNumbers(bool show);
    void showMessage(const QString &message, int msecs = 2000);
    void showPlacementMarker(bool show);
    void showTerritory(bool show);

private Q_SLOTS:
    void updateStoneItems();
    void updateHintItems();
    void updateTerritoryItems();
    void changeBoardSize(int size);
    void hideHint() { showHint(false); }
    void themeChanged();

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &) override;

    Game *m_game;                           ///< Go game

    KGamePopupItem m_gamePopup;
    bool m_showLabels;                      ///< Show board labels or not
    bool m_showHint;
    QTimer *m_hintTimer;
    bool m_showMoveNumbers;
    bool m_showPlacementMarker;
    bool m_showTerritory;
    QRectF m_boardRect;                     ///< Position of board in the scene
    QRectF m_mouseRect;                     ///< Board mouse interaction rect
    QRectF m_gridRect;                      ///< Board grid rect
    int m_cellSize;                         ///< Width of board grid cell
    QSize m_stonePixmapSize;                ///< Size of Go stone pixmap
    QSize m_placementMarkerPixmapSize;
    int m_boardSize;                        ///< Go board size (9, 13, 19, ..)
    QGraphicsPixmapItem *m_placementMarkerItem;

    QList<QGraphicsPixmapItem *> m_stoneItems;
    QList<QGraphicsPixmapItem *> m_hintItems;
    QList<QGraphicsPixmapItem *> m_territoryItems;
};

} // End of namespace Kigo

#endif
