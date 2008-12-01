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

#ifndef KIGO_GAMEVIEW_H
#define KIGO_GAMEVIEW_H

#include <QGraphicsView>

namespace Kigo {

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
};

} // End of namespace Kigo

#endif
