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
 * @file This file is part of KGO and defines the class GameView,
 *       which implements the view for a go game scene.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_GAMEVIEW_H
#define KGO_GAMEVIEW_H

#include <QGraphicsView>

namespace KGo {

class GameScene;

/**
 * This class represents a view on a Go game view. This widget can be
 * included into a main window.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class GameView : public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * Standard constructor. Creates a game view based on a given game scene.
     *
     * @param scene The game scene
     * @param parent The (optional) parent widget
     * @see GameScene
     */
    explicit GameView(GameScene *scene, QWidget *parent = 0);

private:
    /**
     * The resize event happens when the user changes the window size.
     *
     * @param event    The resize event
     */
    virtual void resizeEvent(QResizeEvent *event);

    void drawForeground(QPainter *painter, const QRectF &rect);

    GameScene *m_scene;     ///< Pointer to the game scene
};

} // End of namespace KGo

#endif
