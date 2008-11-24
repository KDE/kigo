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

#include "gamescene.h"
#include "themerenderer.h"
#include "preferences.h"

#include <KLocalizedString>
#include <KDebug>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QPainter>

namespace Kigo {

GameScene::GameScene()
    : m_engine(new GoEngine)
    , m_showLabels(Preferences::showBoardLabels())
    , m_showHint(false)
    , m_showMoveNumbers(Preferences::showMoveNumbers())
    , m_boardSize(Preferences::boardSize())
    , m_placementMarkerItem(0)
{
    connect(m_engine, SIGNAL(boardChanged()), this, SLOT(updateStoneItems()));
    connect(m_engine, SIGNAL(boardSizeChanged(int)), this, SLOT(changeBoardSize(int)));
    connect(m_engine, SIGNAL(currentPlayerChanged(GoEngine::PlayerColor)), this, SLOT(hideHint()));
    connect(m_engine, SIGNAL(consecutivePassMovesPlayed(int)), this, SLOT(showPassMessage(int)));
    connect(m_engine, SIGNAL(playerResigned(GoEngine::PlayerColor)), this, SLOT(showResignMessage(GoEngine::PlayerColor)));

    m_gamePopup.setMessageTimeout(3000);
    m_gamePopup.setHideOnMouseClick(true);
    addItem(&m_gamePopup);                  // TODO: Fix initial placement issue
}

GameScene::~GameScene()
{
    delete m_engine;
}

void GameScene::resizeScene(int width, int height)
{
    setSceneRect(0, 0, width, height);

    int size = qMin(width, height) - 10;    // Add 10 pixel padding around the board
    m_boardRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_cellSize = m_boardRect.width() / (m_boardSize + 1);

    size = static_cast<int>(m_cellSize * (m_boardSize - 1));
    m_gridRect.setRect(width / 2 - size / 2, height / 2 - size / 2, size, size);
    m_mouseRect = m_gridRect.adjusted(-m_cellSize / 2, - m_cellSize / 2, m_cellSize / 2,   m_cellSize / 2);

    m_stonePixmapSize = QSize(static_cast<int>(m_cellSize), static_cast<int>(m_cellSize));
    updateStoneItems();                     // Resize means redraw of board items (stones)
    updateHintItems();                      // and move hint items

    if (m_placementMarkerItem)              // Set the mouse/stone postion placementmarker
        removeItem(m_placementMarkerItem);
    m_placementMarkerPixmapSize = QSize(static_cast<int>(m_cellSize / 4), static_cast<int>(m_cellSize / 4));
    m_placementMarkerItem = addPixmap(ThemeRenderer::instance()->renderElement(ThemeRenderer::PlacementMarker, m_placementMarkerPixmapSize));
    m_placementMarkerItem->setVisible(false);
    m_placementMarkerItem->setZValue(1);
}

void GameScene::showLabels(bool show)
{
    m_showLabels = show;
    invalidate(m_boardRect, QGraphicsScene::BackgroundLayer);
}

void GameScene::showHint(bool show)
{
    m_showHint = show;
    updateHintItems();
}

void GameScene::showMoveNumbers(bool show)
{
    m_showMoveNumbers = show;
    updateStoneItems();
}

void GameScene::showPopupMessage(const QString &message, int msecs)
{
    m_gamePopup.setMessageTimeout(msecs);
    if (message.isEmpty())
        m_gamePopup.forceHide();            // Now message hides the last one
    else
        m_gamePopup.showMessage(message, KGamePopupItem::BottomLeft, KGamePopupItem::ReplacePrevious);
}

void GameScene::updateStoneItems()
{
    QGraphicsPixmapItem *item;
    int halfCellSize = m_cellSize / 2;

    foreach (item, m_stoneItems)            // Clear all standard Go stone graphics pixmap items
        removeItem(item);
    m_stoneItems.clear();

    foreach (const GoEngine::Stone &stone, m_engine->listStones(GoEngine::BlackPlayer)) {
        item = addPixmap(ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStone, m_stonePixmapSize));
        item->setZValue(2);
        int xOff = stone.x() >= 'I' ? stone.x() - 'A' - 1 : stone.x() - 'A';
        item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize,
                             m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfCellSize));
        m_stoneItems.append(item);
    }
    foreach (const GoEngine::Stone &stone, m_engine->listStones(GoEngine::WhitePlayer)) {
        item = addPixmap(ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStone, m_stonePixmapSize));
        item->setZValue(2);
        int xOff = stone.x() >= 'I' ? stone.x() - 'A' - 1 : stone.x() - 'A';
        item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize,
                             m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfCellSize));
        m_stoneItems.append(item);
    }

    if (m_showMoveNumbers) {
        QList<QPair<GoEngine::Stone, GoEngine::PlayerColor> > numbers = m_engine->moveHistory();
        for (int i = 0; i < numbers.size(); i++) {
            int xOff = numbers[i].first.x() >= 'I' ? numbers[i].first.x() - 'A' - 1 : numbers[i].first.x() - 'A';
            QPointF pos = QPointF(m_gridRect.x() + xOff * m_cellSize,
                                  m_gridRect.y() + (m_boardSize - numbers[i].first.y()) * m_cellSize);

            QGraphicsPixmapItem *item = static_cast<QGraphicsPixmapItem *>(itemAt(pos));
            if (item) {
                // We found an item in the scene that is in our move numbers, so we paint it's move number
                // on top of the item and that's all.
                //TODO: Check for existing move number to do special treatment
                QPixmap pixmap = item->pixmap();
                QPainter painter(&pixmap);
                if (numbers[i].second == GoEngine::WhitePlayer)
                    painter.setPen(Qt::black);
                else if (numbers[i].second == GoEngine::BlackPlayer)
                    painter.setPen(Qt::white);
                QFont f = painter.font();
                f.setPointSizeF(m_cellSize / 4);
                painter.setFont(f);
                painter.drawText(pixmap.rect(), Qt::AlignCenter, QString::number(i));
                item->setPixmap(pixmap);
            }
        }
    }
}

void GameScene::updateHintItems()
{
    QGraphicsPixmapItem *item;

    // Old hint is invalid, remove it first
    foreach (QGraphicsPixmapItem *item, m_hintItems)
        removeItem(item);
    m_hintItems.clear();

    if (m_showHint) {
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
            item->setZValue(4);
            int xOff = entry.first.x() >= 'I' ? entry.first.x() - 'A' - 1 : entry.first.x() - 'A';
            item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize,
                                 m_gridRect.y() + (m_boardSize - entry.first.y()) * m_cellSize - halfCellSize));
            m_hintItems.append(item);
        }
        showPopupMessage(i18n("The computer recommends these moves ..."));
    }
}

void GameScene::changeBoardSize(int size)
{
    m_boardSize = size;
    resizeScene(width(), height());
    invalidate(m_boardRect, QGraphicsScene::BackgroundLayer);
}

void GameScene::showPassMessage(int)
{
    m_gamePopup.showMessage(i18n("Both players passed, decide if you want to finish the game and count scores..."), KGamePopupItem::TopLeft, KGamePopupItem::ReplacePrevious);
}

void GameScene::showResignMessage(GoEngine::PlayerColor player)
{
    QString looser;
    player == GoEngine::WhitePlayer ? looser = "White" : looser = "Black";
    m_gamePopup.showMessage(i18n("%1 has resigned and you have won the game, please count your scores...", looser), KGamePopupItem::TopLeft, KGamePopupItem::ReplacePrevious);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPixmap map;
    if (m_mouseRect.contains(event->scenePos())) {
        int row = static_cast<int>((event->scenePos().x() - m_mouseRect.x()) / m_cellSize);
        int col = static_cast<int>((event->scenePos().y() - m_mouseRect.y()) / m_cellSize);

        int x = m_mouseRect.x() + row * m_cellSize + m_cellSize/2 - m_placementMarkerPixmapSize.width()/2;
        int y = m_mouseRect.y() + col * m_cellSize + m_cellSize/2 - m_placementMarkerPixmapSize.height()/2;

        m_placementMarkerItem->setVisible(true);
        m_placementMarkerItem->setPos(x, y);

        if (m_engine->currentPlayer() == GoEngine::WhitePlayer)
            map = ThemeRenderer::instance()->renderElement(ThemeRenderer::WhiteStoneTransparent, m_stonePixmapSize);
        else if (m_engine->currentPlayer() == GoEngine::BlackPlayer)
            map = ThemeRenderer::instance()->renderElement(ThemeRenderer::BlackStoneTransparent, m_stonePixmapSize);
    } else {
        m_placementMarkerItem->setVisible(false);
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

        // Convert to Go board coordinates and try to play the move. GnuGo coordinates don't use the 'I'
        // column, if the row is bigger than 'I', we have to add 1 to jump over that.
        if (row >= 8)
            row += 1;
        GoEngine::Stone move('A' + row, m_boardSize - col);

        if (m_engine->playMove(move))
            showPopupMessage(i18n("Made move at %1", move.toString()));
        else
            showPopupMessage(i18n("Making a move at %1 is not allowed!", move.toString()));
    }
}

void GameScene::drawBackground(QPainter *painter, const QRectF &)
{
    ThemeRenderer::instance()->renderElement(ThemeRenderer::SceneBackground, painter, sceneRect());
    ThemeRenderer::instance()->renderElement(ThemeRenderer::BoardBackground, painter, m_boardRect);

    for (int i = 0; i < m_boardSize; i++) {
        qreal offset = i * m_cellSize;
        painter->setPen(QPen(QColor(20, 30, 20), m_cellSize / 16));
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

} // End of namespace Kigo

#include "moc_gamescene.cpp"
