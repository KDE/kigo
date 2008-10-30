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

#include "gameview.h"
#include "gamescene.h"

#include <KDebug>

#include <QResizeEvent>

namespace KGo {

GameView::GameView(GameScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_gameScene(scene)
    , m_renderInactive(false)
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
    if (cursorPixmap.isNull())
        unsetCursor();
    else
        setCursor(QCursor(cursorPixmap));
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

void GameView::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (m_renderInactive) {
        // Visually show the user that the current view is inactive by rendering
        // a semi-transparent grey rectangle on top of the game scene.
        painter->setBrush(QBrush(QColor(70, 70, 70, 80), Qt::Dense4Pattern));
        painter->drawRect(rect);
    }
}

} // End of namespace KGo

#include "moc_gameview.cpp"
