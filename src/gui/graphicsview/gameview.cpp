/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gameview.h"
#include "gamescene.h"

#include <QResizeEvent>

namespace Kigo {

GameView::GameView(GameScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_gameScene(scene)
{
    setCacheMode(QGraphicsView::CacheBackground);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setOptimizationFlags(
                         QGraphicsView::DontSavePainterState |
                         QGraphicsView::DontAdjustForAntialiasing);

    connect(m_gameScene, &GameScene::cursorPixmapChanged, this, &GameView::changeCursor);
}

void GameView::changeCursor(const QPixmap &cursorPixmap)
{
    if (!isInteractive() || cursorPixmap.isNull()) {
        unsetCursor();
    } else {
        setCursor(QCursor(cursorPixmap));
    }
}

void GameView::showEvent(QShowEvent *)
{
    // Make sure that the game scene has the correct size according to the current view
    // This is necessary because one scene is shared by multiple views but changing them
    // creates no resizeEvent, so resize when another view is shown.
    m_gameScene->resizeScene(width(), height());
}

void GameView::resizeEvent(QResizeEvent *event)
{
    m_gameScene->resizeScene(event->size().width(), event->size().height());
}

} // End of namespace Kigo

#include "moc_gameview.cpp"
