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
 * @file This file is part of KGO and defines the class SetupScreen.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_SETUPSCREEN_H
#define KGO_SETUPSCREEN_H

#include "ui_setupscreen.h"

#include <QWidget>

namespace KGo {

class GoEngine;
class GameScene;

/**
 * The game setup screen lets the user choose a variety of game
 * options preview the configured game and start it.
 *
 * The user can make all necessary settings for the game he/she
 * wishes to start, the welcome screen only sets the KGo::Game to
 * these values and emits a signal when the user wants to start.
 * Further interaction with the KGo::Game (and it's graphical
 * representation KGo::BoardView and KGo::BoardScene) has to happen
 * elsewhere.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class SetupScreen : public QWidget, private Ui::SetupScreen
{
    Q_OBJECT

public:
    /**
     * Standard constructor. Sets up the loaded user interface and initial state.
     *
     * @param scene A valid game scene
     * @param parent The parent widget or none
     */
    explicit SetupScreen(GameScene *scene, QWidget *parent = 0);
    ~SetupScreen();

    /**
     * Use the setup screen to configure a new game. This sets the right widgets
     * for that purpose.
     */
    void setupNewGame();

    /**
     * Use the setup screen to configure a loaded game. This sets the right widgets
     * based on the parameters.
     *
     * @param fileName The SGF file to load the game from
     */
    void setupLoadedGame(const QString &fileName);

signals:
    /**
     * This signal is emmited when the user clicked the 'start game' button.
     */
    void startClicked();

private slots:
    void on_whitePlayerCombo_currentIndexChanged(const QString &);
    void on_blackPlayerCombo_currentIndexChanged(const QString &);
    void on_startMoveSpinBox_valueChanged(int);
    void on_difficultySlider_valueChanged(int);
    void on_sizeGroupBox_changed(int);
    void on_sizeOtherSpinBox_valueChanged(int);
    void on_handicapGroupBox_toggled(bool);
    void on_handicapSpinBox_valueChanged(int);
    void on_startButton_clicked();

private:
    void loadSettings();            ///< Load KConfigXT application settings
    void saveSettings();            ///< Store KConfigXT application settings

    GoEngine * const m_gameEngine;  ///<
};

} // End of namespace KGo

#endif
