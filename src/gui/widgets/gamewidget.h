/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_GAMEWIDGET_H
#define KIGO_GAMEWIDGET_H

#include "ui_gamewidget.h"

#include <QWidget>

namespace Kigo {

class Game;

/**
 * @see Game
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.5
 */
class GameWidget : public QWidget, private Ui::GameWidget
{
    Q_OBJECT

public:
    explicit GameWidget(Game *game, QWidget *parent = nullptr);

Q_SIGNALS:
    void finishClicked();

public Q_SLOTS:
    void init();
    void update();

private Q_SLOTS:
    void finishButtonClicked();
    void enableFinishButton();

private:
    Game *m_game;
};

} // End of namespace Kigo

#endif
