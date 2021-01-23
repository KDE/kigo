/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gamescene.h"
#include "game/game.h"
#include "preferences.h"
#include "themerenderer.h"

#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

namespace Kigo {

    const int dotPositions9[] = {2,2, 2,6, 6,2, 6,6, 4,4};
    const int dotPositions13[] = {3,3, 3,9, 9,3, 9,9, 6,6};
    const int dotPositions19[] = {3,3, 3,9, 3,15, 9,3, 9,9, 9,15, 15,3, 15,9, 15,15};

GameScene::GameScene(Game *game, QObject *parent)
    : QGraphicsScene(parent)
    , m_game(game)
    , m_gamePopup()
    , m_showLabels(Preferences::showBoardLabels())
    , m_showHint(false)
    , m_hintTimer(new QTimer(this))
    , m_showMoveNumbers(Preferences::showMoveNumbers())
    , m_showPlacementMarker(true)
    , m_showTerritory(false)
    , m_boardRect()
    , m_mouseRect()
    , m_gridRect()
    , m_cellSize(0)
    , m_stonePixmapSize()
    , m_placementMarkerPixmapSize()
    , m_boardSize(Preferences::boardSize())
    , m_placementMarkerItem(nullptr)
    , m_stoneItems()
    , m_hintItems()
    , m_territoryItems()
{
    connect(m_game, &Game::boardChanged, this, &GameScene::updateStoneItems);
    connect(m_game, &Game::boardSizeChanged, this, &GameScene::changeBoardSize);
    connect(m_game, &Game::currentPlayerChanged, this, &GameScene::hideHint);
    connect(ThemeRenderer::self(), &ThemeRenderer::themeChanged, this, &GameScene::themeChanged);
    connect(m_hintTimer, &QTimer::timeout, this, &GameScene::hideHint);

    m_gamePopup.setMessageTimeout(3000);
    m_gamePopup.setHideOnMouseClick(true);
    addItem(&m_gamePopup);
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

    m_stonePixmapSize = QSize(static_cast<int>(m_cellSize * 1.4), static_cast<int>(m_cellSize * 1.4));
    updateStoneItems();                     // Resize means redraw of board items (stones)
    updateHintItems();                      // and move hint items
    updateTerritoryItems();

    if (m_placementMarkerItem) {            // Set the mouse/stone postion placementmarker
        removeItem(m_placementMarkerItem);
    }
    m_placementMarkerPixmapSize = QSize(static_cast<int>(m_cellSize / 4), static_cast<int>(m_cellSize / 4));
    m_placementMarkerItem = addPixmap(ThemeRenderer::self()->renderElement(ThemeRenderer::Element::PlacementMarker, m_placementMarkerPixmapSize));
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

    if (show == true) {
        m_hintTimer->start(static_cast<int>(Preferences::hintVisibleTime() * 1000));
    }
}

void GameScene::showMoveNumbers(bool show)
{
    m_showMoveNumbers = show;
    updateStoneItems();
}

void GameScene::showPlacementMarker(bool show)
{
    m_showPlacementMarker = show;
}

void GameScene::showMessage(const QString &message, int msecs)
{
    m_gamePopup.setMessageTimeout(msecs);
    if (message.isEmpty()) {
        m_gamePopup.forceHide();            // Now message hides the last one
    } else {
        m_gamePopup.showMessage(message, KGamePopupItem::BottomLeft, KGamePopupItem::ReplacePrevious);
    }
}

void GameScene::showTerritory(bool show)
{
    m_showTerritory = show;
    updateTerritoryItems();
}

void GameScene::updateStoneItems()
{
    const int halfStoneSize = m_stonePixmapSize.width() / 2;

    const Stone lastStone = (m_game->moves().size() > 0) ? m_game->lastMove().stone() : Stone::Invalid;

    for (QGraphicsPixmapItem *stoneItem : qAsConst(m_stoneItems)) {  // Clear all stone items
        removeItem(stoneItem);
    }
    m_stoneItems.clear();

    QGraphicsPixmapItem *stoneItem;
    const auto blackStones = m_game->stones(m_game->blackPlayer());
    for (const Stone &stone : blackStones) {
        ThemeRenderer::Element element = (stone == lastStone) ? ThemeRenderer::Element::BlackStoneLast : ThemeRenderer::Element::BlackStone;
        stoneItem = addPixmap(ThemeRenderer::self()->renderElement(element, m_stonePixmapSize));
        stoneItem->setZValue(2);
        const int xOff = stone.x() >= 'I' ? stone.x() - 'A' - 1 : stone.x() - 'A';
        stoneItem->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfStoneSize + 1,
                             m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfStoneSize + 1));
        m_stoneItems.append(stoneItem);
    }
    const auto whiteStones = m_game->stones(m_game->whitePlayer());
    for (const Stone &stone : whiteStones) {
        ThemeRenderer::Element element = (stone == lastStone) ? ThemeRenderer::Element::WhiteStoneLast : ThemeRenderer::Element::WhiteStone;
        stoneItem = addPixmap(ThemeRenderer::self()->renderElement(element, m_stonePixmapSize));
        stoneItem->setZValue(2);
        const int xOff = stone.x() >= 'I' ? stone.x() - 'A' - 1 : stone.x() - 'A';
        stoneItem->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfStoneSize + 1,
                             m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfStoneSize + 1));
        m_stoneItems.append(stoneItem);
    }

    if (m_showMoveNumbers) {
        int i = 0;
        const auto moves = m_game->moves();
        for (const Move &move : moves) {
            int xOff = move.stone().x() >= 'I' ? move.stone().x() - 'A' - 1 : move.stone().x() - 'A';
            QPointF pos = QPointF(m_gridRect.x() + xOff * m_cellSize,
                                  m_gridRect.y() + (m_boardSize - move.stone().y()) * m_cellSize);

            if (QGraphicsPixmapItem *item = static_cast<QGraphicsPixmapItem *>(itemAt(pos, QTransform()))) {
                // We found an item in the scene that is in our move numbers, so we paint it's move number
                // on top of the item and that's all.
                //TODO: Check for existing move number to do special treatment
                QPixmap pixmap = item->pixmap();
                QPainter painter(&pixmap);
                if (move.player()->isWhite()) {
                    painter.setPen(Qt::black);
                } else if (move.player()->isBlack()) {
                    painter.setPen(Qt::white);
                }
                QFont f = painter.font();
                f.setPointSizeF(halfStoneSize / 2);
                painter.setFont(f);
                painter.drawText(pixmap.rect(), Qt::AlignCenter, QString::number(i++));
                item->setPixmap(pixmap);
            }
        }
    }
}

void GameScene::updateHintItems()
{
    for (QGraphicsPixmapItem *item : qAsConst(m_hintItems)) {   // Old hint is invalid, remove it first
        removeItem(item);
    }
    m_hintItems.clear();

    QGraphicsPixmapItem *item;

    if (m_showHint) {
        const int halfStoneSize = m_stonePixmapSize.width() / 2;

        const auto moves = m_game->bestMoves(m_game->currentPlayer());
        for (const Stone &move : moves) {
            QPixmap stonePixmap;
            if (m_game->currentPlayer().isWhite()) {
                stonePixmap = ThemeRenderer::self()->renderElement(ThemeRenderer::Element::WhiteStoneTransparent, m_stonePixmapSize);
            } else if (m_game->currentPlayer().isBlack()) {
                stonePixmap = ThemeRenderer::self()->renderElement(ThemeRenderer::Element::BlackStoneTransparent, m_stonePixmapSize);
            }

            QPainter painter(&stonePixmap);
            if (m_game->currentPlayer().isWhite()) {
                painter.setPen(Qt::black);
            } else if (m_game->currentPlayer().isBlack()) {
                painter.setPen(Qt::white);
            }
            QFont f = painter.font();
            f.setPointSizeF(m_cellSize / 4);
            painter.setFont(f);
            painter.drawText(stonePixmap.rect(), Qt::AlignCenter, QString::number(move.value()));
            painter.end();

            item = addPixmap(stonePixmap);
            item->setZValue(4);
            const int xOff = move.x() >= 'I' ? move.x() - 'A' - 1 : move.x() - 'A';
            item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfStoneSize + 2,
                                 m_gridRect.y() + (m_boardSize - move.y()) * m_cellSize - halfStoneSize + 2));
            m_hintItems.append(item);
        }
    }
}

void GameScene::updateTerritoryItems()
{
    for(QGraphicsPixmapItem *item : qAsConst(m_territoryItems)) {  // Old territory is invalid, remove it first
        removeItem(item);
    }
    m_territoryItems.clear();

    QGraphicsPixmapItem *item;

    if (m_showTerritory) {
        QPixmap stonePixmap;
        const int halfCellSize = m_cellSize / 2;
        //qCDebug(KIGO_LOG) << "Fetching territory from engine ...";

        stonePixmap = ThemeRenderer::self()->renderElement(ThemeRenderer::Element::WhiteTerritory, QSize(m_cellSize, m_cellSize));
        const auto whiteStones = m_game->finalStates(Game::FinalState::FinalWhiteTerritory);
        for (const Stone &stone : whiteStones) {
            item = addPixmap(stonePixmap);
            item->setZValue(8);
            const int xOff = stone.x() >= 'I' ? stone.x() - 'A' - 1 : stone.x() - 'A';
            item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize + 2,
                                 m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfCellSize + 2));
            m_territoryItems.append(item);
        }

        stonePixmap = ThemeRenderer::self()->renderElement(ThemeRenderer::Element::BlackTerritory, QSize(m_cellSize, m_cellSize));
        const auto blackStones = m_game->finalStates(Game::FinalState::FinalBlackTerritory);
        for (const Stone &stone : blackStones) {
            item = addPixmap(stonePixmap);
            item->setZValue(8);
            const int xOff = stone.x() >= 'I' ? stone.x() - 'A' - 1 : stone.x() - 'A';
            item->setPos(QPointF(m_gridRect.x() + xOff * m_cellSize - halfCellSize + 2,
                                 m_gridRect.y() + (m_boardSize - stone.y()) * m_cellSize - halfCellSize + 2));
            m_territoryItems.append(item);
        }
    }
}

void GameScene::changeBoardSize(int size)
{
    m_boardSize = size;
    resizeScene(width(), height());
    invalidate(m_boardRect, QGraphicsScene::BackgroundLayer);
}

void GameScene::themeChanged()
{
    invalidate(sceneRect(), QGraphicsScene::AllLayers);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPixmap map;

    if (m_mouseRect.contains(event->scenePos())) {
        // Show a placement marker at the nearest board intersection
        // as a visual aid for the user.
        if (m_showPlacementMarker) {
            const int row = static_cast<int>((event->scenePos().x() - m_mouseRect.x()) / m_cellSize);
            const int col = static_cast<int>((event->scenePos().y() - m_mouseRect.y()) / m_cellSize);

            const int x = m_mouseRect.x() + row * m_cellSize + m_cellSize/2 - m_placementMarkerPixmapSize.width()/2;
            const int y = m_mouseRect.y() + col * m_cellSize + m_cellSize/2 - m_placementMarkerPixmapSize.height()/2;

            m_placementMarkerItem->setVisible(true);
            m_placementMarkerItem->setPos(x, y);
        } else {
            m_placementMarkerItem->setVisible(false);
        }

        if (m_game->currentPlayer().isHuman()) {
            if (m_game->currentPlayer().isWhite()) {
                map = ThemeRenderer::self()->renderElement(ThemeRenderer::Element::WhiteStoneTransparent, m_stonePixmapSize);
            } else if (m_game->currentPlayer().isBlack()) {
                map = ThemeRenderer::self()->renderElement(ThemeRenderer::Element::BlackStoneTransparent, m_stonePixmapSize);
            }
            Q_EMIT cursorPixmapChanged(map);
        }
    } else {
        m_placementMarkerItem->setVisible(false);
        Q_EMIT cursorPixmapChanged(map);
    }
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_mouseRect.contains(event->scenePos())) {
        int row = static_cast<int>((event->scenePos().x() - m_mouseRect.x()) / m_cellSize);
        const int col = static_cast<int>((event->scenePos().y() - m_mouseRect.y()) / m_cellSize);
        if (row < 0 || row >= m_boardSize || col < 0 || col >= m_boardSize) {
            return;
        }

        // Convert to Go board coordinates and try to play the move. GnuGo coordinates don't use the 'I'
        // column, if the row is bigger than 'I', we have to add 1 to jump over that.
        if (row >= 8) {
            row += 1;
        }
        m_game->playMove(m_game->currentPlayer(), Stone('A' + row, m_boardSize - col));
    }
}

void GameScene::drawBackground(QPainter *painter, const QRectF &)
{
    ThemeRenderer::self()->renderElement(ThemeRenderer::Element::Background, painter, sceneRect());
    ThemeRenderer::self()->renderElement(ThemeRenderer::Element::Board, painter, m_boardRect);

    const int width = m_cellSize / 16;
    const QColor color = QColor(20, 30, 20);
    painter->setPen(QPen(color, width));

    for (int i = 0; i < m_boardSize; i++) {
        const qreal offset = i * m_cellSize;
        painter->drawLine(QPointF(m_gridRect.left(),  m_gridRect.top() + offset),
                          QPointF(m_gridRect.right(), m_gridRect.top() + offset));
        painter->drawLine(QPointF(m_gridRect.left() + offset, m_gridRect.top()),
                          QPointF(m_gridRect.left() + offset, m_gridRect.bottom()));

        if (m_showLabels) {
            QChar c('A' + i);
            // GnuGo does not use the 'I' column (for whatever strange reason), we have to skip that too
            if (i >= 8) {
                c = QChar('A' + i + 1);
            }

            const QString n = QString::number(m_boardSize - i);
            QFont f = painter->font();
            f.setPointSizeF(m_cellSize / 4);
            painter->setFont(f);
            const QFontMetrics fm = painter->fontMetrics();

            // Draw vertical numbers for board coordinates
            const qreal yVert = m_gridRect.top() + offset + fm.descent();
            painter->drawText(QPointF(m_gridRect.left() - m_cellSize + 2, yVert), n);
            painter->drawText(QPointF(m_gridRect.right() + m_cellSize - fm.boundingRect(n).width() - 3, yVert), n);

            // Draw horizontal characters for board coordinates
            const qreal xHor = m_gridRect.left() + offset - fm.boundingRect(c).width() / 2;
            painter->drawText(QPointF(xHor, m_gridRect.top() - m_cellSize + fm.ascent() + 2), QString(c));
            painter->drawText(QPointF(xHor, m_gridRect.bottom() + m_cellSize - 3), QString(c));
        }
    }

    // Draw thicker connections on some defined points.
    // This is extremely helpful to orientate oneself especially on the 19x19 board.
    const int radius = m_cellSize / 10;
    painter->setBrush(color);
    painter->setRenderHint(QPainter::Antialiasing);

    // in order to center properly we need to take line width into account
    // if the line has an odd width, we shift 1/5 pixel
    const qreal centerOffset = (width % 2) ? 0.5 : 0.0;

    // only do this for the common board sizes,
    // other sizes are a bit odd anyway
    int numDots = 0;
    const int *dotPositions;

    if (m_boardSize == 9) {
        numDots = 5;
        dotPositions = dotPositions9;
    } else if (m_boardSize == 13) {
        numDots = 5;
        dotPositions = dotPositions13;
    } else if (m_boardSize == 19) {
        numDots = 9;
        dotPositions = dotPositions19;
    }

    for (int i = 0; i < numDots; ++i) {
        painter->drawEllipse(
            QPointF(m_gridRect.left() + m_cellSize*dotPositions[i*2] + centerOffset,
                    m_gridRect.top() + m_cellSize*dotPositions[i*2+1] + centerOffset),
            radius, radius);
    }
}

} // End of namespace Kigo
