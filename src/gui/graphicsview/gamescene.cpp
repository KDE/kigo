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
    m_boardGridCellSize = m_boardRect.width() / (m_boardSize + 1);

    size = static_cast<int>(m_boardGridCellSize * (m_boardSize - 1));
    m_boardGridRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_boardMouseRect = m_boardGridRect.adjusted(-m_boardGridCellSize / 8, - m_boardGridCellSize / 8,
                                                 m_boardGridCellSize / 8,   m_boardGridCellSize / 8);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPixmap map;
    if (m_boardMouseRect.contains(event->scenePos())) {
        QSize size(static_cast<int>(m_boardGridCellSize), static_cast<int>(m_boardGridCellSize));
        if (m_engine->currentPlayer() == GoEngine::WhitePlayer)
            map = ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, size);
        else if (m_engine->currentPlayer() == GoEngine::BlackPlayer)
            map = ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStone, size);
    }
    emit cursorPixmapChanged(map);
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

        if (m_engine->isLegal(move)) {
            emit statusMessage(i18n("Make move at %1", move.toString()));
            if (m_engine->playMove(move)) {
                updateBoard();

                // Note: Change cursor pixmap just in case the user does not move the mouse cursor.
                QPixmap map;
                QSize size(static_cast<int>(m_boardGridCellSize), static_cast<int>(m_boardGridCellSize));
                if (m_engine->currentPlayer() == GoEngine::WhitePlayer)
                    map = ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, size);
                else if (m_engine->currentPlayer() == GoEngine::BlackPlayer)
                    map = ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStone, size);
                emit cursorPixmapChanged(map);
            }
        } else {
            emit statusMessage(i18n("Making a move at %1 is not allowed!", move.toString()));
        }
    }
}

void GameScene::updateBoard()
{
    kDebug() << "Update board";

    // Handicap stones are not part of the move history and have to be taken into account
    // only once the game is set up.
    /*if (!m_handicapPlaced && handicapStones > 0) {
        //TODO: Add handicap stones only once, remove them if board is cleared/size changed/...
        //TODO: Keep track of board resize and board whiping to set that flag
    }*/

    // We need to add only the stones that are new since the last updateBoard() call.
    QList<QPair<GoEngine::PlayerColor, GoEngine::Stone> > moveHistory = m_engine->moveHistory();
    int difference = moveHistory.size() - m_stoneItemList.size();
    if (difference > 0) {
        kDebug() << "Add" << difference << "new items to stone item list";
        // We have to add a certain amount of new stones.
        QGraphicsPixmapItem *newItem;


        m_stoneItemList.append(newItem);
    } else if (difference < 0) {
        kDebug() << "Remove" << difference << "items from stone item list";
        // A negative difference means we have to remove a certain amount
        // of stones. This happens for example when the user undo's moves.
        for (int i = 0; i < difference; i++)
            m_stoneItemList.removeLast();
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
