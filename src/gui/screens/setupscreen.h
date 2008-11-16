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

#ifndef KGO_SETUPSCREEN_H
#define KGO_SETUPSCREEN_H

#include "ui_setupscreen.h"

#include <QWidget>

namespace Kigo {

class GoEngine;
class GameScene;

/**
 * The game setup screen lets the user choose a variety of game
 * options preview the configured game and start it.
 *
 * The user can make all necessary settings for the game he/she
 * wishes to start, the welcome screen only sets the Kigo::Game to
 * these values and emits a signal when the user wants to start.
 * Further interaction with the Kigo::Game (and it's graphical
 * representation Kigo::BoardView and Kigo::BoardScene) has to happen
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
    void on_startMoveSpinBox_valueChanged(int);
    void on_sizeGroupBox_changed(int);
    void on_sizeOtherSpinBox_valueChanged(int);
    void on_handicapGroupBox_toggled(bool);
    void on_handicapSpinBox_valueChanged(int);
    void on_startButton_clicked();

    void updateHandicapBox();

private:
    void loadSettings();            ///< Load KConfigXT application settings
    void saveSettings();            ///< Store KConfigXT application settings

    GoEngine * const m_gameEngine;  ///<

    int m_lastFixedHandicap;        ///<
    QString m_lastFileName;         ///<
};

} // End of namespace Kigo

#endif
