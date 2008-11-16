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

#ifndef KGO_MAINWINDOW_H
#define KGO_MAINWINDOW_H

#include <KXmlGuiWindow>

class KToggleAction;
class KAction;
class QStackedWidget;

namespace Kigo {

class SetupScreen;
class GameScreen;
class MessageScreen;
class GameScene;

/**
 * The MainWindow class acts as the main window of the Kigo graphical
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
    void showPreferences();                 ///< Show configuration dialog
    void updatePreferences();               ///< React on user changed configuration
    void showBusy(bool busy);

private:
    MessageScreen *messageScreen();         ///< Lazy instantiation for faster startup
    SetupScreen *setupScreen();             ///< Lazy instantiation for faster startup
    GameScreen *gameScreen();               ///< Lazy instantiation for faster startup

    void setupActions();

    QStackedWidget * const m_mainWidget;
    GameScene * const m_gameScene;

    MessageScreen *m_messageScreen;         ///< Pointer to the engine message screen
    SetupScreen *m_setupScreen;             ///< Pointer to the game setup screen
    GameScreen *m_gameScreen;               ///< Pointer to the game playing screen

    KAction *m_newGameAction;
    KAction *m_loadGameAction;
    KAction *m_saveAsAction;                ///< Action to save the current game
    KAction *m_editGameAction;
    KAction *m_previousMoveAction;          ///< Action to jump to the previous move
    KAction *m_nextMoveAction;              ///< Action to jump to the next move
    KAction *m_passMoveAction;              ///< Action to pass current move
    KAction *m_hintAction;
    KToggleAction *m_moveHistoryAction;
};

} // End of namespace Kigo

#endif
