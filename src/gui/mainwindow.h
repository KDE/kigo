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

#ifndef KIGO_MAINWINDOW_H
#define KIGO_MAINWINDOW_H

#include <KXmlGuiWindow>

class KAction;
class KToggleAction;
class QDockWidget;

namespace Kigo {

class GoEngine;
class GameScene;
class GameView;
class SetupWidget;

/**
 * The MainWindow class acts as the main window for the Kigo graphical
 * user interface.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void newGame();                         ///< Configure new game
    void loadGame();                        ///< Configure loaded game
    void editGame();
    void saveGame();                        ///< Save the current game state
    void startGame();                       ///< React on start button
    void finishGame();
    void undo();                            ///< Undo last move
    void redo();                            ///< Redo last move
    void pass();                            ///< Pass current move
    void hint();
    void showPreferences();                 ///< Show configuration dialog
    void updatePreferences();               ///< React changed configuration
    void showBusy(bool busy);               ///< Signal the user a busy app
    void showFinish();

private:
    void setupActions();
    void setupDockWindows();

    GoEngine *m_engine;
    GameScene *m_gameScene;
    GameView *m_gameView;

    SetupWidget *m_setupWidget;

    QDockWidget *m_setupDock;
    QDockWidget *m_gameDock;
    QDockWidget *m_movesDock;
    QDockWidget *m_editDock;

    KAction *m_newGameAction;
    KAction *m_loadGameAction;
    KAction *m_saveAction;                  ///< Action to save the current game
    KAction *m_editorAction;
    KAction *m_undoMoveAction;              ///< Action to jump to the last move
    KAction *m_redoMoveAction;              ///< Action to jump to the next move
    KAction *m_passMoveAction;              ///< Action to pass current move
    KAction *m_hintAction;
    KAction *m_startGameAction;
    KAction *m_finishGameAction;
    KToggleAction *m_moveNumbersAction;
};

} // End of namespace Kigo

#endif
