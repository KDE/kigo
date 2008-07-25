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

#include <QGraphicsPixmapItem>
#include <QPainter>

#include <KDebug>

namespace KGo {

GameScene::GameScene()
    : m_engine(new GoEngine())
    , m_showLabels(false)
{
    connect(m_engine, SIGNAL(boardChanged()), this, SLOT(updateBoard()));
}

void GameScene::resizeScene(int width, int height)
{
    setSceneRect(0, 0, width, height);

    int size = qMin(width, height);
    m_boardRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);

    size = static_cast<int>(size * 0.9);
    m_boardGridRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_boardGridSize = m_boardGridRect.width() / m_engine->boardSize();
}

GoEngine * const GameScene::engine() const
{
    return m_engine;
}

void GameScene::updateBoard()
{
    //TODO: set komi, board size ...

    update();
}

void GameScene::showMoveHistory(bool show)
{
    kDebug() << "Show move history:" << show;
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

void GameScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    ThemeRenderer::instance()->renderElement(ThemeRenderer::SceneBackground, painter, sceneRect());
    ThemeRenderer::instance()->renderElement(ThemeRenderer::BoardBackground, painter, m_boardRect);

    //FIXME: Rentrancy problem with m_engine->waitProcess, use this for now:
    for (int i = 0; i < 20/*m_engine->boardSize()*/; i++) {
        painter->save();
        painter->setPen(QPen(QColor(20, 30, 20, 200), static_cast<int>(m_boardGridSize / 10)));
        painter->drawLine(QPointF(m_boardGridRect.left(),  m_boardGridRect.top() + i * m_boardGridSize),
                          QPointF(m_boardGridRect.right(), m_boardGridRect.top() + i * m_boardGridSize));
        painter->drawLine(QPointF(m_boardGridRect.left() + i * m_boardGridSize, m_boardGridRect.top()),
                          QPointF(m_boardGridRect.left() + i * m_boardGridSize, m_boardGridRect.bottom()));

        if (m_showLabels) {
            QChar character('A' + i);
            QString number = QString::number(19 - i);
            QFontMetrics fm = painter->fontMetrics();

            // Draw vertical numbers for board coordinates
            painter->drawText(QPointF(m_boardGridRect.left() - m_boardGridSize,
                                      m_boardGridRect.top() + i * m_boardGridSize + fm.descent()), number);
            painter->drawText(QPointF(m_boardGridRect.right() + m_boardGridSize - fm.width(number),
                                      m_boardGridRect.top() + i * m_boardGridSize + fm.descent()), number);

            // Draw horizontal characters for board coordinates
            painter->drawText(QPointF(m_boardGridRect.left() + i * m_boardGridSize - fm.width(character) / 2,
                                      m_boardGridRect.top() - m_boardGridSize + fm.ascent()), QString(character));
            painter->drawText(QPointF(m_boardGridRect.left() + i * m_boardGridSize - fm.width(character) / 2,
                                      m_boardGridRect.bottom() + m_boardGridSize), QString(character));
        }
        painter->restore();
    }
}

} // End of namespace KGo

#include "moc_gamescene.cpp"
