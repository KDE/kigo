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

private slots:
    void changeCursor(const QPixmap &cursorPixmap);

private:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);
    void drawForeground(QPainter *painter, const QRectF &rect);

    GameScene * const m_gameScene;  ///< Pointer to the game scene

    bool m_renderInactive;
};

} // End of namespace KGo

#endif
