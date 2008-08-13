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
 * @file This file is part of KGO and defines the class MainWindow,
 *       which acts as the main window of the graphical user interface
 *       for KGo.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_MAINWINDOW_H
#define KGO_MAINWINDOW_H

#include <KXmlGuiWindow>

class KToggleAction;
class KAction;
class QStackedWidget;

namespace KGo {

class SetupScreen;
class GameScreen;
class ErrorScreen;
class GameScene;

/**
 * The MainWindow class acts as the main window of the KGo graphical
 * user interface. Holds the different screens for setup and gameplay,
 * displays configuration dialog(s) and the GameScene object (which is
 * modified and passed around the differend sub-screens).
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, bool startDemo = false);

private slots:
    void newGame();                         ///< Configure new game with setupscreen
    void loadGame();                        ///< Configure loaded game with ~
    void saveGame();                        ///< Save the current game state
    void startGame();                       ///< React on setupscreen start button
    void undo();                            ///< Undo last move
    void redo();                            ///< Redo last move
    void pass();                            ///< Pass current move
    void hint();
    void toggleDemoMode();                  ///<
    void showPreferences();                 ///< Show configuration dialog
    void updatePreferences();               ///< React on user changed configuration

private:
    ErrorScreen *errorScreen();             ///< Lazy instantiation for faster startup
    SetupScreen *setupScreen();             ///< Lazy instantiation for faster startup
    GameScreen *gameScreen();               ///< Lazy instantiation for faster startup

    void setupActions();

    QStackedWidget * const m_mainWidget;
    GameScene * const m_gameScene;

    ErrorScreen *m_errorScreen;             ///< Pointer to the engine error screen
    SetupScreen *m_setupScreen;             ///< Pointer to the game setup screen
    GameScreen *m_gameScreen;               ///< Pointer to the game playing screen

    KAction *m_newGameAction;
    KAction *m_loadGameAction;
    KAction *m_saveAsAction;                ///< Action to save the current game
    KAction *m_editGameAction;
    KAction *m_firstMoveAction;             ///< Action to jump to the first move
    KAction *m_previousMoveAction;          ///< Action to jump to the previous move
    KAction *m_nextMoveAction;              ///< Action to jump to the next move
    KAction *m_lastMoveAction;              ///< Action to jump to the last move
    KAction *m_passMoveAction;              ///< Action to pass current move
    KAction *m_hintAction;
    KToggleAction *m_moveHistoryAction;
};

} // End of namespace KGo

#endif
