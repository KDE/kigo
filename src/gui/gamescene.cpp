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
 * @file This file is part of KGO and implements the class GameScene,
 *       which displays the go game.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#include "gamescene.h"
#include "game/gtp.h"
#include "themerenderer.h"

#include <QGraphicsPixmapItem>
#include <QPainter>

#include <KDebug>

namespace KGo {

GameScene::GameScene()
    : m_gtp(new Gtp())
    , m_showLabels(false)
    , m_cursorItem(0)
{
    Q_ASSERT(m_gtp);

    connect(m_gtp, SIGNAL(boardChanged()), this, SLOT(updateBoard()));
}

void GameScene::resizeScene(int width, int height)
{
    setSceneRect(0, 0, width, height);

    int size = qMin(width, height);
    m_boardRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);

    // Determine size of board cell element based on width (same as height, board is a quad)
    int defWidth = ThemeRenderer::instance()->elementSize(ThemeRenderer::BoardBackground).width();
    qreal scale = (qreal)size / defWidth;
    if (scale <= 0)
        return;

    // Make sure the cell element has the correct size (correspoding to current board size)
    m_currentCellSize = defWidth * scale / (m_gtp->boardSize() + 3);
}

Gtp * const GameScene::gtp() const
{
    return m_gtp;
}

void GameScene::updateBoard()
{
    //TODO: set komi, board size ...

    update();
}

void GameScene::showMoveHistory(bool show)
{

}

void GameScene::hint()
{

}

void GameScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    ThemeRenderer::instance()->renderElement(ThemeRenderer::SceneBackground, painter, sceneRect());
    ThemeRenderer::instance()->renderElement(ThemeRenderer::BoardBackground, painter, m_boardRect);
    if (m_showLabels)
        ThemeRenderer::instance()->renderElement(ThemeRenderer::BoardLabels, painter, m_boardRect);

    // Draw the board cells
    /*QPointF upLeftPoint(m_boardRect.x() + m_currentCellSize * 1.5,
                        m_boardRect.y() + m_currentCellSize * 1.5);
    QPointF upRightPoint(m_boardRect.x() + m_boardRect.width() - m_currentCellSize * 1.5,
                        m_boardRect.y() + m_currentCellSize * 1.5);
    QPointF downLeftPoint(upLeftPoint.x(),
                        upLeftPoint.y() + m_boardRect.height() - m_currentCellSize * 3);

    for (int i = 0; i < m_gtp->boardSize() - 1; i++) {
        // Draw horizontal line
        painter->drawLine(QPointF(upLeftPoint.x(), upLeftPoint.y() + i * m_currentCellSize),
                        QPointF(upRightPoint.x(), upRightPoint.y() + i * m_currentCellSize));

        // Draw vertical line
        painter->drawLine(QPointF(upLeftPoint.x() + i * m_currentCellSize, upLeftPoint.y()),
                        QPointF(downLeftPoint.x() + i * m_currentCellSize, downLeftPoint.y()));
    }*/
}

} // End of namespace KGo

#include "moc_gamescene.cpp"
