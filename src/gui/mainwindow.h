/*
    Copyright 2008 Sascha Peilicke <sasch.pe@gmx.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KIGO_MAINWINDOW_H
#define KIGO_MAINWINDOW_H

#include <KXmlGuiWindow>

class QAction;
class KToggleAction;
class QDockWidget;
class QUndoView;

namespace Kigo {

class Game;
class GameScene;
class GameView;
class Player;
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
    explicit MainWindow(const QString &fileName = QLatin1String(""), QWidget *parent = nullptr);

private slots:
    void newGame();                         ///< Configure new game
    void loadGame();                        ///< Configure loaded game
    bool loadGame(const QString &fileName);
    void getMoreGames();
    void backendError();                    ///<
    void saveGame();                        ///< Save current game state
    void startGame();                       ///< React on start button
    void finishGame();                      ///< Final screen, scores, ...
    void undo();                            ///< Undo last move
    void redo();                            ///< Redo last move
    void pass();                            ///< Pass current move
    void hint();                            ///< Show a playing hint
    void showPreferences();                 ///< Show configuration dialog
    void applyPreferences();                ///< React on changed config
    void showBusy(bool busy);               ///< Signal a busy app
    void showFinishGameAction();
    void playerChanged();
    void generateMove();
    void passMovePlayed(const Player &);

private:
    void setupActions();
    void setupDockWindows();

    bool isBackendWorking();

    Game *m_game;                           ///< Handles complete game state
    GameScene *m_gameScene;                 ///< QGraphicsScene for Go board
    GameView *m_gameView;                   ///< QGraphicsView for Go board

    SetupWidget *m_setupWidget;             ///< Part of dock widget
    QUndoView *m_undoView;                  ///< Part of dock widget

    QDockWidget *m_setupDock;               ///< Game setup dock widget
    QDockWidget *m_gameDock;                ///< Game info dock widget
    QDockWidget *m_movesDock;               ///< Move history dock widget
    QDockWidget *m_errorDock;               ///< Dock shown when errors occur

    QAction *m_newGameAction;
    QAction *m_loadGameAction;
    QAction *m_getMoreGamesAction;
    QAction *m_saveAction;                  ///< Action to save the current game
    QAction *m_undoMoveAction;              ///< Action to jump to the last move
    QAction *m_redoMoveAction;              ///< Action to jump to the next move
    QAction *m_passMoveAction;              ///< Action to pass current move
    QAction *m_hintAction;
    QAction *m_startGameAction;
    QAction *m_finishGameAction;
    KToggleAction *m_moveNumbersAction;
};

} // End of namespace Kigo

#endif
