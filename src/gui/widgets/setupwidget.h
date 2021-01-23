/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_SETUPWIDGET_H
#define KIGO_SETUPWIDGET_H

#include "ui_setupwidget.h"

#include <QWidget>

namespace Kigo {

class Game;

/**
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.5
 */
class SetupWidget : public QWidget, private Ui::SetupWidget
{
    Q_OBJECT

public:
    explicit SetupWidget(Game *game, QWidget *parent = nullptr);
    ~SetupWidget() override;

Q_SIGNALS:
    void startClicked();

public Q_SLOTS:
    /**
     *
     */
    void newGame();

    /**
     * @param fileName The SGF file to load the game from
     */
    void loadedGame(const QString &fileName);

    /**
     * To actually commit the configured values to the Go backend,
     * this method has to be called.
     */
    void commit();

private Q_SLOTS:
    void on_startMoveSpinBox_valueChanged(int);
    void onBoardSizeChanged();
    void on_handicapSpinBox_valueChanged(int);
    void whiteIsComputer(bool);
    void blackIsComputer(bool);

private:
    void updateHandicapBox();
    void loadSettings();
    void saveSettings();

    Game *m_game;
    QString m_lastFileName;
    int m_lastFixedHandicap;
};

} // End of namespace Kigo

#endif
