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
    , m_boardSize(0)
{
    connect(m_engine, SIGNAL(boardChanged()), this, SLOT(updateBoard()));
    connect(m_engine, SIGNAL(boardSizeChanged(int)), this, SLOT(boardSizeChanged(int)));
}

void GameScene::resizeScene(int width, int height)
{
    setSceneRect(0, 0, width, height);

    int size = qMin(width, height) - 10; // Add 10 pixel margin around the board

    m_boardRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_boardGridSize = m_boardRect.width() / (m_boardSize + 1);

    size = static_cast<int>(m_boardGridSize * (m_boardSize - 1));
    m_boardGridRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);

    update();
}

GoEngine * const GameScene::engine() const
{
    return m_engine;
}

void GameScene::updateBoard()
{
    //TODO: set komi, board size ...
    kDebug() << "Update board";

    update();
}

void GameScene::boardSizeChanged(int size)
{
    m_boardSize = size;
}

void GameScene::showMoveHistory(bool show)
{
    kDebug() << "Show move history:" << show;
    //TODO: Get move history from engine and display them with half-transparent go stone pixmaps
    update();
}

void GameScene::showLabels(bool show)
{
    kDebug() << "Show labels:" << show;
    m_showLabels = show;
    update();
}

void GameScene::hint()
{
    update();
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_boardRect.contains(event->scenePos())) {
        QSize size(static_cast<int>(m_boardGridSize), static_cast<int>(m_boardGridSize));
        //TODO: Get correct pixmap based on current active player
        QPixmap map = ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, size);
        emit changeCursor(map);
    } else
        emit changeCursor(QPixmap());
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_boardRect.contains(event->scenePos())) {

        int row = static_cast<int>((event->scenePos().x() - m_boardGridRect.x() + m_boardGridSize / 2) / m_boardGridSize);
        int col = static_cast<int>((event->scenePos().y() - m_boardGridRect.y() + m_boardGridSize / 2) / m_boardGridSize);

        kDebug() << "Row" << row << "col" << col;
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

    //TODO: Cache all that into pixmap to speed up rendering, maybe move to ThemeRenderer but this
    //      would add unnecessary GoEngine dependency on ThemeRenderer.
    for (int i = 0; i < m_boardSize; i++) {
        qreal offset = i * m_boardGridSize;
        painter->save();
        painter->setPen(QPen(QColor(20, 30, 20), m_boardGridSize / 15));
        painter->drawLine(QPointF(m_boardGridRect.left(),  m_boardGridRect.top() + offset),
                          QPointF(m_boardGridRect.right(), m_boardGridRect.top() + offset));
        painter->drawLine(QPointF(m_boardGridRect.left() + offset, m_boardGridRect.top()),
                          QPointF(m_boardGridRect.left() + offset, m_boardGridRect.bottom()));

        if (m_showLabels) {
            QChar character('A' + i);
            QString number = QString::number(m_boardSize - i);
            QFont f = painter->font();
            f.setPointSizeF(m_boardGridSize / 4);
            painter->setFont(f);
            QFontMetrics fm = painter->fontMetrics();

            // Draw vertical numbers for board coordinates
            qreal yVert = m_boardGridRect.top() + offset + fm.descent();
            painter->drawText(QPointF(m_boardGridRect.left() - m_boardGridSize + 2, yVert), number);
            painter->drawText(QPointF(m_boardGridRect.right() + m_boardGridSize - fm.width(number) - 3, yVert), number);

            // Draw horizontal characters for board coordinates
            qreal xHor = m_boardGridRect.left() + offset - fm.width(character) / 2;
            painter->drawText(QPointF(xHor, m_boardGridRect.top() - m_boardGridSize + fm.ascent() + 2), QString(character));
            painter->drawText(QPointF(xHor, m_boardGridRect.bottom() + m_boardGridSize - 3), QString(character));
        }
        painter->restore();
    }
}

} // End of namespace KGo

#include "moc_gamescene.cpp"
