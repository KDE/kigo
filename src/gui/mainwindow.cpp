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

#include "mainwindow.h"
#include "game/game.h"
#include "game/score.h"
#include "gui/config/generalconfig.h"
#include "gui/graphicsview/gamescene.h"
#include "gui/graphicsview/gameview.h"
#include "gui/graphicsview/themerenderer.h"
#include "gui/widgets/errorwidget.h"
#include "gui/widgets/gamewidget.h"
#include "gui/widgets/setupwidget.h"
#include "preferences.h"

#include <KAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <KFileDialog>
#include <KGameThemeSelector>
#include <knewstuff2/engine.h>
#include <KStandardDirs>
#include <KStandardGameAction>
#include <KToggleAction>

#include <QDockWidget>
#include <QTimer>
#include <QUndoView>

namespace Kigo {

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent), m_game(new Game(this))
    , m_gameScene(new GameScene(m_game)), m_gameView(new GameView(m_gameScene))
{
    setCentralWidget(m_gameView);

    setupActions();
    setupDockWindows();
    setupGUI(QSize(800, 700), KXmlGuiWindow::ToolBar | KXmlGuiWindow::Keys |
                              KXmlGuiWindow::Save | KXmlGuiWindow::Create);

    connect(m_game, SIGNAL(waiting(bool)), this, SLOT(showBusy(bool)));
    connect(m_game, SIGNAL(consecutivePassMovesPlayed(int)), this, SLOT(showFinishGameAction()));
    connect(m_game, SIGNAL(resigned(const Player &)), this, SLOT(finishGame()));
    connect(m_game, SIGNAL(passMovePlayed(const Player &)), this, SLOT(passMovePlayed(const Player &)));

    if (isBackendWorking()) {
        newGame();
    } else {
        backendError();
    }
}

void MainWindow::newGame()
{
    m_game->start(Preferences::engineCommand());

    m_gameScene->showTerritory(false);
    m_gameView->setInteractive(false);

    m_newGameAction->setEnabled(true);
    m_loadGameAction->setEnabled(true);
    m_saveAction->setEnabled(false);
    m_startGameAction->setEnabled(true);
    m_finishGameAction->setEnabled(false);

    m_undoMoveAction->setEnabled(false);
    m_redoMoveAction->setEnabled(false);
    m_passMoveAction->setEnabled(false);
    m_hintAction->setEnabled(false);
    m_moveNumbersAction->setEnabled(false);

    disconnect(m_game, SIGNAL(canRedoChanged(bool)), m_redoMoveAction, SLOT(setEnabled(bool)));
    disconnect(m_game, SIGNAL(canUndoChanged(bool)), m_undoMoveAction, SLOT(setEnabled(bool)));
    disconnect(m_game, SIGNAL(currentPlayerChanged(const Player &)), this, SLOT(playerChanged()));

    m_gameDock->setVisible(false);
    m_gameDock->toggleViewAction()->setEnabled(false);
    m_movesDock->setVisible(false);
    m_movesDock->toggleViewAction()->setEnabled(false);
    m_setupDock->setVisible(true);
    m_errorDock->setVisible(false);

    m_setupWidget->newGame();
    m_gameScene->showMessage(i18n("Set up a new game..."));
}

void MainWindow::loadGame()
{
    QString fileName = KFileDialog::getOpenFileName(KUrl(KStandardDirs::locate("appdata", "games/")), "*.sgf");
    if (!fileName.isEmpty()) {
        m_game->start(Preferences::engineCommand());

        m_gameScene->showTerritory(false);
        m_gameView->setInteractive(false);

        m_newGameAction->setEnabled(true);
        m_loadGameAction->setEnabled(true);
        m_saveAction->setEnabled(false);
        m_startGameAction->setEnabled(true);
        m_finishGameAction->setEnabled(false);

        m_undoMoveAction->setEnabled(false);
        m_redoMoveAction->setEnabled(false);
        m_passMoveAction->setEnabled(false);
        m_hintAction->setEnabled(false);
        m_moveNumbersAction->setEnabled(true);

        disconnect(m_game, SIGNAL(canRedoChanged(bool)), m_redoMoveAction, SLOT(setEnabled(bool)));
        disconnect(m_game, SIGNAL(canUndoChanged(bool)), m_undoMoveAction, SLOT(setEnabled(bool)));
        disconnect(m_game, SIGNAL(currentPlayerChanged(const Player &)), this, SLOT(playerChanged()));

        m_gameDock->setVisible(false);
        m_gameDock->toggleViewAction()->setEnabled(false);
        m_movesDock->setVisible(false);
        m_movesDock->toggleViewAction()->setEnabled(false);
        m_setupDock->setVisible(true);
        m_errorDock->setVisible(false);

        m_setupWidget->loadedGame(fileName);
        m_gameScene->showMessage(i18n("Set up a loaded game..."));
    } else {
        m_gameScene->showMessage(i18n("Unable to load game..."));
        //Note: New game implied here
    }
}

void MainWindow::getMoreGames()
{
    KNS::Engine engine(0);

    if (engine.init("kigo-games.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(this);
        /*if (entries.size() > 0) {
            // do something with the modified entries here if you want
            // such as rescaning your data folder or whatnot
        }*/
    }
}

void MainWindow::backendError()
{
    m_gameView->setInteractive(false);

    m_newGameAction->setEnabled(false);
    m_loadGameAction->setEnabled(false);
    m_saveAction->setEnabled(false);
    m_startGameAction->setEnabled(false);
    m_finishGameAction->setEnabled(false);

    m_undoMoveAction->setEnabled(false);
    m_redoMoveAction->setEnabled(false);
    m_passMoveAction->setEnabled(false);
    m_hintAction->setEnabled(false);
    m_moveNumbersAction->setEnabled(false);

    disconnect(m_game, SIGNAL(canRedoChanged(bool)), m_redoMoveAction, SLOT(setEnabled(bool)));
    disconnect(m_game, SIGNAL(canUndoChanged(bool)), m_undoMoveAction, SLOT(setEnabled(bool)));
    disconnect(m_game, SIGNAL(currentPlayerChanged(const Player &)), this, SLOT(playerChanged()));
    m_gameDock->setVisible(false);
    m_gameDock->toggleViewAction()->setEnabled(false);
    m_movesDock->setVisible(false);
    m_movesDock->toggleViewAction()->setEnabled(false);
    m_setupDock->setVisible(false);
    m_errorDock->setVisible(true);
}

void MainWindow::saveGame()
{
    QString fileName = KFileDialog::getSaveFileName(KUrl(QDir::homePath()), "*.sgf");

    if (!fileName.isEmpty()) {
        if (m_game->save(fileName))
            m_gameScene->showMessage(i18n("Game saved..."));
        else
            m_gameScene->showMessage(i18n("Unable to save game."));
    }
}

void MainWindow::startGame()
{
    m_saveAction->setEnabled(true);
    m_startGameAction->setEnabled(false);
    m_finishGameAction->setEnabled(false);

    m_setupWidget->commit();

    //Decide on players how to display the UI
    if (m_game->whitePlayer().isHuman() || m_game->blackPlayer().isHuman()) {
        connect(m_game, SIGNAL(canRedoChanged(bool)), m_redoMoveAction, SLOT(setEnabled(bool)));
        connect(m_game, SIGNAL(canUndoChanged(bool)), m_undoMoveAction, SLOT(setEnabled(bool)));

        m_passMoveAction->setEnabled(true);
        m_hintAction->setEnabled(true);
        m_moveNumbersAction->setEnabled(true);

        m_gameView->setInteractive(true);
        m_undoView->setEnabled(true);
    } else {
        m_passMoveAction->setEnabled(false);
        m_hintAction->setEnabled(false);
        m_moveNumbersAction->setEnabled(false);

        m_gameView->setInteractive(false);
        m_undoView->setEnabled(false);
    }

    connect(m_game, SIGNAL(currentPlayerChanged(const Player &)), this, SLOT(playerChanged()));
    // Trigger the slot once to make a move if the starting player
    // (black) is a computer player.
    playerChanged();

    m_setupDock->setVisible(false);
    m_errorDock->setVisible(false);
    m_gameDock->setVisible(true);
    m_gameDock->toggleViewAction()->setEnabled(true);
    m_movesDock->setVisible(true);
    m_movesDock->toggleViewAction()->setEnabled(true);

    m_gameScene->showMessage(i18n("Game started..."));

    //TODO: Fix undo view clicking somehow
    m_undoView->setEnabled(false);
}

void MainWindow::finishGame()
{
    m_gameView->setInteractive(false);
    m_gameScene->showHint(false);
    m_gameScene->showTerritory(true);

    m_undoMoveAction->setEnabled(false);
    m_redoMoveAction->setEnabled(false);
    m_passMoveAction->setEnabled(false);
    m_hintAction->setEnabled(false);
    m_moveNumbersAction->setEnabled(false);
    m_startGameAction->setEnabled(false);
    m_finishGameAction->setEnabled(false);

    m_undoView->setEnabled(false);

    kDebug() << "Fetching final score from engine ...";
    Score score = m_game->estimatedScore();
    QString name;
    if (score.color() == 'W') {
        name = m_game->whitePlayer().name() + " (White)";
    } else {
        name = m_game->blackPlayer().name() + " (Black)";
    }
    // Show a score message that stays visible until the next
    // popup message arrives
    m_gameScene->showMessage(i18n("%1 has won this game with a score of %2 (bounded by %3 and %4).",
                             name, score.score(), score.upperBound(), score.lowerBound()), 0);
}

void MainWindow::undo()
{
    if (m_game->undoMove()) {
        m_gameScene->showMessage(i18n("Undone move"));
        m_gameScene->showHint(false);
    }
}

void MainWindow::redo()
{
    if (m_game->redoMove()) {
        m_gameScene->showMessage(i18n("Redone move"));
        m_gameScene->showHint(false);
    }
}

void MainWindow::pass()
{
    if (m_game->playMove(m_game->currentPlayer())) {     // E.g. Pass move
        //m_gameScene->showMessage(i18n("Passed move"));
        m_gameScene->showHint(false);
    }
}

void MainWindow::hint()
{
    m_gameScene->showHint(true);
    //m_gameScene->showMessage(i18n("These are the recommended moves..."));
}

void MainWindow::showPreferences()
{
    if (KConfigDialog::showDialog("settings"))
        return;

    KConfigDialog *dialog = new KConfigDialog(this, "settings", Preferences::self());
    dialog->addPage(new GeneralConfig(), i18n("General"), "preferences-other");
    dialog->addPage(new KGameThemeSelector(dialog, Preferences::self()), i18n("Themes"), "games-config-theme");
    dialog->setHelp(QString(), "Kigo");
    connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(applyPreferences()));
    dialog->show();
}

void MainWindow::applyPreferences()
{
    //kDebug() << "Update settings based on changed configuration...";
    m_gameScene->showLabels(Preferences::showBoardLabels());

    ThemeRenderer::self()->loadTheme(Preferences::theme());

    if (!isBackendWorking()) {
        backendError();
    } else if (m_game->engineCommand() != Preferences::engineCommand()) {
        // Restart the Go game if the game command was changed by the user.
        m_gameScene->showMessage(i18n("Backend was changed, restart necessary..."));
        newGame();
    }
}

void MainWindow::showBusy(bool busy)
{
    //Decide on players how to display the UI
    if (m_game->whitePlayer().isHuman() || m_game->blackPlayer().isHuman()) {
        if (busy) {
            m_undoMoveAction->setDisabled(true);
            m_redoMoveAction->setDisabled(true);
        } else {
            // Only re-enable undo/redo if it actually makes sense
            if (m_game->canUndo()) {
                m_undoMoveAction->setDisabled(false);
            }
            if (m_game->canRedo()) {
                m_redoMoveAction->setDisabled(false);
            }
        }
        m_passMoveAction->setDisabled(busy);
        m_hintAction->setDisabled(busy);
        m_moveNumbersAction->setDisabled(busy);
        m_undoView->setEnabled(false);
    }

    m_gameView->setInteractive(!busy);
}

void MainWindow::showFinishGameAction()
{
    m_finishGameAction->setEnabled(true);
}

void MainWindow::playerChanged()
{
    if (!m_game->currentPlayer().isHuman() && !m_game->isFinished()) {
        QTimer::singleShot(200, this, SLOT(generateMove()));
    }
}

void MainWindow::generateMove()
{
    m_game->generateMove(m_game->currentPlayer());
}

void MainWindow::passMovePlayed(const Player &player)
{
    if (player.isComputer()) {
        if (player.isWhite()) {
            m_gameScene->showMessage(i18n("White passed"));
        } else {
            m_gameScene->showMessage(i18n("Black passed"));
        }
    }
}

void MainWindow::setupActions()
{
    // Game menu
    m_newGameAction = KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    m_loadGameAction = KStandardGameAction::load(this, SLOT(loadGame()), actionCollection());
    m_getMoreGamesAction = new KAction(KIcon("get-hot-new-stuff"), i18nc("@action", "Get More Games..."), this);
    m_getMoreGamesAction->setShortcut(Qt::CTRL + Qt::Key_G);
    m_getMoreGamesAction->setToolTip(i18nc("@action", "Get More Games..."));
    connect(m_getMoreGamesAction, SIGNAL(triggered(bool)), this, SLOT(getMoreGames()));
    actionCollection()->addAction("get_more_games", m_getMoreGamesAction);
    m_saveAction = KStandardGameAction::save(this, SLOT(saveGame()), actionCollection());
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    m_startGameAction = new KAction(KIcon("media-playback-start"), i18nc("@action", "Start Game"), this);
    m_startGameAction->setShortcut(Qt::Key_S);
    m_startGameAction->setToolTip(i18nc("@action", "Start Game"));
    connect(m_startGameAction, SIGNAL(triggered(bool)), this, SLOT(startGame()));
    actionCollection()->addAction("game_start", m_startGameAction);

    m_finishGameAction = new KAction(KIcon("media-playback-stop"), i18nc("@action", "Finish Game"), this);
    m_finishGameAction->setShortcut(Qt::Key_F);
    m_finishGameAction->setToolTip(i18nc("@action", "Finish Game"));
    connect(m_finishGameAction, SIGNAL(triggered(bool)), this, SLOT(finishGame()));
    actionCollection()->addAction("game_finish", m_finishGameAction);

    // Move menu
    m_undoMoveAction = KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
    m_redoMoveAction = KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
    m_passMoveAction = KStandardGameAction::endTurn(this, SLOT(pass()), actionCollection());
    m_passMoveAction->setText(i18nc("@action:inmenu Move", "Pass Move"));
    m_passMoveAction->setShortcut(Qt::Key_P);
    m_hintAction = KStandardGameAction::hint(this, SLOT(hint()), actionCollection());

    // View menu
    m_moveNumbersAction = new KToggleAction(KIcon("lastmoves"), i18nc("@action:inmenu View", "Show Move &Numbers"), this);
    m_moveNumbersAction->setShortcut(Qt::Key_N);
    m_moveNumbersAction->setChecked(Preferences::showMoveNumbers());
    connect(m_moveNumbersAction, SIGNAL(toggled(bool)), m_gameScene, SLOT(showMoveNumbers(bool)));
    actionCollection()->addAction("move_numbers", m_moveNumbersAction);

    // Settings menu
    KStandardAction::preferences(this, SLOT(showPreferences()), actionCollection());
}

void MainWindow::setupDockWindows()
{
    // Setup dock
    m_setupDock = new QDockWidget(i18nc("@title:window", "Game Setup"), this);
    m_setupDock->setObjectName("setupDock");
    m_setupDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_setupWidget = new SetupWidget(m_game, this);
    m_setupDock->setWidget(m_setupWidget);
    connect(m_setupWidget, SIGNAL(startClicked()), this, SLOT(startGame()));
    //m_setupDock->toggleViewAction()->setText(i18nc("@title:window", "Game setup"));
    //m_setupDock->toggleViewAction()->setShortcut(Qt::Key_S);
    //actionCollection()->addAction("show_setup_panel", m_setupDock->toggleViewAction());
    addDockWidget(Qt::BottomDockWidgetArea, m_setupDock);

    // Game dock
    m_gameDock = new QDockWidget(i18nc("@title:window", "Information"), this);
    m_gameDock->setObjectName("gameDock");
    m_gameDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_gameDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    GameWidget *gameWidget = new GameWidget(m_game, this);
    connect(gameWidget, SIGNAL(finishClicked()), this, SLOT(finishGame()));
    m_gameDock->setWidget(gameWidget);
    //m_gameDock->toggleViewAction()->setText(i18nc("@title:window", "Information"));
    //m_gameDock->toggleViewAction()->setShortcut(Qt::Key_G);
    //actionCollection()->addAction("show_game_panel", m_gameDock->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea, m_gameDock);

    // Move history dock
    m_movesDock = new QDockWidget(i18nc("@title:window", "Moves"), this);
    m_movesDock->setObjectName("movesDock");
    m_undoView = new QUndoView(m_game->undoStack());
    m_undoView->setEmptyLabel(i18n("No move"));
    m_undoView->setAlternatingRowColors(true);
    m_undoView->setFocusPolicy(Qt::NoFocus);
    m_undoView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_undoView->setSelectionMode(QAbstractItemView::NoSelection);
    m_movesDock->setWidget(m_undoView);
    m_movesDock->toggleViewAction()->setText(i18nc("@title:window", "Moves"));
    m_movesDock->toggleViewAction()->setShortcut(Qt::Key_M);
    actionCollection()->addAction("show_moves_panel", m_movesDock->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea, m_movesDock);

    m_errorDock = new QDockWidget(i18nc("@title:window", "Error"), this);
    m_errorDock->setObjectName("errorDock");
    m_errorDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    ErrorWidget *errorWidget = new ErrorWidget(this);
    connect(errorWidget, SIGNAL(configureClicked()), this, SLOT(showPreferences()));
    m_errorDock->setWidget(errorWidget);
    //m_errorDock->toggleViewAction()->setText(i18nc("@title:window", "Error"));
    //m_errorDock->toggleViewAction()->setShortcut(Qt::Key_E);
    //actionCollection()->addAction("show_error_panel", m_errorDock->toggleViewAction());
    addDockWidget(Qt::BottomDockWidgetArea, m_errorDock);
}

bool MainWindow::isBackendWorking()
{
    Game game;
    return game.start(Preferences::engineCommand());
}

} // End of namespace Kigo

#include "moc_mainwindow.cpp"
