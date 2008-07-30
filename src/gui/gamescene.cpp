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
#include "game/goengine.h"
#include "themerenderer.h"
#include "preferences.h"

#include <KDebug>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QPainter>

namespace KGo {

GameScene::GameScene()
    : m_engine(new GoEngine())
    , m_showLabels(Preferences::showBoardLabels())
    , m_boardSize(19)
{
    connect(m_engine, SIGNAL(boardChanged()), this, SLOT(updateBoard()));
    connect(m_engine, SIGNAL(boardSizeChanged(int)), this, SLOT(boardSizeChanged(int)));
}

void GameScene::resizeScene(int width, int height)
{
    setSceneRect(0, 0, width, height);

    int size = qMin(width, height) - 10; // Add 10 pixel margin around the board
    m_boardRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_boardGridCellSize = m_boardRect.width() / (m_boardSize + 1);

    size = static_cast<int>(m_boardGridCellSize * (m_boardSize - 1));
    m_boardGridRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_boardMouseRect = m_boardGridRect.adjusted(-m_boardGridCellSize / 8, - m_boardGridCellSize / 8, m_boardGridCellSize / 8, m_boardGridCellSize / 8);
}

GoEngine * const GameScene::engine() const
{
    return m_engine;
}

void GameScene::updateBoard()
{
    kDebug() << "Update board";
    //TODO: Show stones at positions

    //QList<GoEngine::Stone> whiteStones = m_engine->listStones(GoEngine::WhitePlayer);
    //QList<GoEngine::Stone> blackStones = m_engine->listStones(GoEngine::BlackPlayer);

    invalidate(m_boardRect, QGraphicsScene::ItemLayer);
}

void GameScene::boardSizeChanged(int size)
{
    m_boardSize = size;
    resizeScene(width(), height());
    invalidate(m_boardRect, QGraphicsScene::BackgroundLayer);
}

void GameScene::showMoveHistory(bool show)
{
    kDebug() << "Show move history:" << show;
    //TODO: Get move history from engine and display them with half-transparent go stone pixmaps
    invalidate(m_boardRect, QGraphicsScene::ItemLayer);
}

void GameScene::showLabels(bool show)
{
    m_showLabels = show;
    invalidate(m_boardRect, QGraphicsScene::BackgroundLayer);
}

void GameScene::showHint()
{
    //TODO: Get hint from Engine and display it
    update();
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_boardMouseRect.contains(event->scenePos())) {
        QSize size(static_cast<int>(m_boardGridCellSize), static_cast<int>(m_boardGridCellSize));
        //TODO: Get correct pixmap based on current active player
        QPixmap map = ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, size);
        emit cursorChanged(map);
    } else
        emit cursorChanged(QPixmap());
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_boardMouseRect.contains(event->scenePos())) {
        int row = static_cast<int>((event->scenePos().x() - m_boardMouseRect.x()) / m_boardGridCellSize);
        int col = static_cast<int>((event->scenePos().y() - m_boardMouseRect.y()) / m_boardGridCellSize);
        if (row < 0 || row >= m_boardSize || col < 0 || col >= m_boardSize)
            return;

        // Convert to Go board coordinates
        GoEngine::Stone move('A' + row, m_boardSize - col);

        //TODO: Check if field is occupied, otherwise make move
        //if (m_engine->isLegal(GoEngine::WhitePlayer, move)) {

            kDebug() << "Make move at " << move.toString();
        //}
    }
}

void GameScene::drawBackground(QPainter *painter, const QRectF &)
{
    ThemeRenderer::instance()->renderElement(ThemeRenderer::SceneBackground, painter, sceneRect());
    ThemeRenderer::instance()->renderElement(ThemeRenderer::BoardBackground, painter, m_boardRect);

    for (int i = 0; i < m_boardSize; i++) {
        qreal offset = i * m_boardGridCellSize;
        painter->setPen(QPen(QColor(20, 30, 20), m_boardGridCellSize / 15));
        painter->drawLine(QPointF(m_boardGridRect.left(),  m_boardGridRect.top() + offset),
                          QPointF(m_boardGridRect.right(), m_boardGridRect.top() + offset));
        painter->drawLine(QPointF(m_boardGridRect.left() + offset, m_boardGridRect.top()),
                          QPointF(m_boardGridRect.left() + offset, m_boardGridRect.bottom()));

        if (m_showLabels) {
            QChar c('A' + i);
            QString n = QString::number(m_boardSize - i);
            QFont f = painter->font();
            f.setPointSizeF(m_boardGridCellSize / 4);
            painter->setFont(f);
            QFontMetrics fm = painter->fontMetrics();

            // Draw vertical numbers for board coordinates
            qreal yVert = m_boardGridRect.top() + offset + fm.descent();
            painter->drawText(QPointF(m_boardGridRect.left() - m_boardGridCellSize + 2, yVert), n);
            painter->drawText(QPointF(m_boardGridRect.right() + m_boardGridCellSize - fm.width(n) - 3, yVert), n);

            // Draw horizontal characters for board coordinates
            qreal xHor = m_boardGridRect.left() + offset - fm.width(c) / 2;
            painter->drawText(QPointF(xHor, m_boardGridRect.top() - m_boardGridCellSize + fm.ascent() + 2), QString(c));
            painter->drawText(QPointF(xHor, m_boardGridRect.bottom() + m_boardGridCellSize - 3), QString(c));
        }
    }
}

} // End of namespace KGo

#include "moc_gamescene.cpp"
