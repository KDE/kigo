/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_GAMEVIEW_H
#define KIGO_GAMEVIEW_H

#include <QGraphicsView>

namespace Kigo {

class GameScene;

/**
 * This class represents a view on a Go game view. This widget can be
 * included into a main window.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class GameView : public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * Standard constructor. Creates a game view based on a given game scene.
     *
     * @param scene The game scene
     * @param parent The (optional) parent widget
     * @see GameScene
     */
    explicit GameView(GameScene *scene, QWidget *parent = nullptr);

private Q_SLOTS:
    void changeCursor(const QPixmap &cursorPixmap);

private:
    //void drawForeground(QPainter *painter, const QRectF &rect);
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    GameScene * const m_gameScene;  ///< Pointer to the game scene
};

} // End of namespace Kigo

#endif
