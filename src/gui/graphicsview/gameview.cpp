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

#include "gameview.h"
#include "gamescene.h"

#include <QResizeEvent>

namespace Kigo {

GameView::GameView(GameScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_gameScene(scene)
{
    setCacheMode(QGraphicsView::CacheBackground);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setOptimizationFlags(QGraphicsView::DontClipPainter |
                         QGraphicsView::DontSavePainterState |
                         QGraphicsView::DontAdjustForAntialiasing);

    connect(m_gameScene, SIGNAL(cursorPixmapChanged(QPixmap)), this, SLOT(changeCursor(QPixmap)));
}

void GameView::changeCursor(const QPixmap &cursorPixmap)
{
    if (!isInteractive() || cursorPixmap.isNull())
        unsetCursor();
    else
        setCursor(QCursor(cursorPixmap));
}

void GameView::drawForeground(QPainter * /*painter*/, const QRectF & /*rect*/)
{
    //TODO: Get rid of that ugly looking white pixels on the left and top of rect
    /*if (!isInteractive()) {
        painter->save();
        painter->setBrush(QBrush(QColor(60,60,60,100), Qt::Dense4Pattern));
        painter->drawRect(rect);
        painter->restore();
    }*/
}

void GameView::showEvent(QShowEvent *)
{
    // Make sure that the game scene has the correct size according to the current view
    // This is necessary because one scene is shared by multiple views but changing them
    // creates no resizeEvent, so resize when another view is shown.
    m_gameScene->resizeScene(width(), height());
}

void GameView::resizeEvent(QResizeEvent *event)
{
    m_gameScene->resizeScene(event->size().width(), event->size().height());
}

} // End of namespace Kigo

#include "moc_gameview.cpp"
