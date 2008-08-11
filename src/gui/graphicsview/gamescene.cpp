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
#include <QTimer>

namespace KGo {

GameScene::GameScene()
    : m_engine(new GoEngine())
    , m_showMoveHistory(false)
    , m_showLabels(Preferences::showBoardLabels())
    , m_boardSize(Preferences::boardSize())
{
    connect(m_engine, SIGNAL(boardChanged()), this, SLOT(updateStoneItems()));
    connect(m_engine, SIGNAL(boardSizeChanged(int)), this, SLOT(changeBoardSize(int)));
}

GoEngine * const GameScene::engine() const
{
    return m_engine;
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
    showHint(false);
    updateStoneItems();             // Resize means redraw of board items (stones)
    updateMoveHistoryItems();
}

void GameScene::showMoveHistory(bool show)
{
    kDebug() << "Show move history:" << show;
    m_showMoveHistory = show;
    updateMoveHistoryItems();
}

void GameScene::showLabels(bool show)
{
    m_showLabels = show;
    invalidate(m_boardRect, QGraphicsScene::BackgroundLayer);
}

void GameScene::showHint(bool show)
{
    kDebug() << "Show hint" << show;
    QGraphicsPixmapItem *item;

    if (show) {
        int halfCellSize = m_cellSize / 2;

        GoEngine::PlayerColor currentPlayer = m_engine->currentPlayer();

        QPair<GoEngine::Stone, float> entry;
        foreach (entry, m_engine->topMoves(currentPlayer)) {
            QPixmap stonePixmap;
            if (currentPlayer == GoEngine::WhitePlayer)
                stonePixmap = ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStoneTransparent, m_stonePixmapSize);
            else if (currentPlayer == GoEngine::BlackPlayer)
                stonePixmap = ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStoneTransparent, m_stonePixmapSize);

            QPainter painter(&stonePixmap);
            if (currentPlayer == GoEngine::WhitePlayer)
                painter.setPen(Qt::black);
            else if (currentPlayer == GoEngine::BlackPlayer)
                painter.setPen(Qt::white);
            QFont f = painter.font();
            f.setPointSizeF(m_cellSize / 4);
            painter.setFont(f);
            painter.drawText(stonePixmap.rect(), Qt::AlignCenter, QString::number(entry.second));
            painter.end();

            item = addPixmap(stonePixmap);
            int xOff = entry.first.x() >= 'I' ? entry.first.x() - 'A' - 1 : entry.first.x() - 'A';
            item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize,
                                 m_gridRect.y() + (m_boardSize - entry.first.y()) * m_cellSize - halfCellSize));
            m_hintItems.append(item);
        }

        emit statusMessage(i18n("The computer assumes these are the best moves..."));
        QTimer::singleShot(Preferences::hintVisibleTime() * 1000, this, SLOT(showHint(false)));
    } else {
        foreach (item, m_hintItems)
            removeItem(item);
        m_hintItems.clear();
    }
}

void GameScene::updateStoneItems()
{
    kDebug() << "Update board";
    QGraphicsPixmapItem *item;
    int halfCellSize = m_cellSize / 2;

    // Clear all standard Go stone graphics pixmap items
    foreach (item, m_stoneItems)
        removeItem(item);
    m_stoneItems.clear();

    foreach (const GoEngine::Stone &stone, m_engine->listStones(GoEngine::BlackPlayer)) {
        item = addPixmap(ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStone, m_stonePixmapSize));
        int xOff = stone.x() >= 'I' ? stone.x() - 'A' - 1 : stone.x() - 'A';
        item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize,
                             m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfCellSize));
        m_stoneItems.append(item);
    }
    foreach (const GoEngine::Stone &stone, m_engine->listStones(GoEngine::WhitePlayer)) {
        item = addPixmap(ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, m_stonePixmapSize));
        int xOff = stone.x() >= 'I' ? stone.x() - 'A' - 1 : stone.x() - 'A';
        item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize,
                             m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfCellSize));
        m_stoneItems.append(item);
    }
}

void GameScene::updateMoveHistoryItems()
{
    kDebug() << "Update move history";
    QGraphicsTextItem *item;
    int halfCellSize = m_cellSize / 2;

    foreach (item, m_moveHistoryItems)
        removeItem(item);
    m_moveHistoryItems.clear();

    if (m_showMoveHistory) {
        QList<QPair<GoEngine::Stone, GoEngine::PlayerColor> > history = m_engine->moveHistory();
        for (int i = history.size(); i > 0; i--) {
            item = addText(QString::number(i));
            int xOff = history[i].first.x() >= 'I' ? history[i].first.x() - 'A' - 1 : history[i].first.x() - 'A';
            item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize,
                                 m_gridRect.y() + (m_boardSize - history[i].first.y()) * m_cellSize - halfCellSize));
            m_moveHistoryItems.append(item);
        }
    }
}

void GameScene::changeBoardSize(int size)
{
    m_boardSize = size;
    resizeScene(width(), height());
    invalidate(m_boardRect, QGraphicsScene::BackgroundLayer);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPixmap map;
    if (m_mouseRect.contains(event->scenePos())) {
        if (m_engine->currentPlayer() == GoEngine::WhitePlayer)
            map = ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStoneTransparent, m_stonePixmapSize);
        else if (m_engine->currentPlayer() == GoEngine::BlackPlayer)
            map = ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStoneTransparent, m_stonePixmapSize);
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

        showHint(false);  // A click invalidates the move hint

        // Convert to Go board coordinates and try to play the move. GnuGo coordinates don't use the 'I'
        // column, if the row is bigger than 'I', we have to add 1 to jump over that (strange, eh?).
        if (row >= 8)
            row += 1;
        GoEngine::Stone move('A' + row, m_boardSize - col);

        if (m_engine->playMove(move))
            emit statusMessage(i18n("Made move at %1", move.toString()));
        else
            emit statusMessage(i18n("Making a move at %1 is not allowed!", move.toString()));
    }
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
            // GnuGo does not use the 'I' column (for whatever strange reason), we have to skip that too
            if (i >= 8)
                c = QChar('A' + i + 1);

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
