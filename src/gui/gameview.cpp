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
#include "themerenderer.h"

#include <QResizeEvent>

#include <KDebug>

namespace KGo {

GameView::GameView(GameScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_scene(scene)
    , m_whiteStoneCursor(ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, QSize(32, 32)))
    , m_blackStoneCursor(ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStone, QSize(32, 32)))
{
    setCacheMode(QGraphicsView::CacheBackground);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setOptimizationFlags(QGraphicsView::DontClipPainter |
                         QGraphicsView::DontSavePainterState |
                         QGraphicsView::DontAdjustForAntialiasing);
}

void GameView::resizeEvent(QResizeEvent *event)
{
    m_scene->resizeScene(event->size().width(), event->size().height());
}

void GameView::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (!isInteractive()) {
        painter->save();
        painter->setBrush(QBrush(QColor(70, 70, 70, 100), Qt::Dense4Pattern));
        painter->drawRect(rect);
        painter->restore();
    }
}

} // End of namespace KGo

#include "moc_gameview.cpp"
