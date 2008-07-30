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
 * @file This file is part of KGO and implements the class GameView,
 *       which implements the view for a go game scene.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#include "gameview.h"
#include "gamescene.h"

#include <KDebug>

#include <QResizeEvent>

namespace KGo {

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

    connect(m_gameScene, SIGNAL(cursorChanged(QPixmap)), this, SLOT(changeCursor(QPixmap)));
}

void GameView::drawForeground(QPainter *painter, const QRectF &rect)
{
    // Visually show the user that the current view is inactive by rendering
    // a semi-transparent grey rectangle on top of the game scene.
    if (!isInteractive()) {
        painter->save();
        painter->setBrush(QBrush(QColor(70, 70, 70, 100), Qt::Dense4Pattern));
        painter->drawRect(rect);
        painter->restore();
    }
}

void GameView::resizeEvent(QResizeEvent *event)
{
    m_gameScene->resizeScene(event->size().width(), event->size().height());
}

void GameView::showEvent(QShowEvent *)
{
    // Make sure that the game scene has the correct size according to the current view
    // This is necessary because one scene is shared by multiple views but changing them
    // creates no resizeEvent, so resize when another view is shown.
    m_gameScene->resizeScene(width(), height());
}

void GameView::changeCursor(QPixmap cursorPixmap)
{
    if (cursorPixmap.isNull())
        unsetCursor();
    else
        setCursor(QCursor(cursorPixmap));
}

} // End of namespace KGo

#include "moc_gameview.cpp"
