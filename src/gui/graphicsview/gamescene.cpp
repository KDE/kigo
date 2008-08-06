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
#include "themerenderer.h"
#include "preferences.h"
#include "game/goengine.h"

#include <KLocalizedString>
#include <KDebug>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QPainter>

namespace KGo {

GameScene::GameScene()
    : m_engine(new GoEngine())
    , m_showLabels(Preferences::showBoardLabels())
    , m_boardSize(Preferences::boardSize())
{
    connect(m_engine, SIGNAL(boardChanged()), this, SLOT(updateBoard()));
    connect(m_engine, SIGNAL(boardSizeChanged(int)), this, SLOT(changeBoardSize(int)));
}

GoEngine * const GameScene::engine() const
{
    return m_engine;
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
    //TODO: Get hint from Engine and display it for some seconds
    emit statusMessage(i18n("The computer assumes this is the best move..."));
    update();
}

void GameScene::resizeScene(int width, int height)
{
    setSceneRect(0, 0, width, height);

    int size = qMin(width, height) - 10; // Add 10 pixel padding around the board
    m_boardRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_cellSize = m_boardRect.width() / (m_boardSize + 1);

    size = static_cast<int>(m_cellSize * (m_boardSize - 1));
    m_gridRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_mouseRect = m_gridRect.adjusted(-m_cellSize / 8, - m_cellSize / 8, m_cellSize / 8,   m_cellSize / 8);

    m_stonePixmapSize = QSize(static_cast<int>(m_cellSize), static_cast<int>(m_cellSize));
    updateBoard(); // Resize means redraw of board items (stones)
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPixmap map;
    if (m_mouseRect.contains(event->scenePos())) {
        if (m_engine->currentPlayer() == GoEngine::WhitePlayer)
            map = ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, m_stonePixmapSize);
        else if (m_engine->currentPlayer() == GoEngine::BlackPlayer)
            map = ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStone, m_stonePixmapSize);
    }
    emit cursorPixmapChanged(map);
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_mouseRect.contains(event->scenePos())) {
        int row = static_cast<int>((event->scenePos().x() - m_mouseRect.x()) / m_cellSize);
        int col = static_cast<int>((event->scenePos().y() - m_mouseRect.y()) / m_cellSize);
        if (row < 0 || row >= m_boardSize || col < 0 || col >= m_boardSize)
            return;

        // Convert to Go board coordinates and try to play the move
        GoEngine::Stone move('A' + row, m_boardSize - col);
        if (m_engine->playMove(move))
            emit statusMessage(i18n("Made move at %1", move.toString()));
        else
            emit statusMessage(i18n("Making a move at %1 is not allowed!", move.toString()));
    }
}

void GameScene::updateBoard()
{
    QGraphicsPixmapItem *item;
    int halfCellSize = m_cellSize / 2;

    kDebug() << "Update board";
    clear(); // Remove all graphics items (stones) from the scene

    foreach (const GoEngine::Stone &stone, m_engine->listStones(GoEngine::BlackPlayer)) {
        item = addPixmap(ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStone, m_stonePixmapSize));
        item->setPos(QPointF(m_gridRect.x() + (stone.x() - 'A') * m_cellSize - halfCellSize,
                             m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfCellSize));
    }
    foreach (const GoEngine::Stone &stone, m_engine->listStones(GoEngine::WhitePlayer)) {
        item = addPixmap(ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, m_stonePixmapSize));
        item->setPos(QPointF(m_gridRect.x() + (stone.x() - 'A') * m_cellSize - halfCellSize,
                             m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfCellSize));
    }
}

void GameScene::changeBoardSize(int size)
{
    m_boardSize = size;
    resizeScene(width(), height());
    invalidate(m_boardRect, QGraphicsScene::BackgroundLayer);
}

void GameScene::drawBackground(QPainter *painter, const QRectF &)
{
    ThemeRenderer::instance()->renderElement(ThemeRenderer::SceneBackground, painter, sceneRect());
    ThemeRenderer::instance()->renderElement(ThemeRenderer::BoardBackground, painter, m_boardRect);

    for (int i = 0; i < m_boardSize; i++) {
        qreal offset = i * m_cellSize;
        painter->setPen(QPen(QColor(20, 30, 20), m_cellSize / 15));
        painter->drawLine(QPointF(m_gridRect.left(),  m_gridRect.top() + offset),
                          QPointF(m_gridRect.right(), m_gridRect.top() + offset));
        painter->drawLine(QPointF(m_gridRect.left() + offset, m_gridRect.top()),
                          QPointF(m_gridRect.left() + offset, m_gridRect.bottom()));

        if (m_showLabels) {
            QChar c('A' + i);
            QString n = QString::number(m_boardSize - i);
            QFont f = painter->font();
            f.setPointSizeF(m_cellSize / 4);
            painter->setFont(f);
            QFontMetrics fm = painter->fontMetrics();

            // Draw vertical numbers for board coordinates
            qreal yVert = m_gridRect.top() + offset + fm.descent();
            painter->drawText(QPointF(m_gridRect.left() - m_cellSize + 2, yVert), n);
            painter->drawText(QPointF(m_gridRect.right() + m_cellSize - fm.width(n) - 3, yVert), n);

            // Draw horizontal characters for board coordinates
            qreal xHor = m_gridRect.left() + offset - fm.width(c) / 2;
            painter->drawText(QPointF(xHor, m_gridRect.top() - m_cellSize + fm.ascent() + 2), QString(c));
            painter->drawText(QPointF(xHor, m_gridRect.bottom() + m_cellSize - 3), QString(c));
        }
    }
}

} // End of namespace KGo

#include "moc_gamescene.cpp"
