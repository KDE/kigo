/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include <QAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <KNS3/QtQuickDialogWrapper>
#include <KStandardGameAction>
#include <KToggleAction>
#include <QIcon>
#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgamethemeselector.h>

#include <QDockWidget>
#include <QFileDialog>
#include <QTimer>
#include <QUndoView>

namespace Kigo {

MainWindow::MainWindow(const QString &fileName, QWidget *parent)
    : KXmlGuiWindow(parent), m_game(new Game(this))
    , m_gameScene(new GameScene(m_game)), m_gameView(new GameView(m_gameScene))
{
    setCentralWidget(m_gameView);

    setupActions();
    setupDockWindows();
    setupGUI(QSize(800, 700), KXmlGuiWindow::ToolBar | KXmlGuiWindow::Keys |
                              KXmlGuiWindow::Save | KXmlGuiWindow::Create);

    connect(m_game, &Game::waiting, this, &MainWindow::showBusy);
    connect(m_game, &Game::consecutivePassMovesPlayed, this, &MainWindow::showFinishGameAction);
    connect(m_game, &Game::resigned, this, &MainWindow::finishGame);
    connect(m_game, &Game::passMovePlayed, this, &MainWindow::passMovePlayed);

    if (isBackendWorking()) {
        if (!loadGame(fileName)) {
            newGame();
        }
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

    disconnect(m_game, &Game::canRedoChanged, m_redoMoveAction, &QAction::setEnabled);
    disconnect(m_game, &Game::canUndoChanged, m_undoMoveAction, &QAction::setEnabled);
    disconnect(m_game, &Game::currentPlayerChanged, this, &MainWindow::playerChanged);

    m_gameDock->setVisible(false);
    m_gameDock->toggleViewAction()->setEnabled(false);
    m_movesDock->setVisible(false);
    m_movesDock->toggleViewAction()->setEnabled(false);
    m_setupDock->setVisible(true);
    m_errorDock->setVisible(false);

    m_setupWidget->newGame();
    QTimer::singleShot(0, this, [this] { m_gameScene->showMessage(i18n("Set up a new game...")); } );
}

void MainWindow::loadGame()
{
    const QString folderName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("games"), QStandardPaths::LocateDirectory);
    const QString fileName = QFileDialog::getOpenFileName(this, QString(), folderName, i18n("Kigo Game Files (*.sgf)"));
    if (!fileName.isEmpty()) {
        loadGame(fileName);
    }
}

bool MainWindow::loadGame(const QString &fileName)
{
    if (!fileName.isEmpty() && QFile(fileName).exists()) {
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

        disconnect(m_game, &Game::canRedoChanged, m_redoMoveAction, &QAction::setEnabled);
        disconnect(m_game, &Game::canUndoChanged, m_undoMoveAction, &QAction::setEnabled);
        disconnect(m_game, &Game::currentPlayerChanged, this, &MainWindow::playerChanged);

        m_gameDock->setVisible(false);
        m_gameDock->toggleViewAction()->setEnabled(false);
        m_movesDock->setVisible(false);
        m_movesDock->toggleViewAction()->setEnabled(false);
        m_setupDock->setVisible(true);
        m_errorDock->setVisible(false);

        m_setupWidget->loadedGame(fileName);
        m_gameScene->showMessage(i18n("Set up a loaded game..."));
        return true;
    } else {
        m_gameScene->showMessage(i18n("Unable to load game..."));
        //Note: New game implied here
        return false;
    }
}

void MainWindow::getMoreGames()
{
    KNS3::QtQuickDialogWrapper dialog(QStringLiteral("kigo-games.knsrc"));
    const QList<KNSCore::EntryInternal> entries = dialog.exec();
    /*KNS3::Entry::List entries = dialog.changedEntries();
    if (entries.size() > 0) {
        // do something with the modified entries here if you want
        // such as rescaning your data folder or whatnot
    }*/
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

    disconnect(m_game, &Game::canRedoChanged, m_redoMoveAction, &QAction::setEnabled);
    disconnect(m_game, &Game::canUndoChanged, m_undoMoveAction, &QAction::setEnabled);
    disconnect(m_game, &Game::currentPlayerChanged, this, &MainWindow::playerChanged);
    m_gameDock->setVisible(false);
    m_gameDock->toggleViewAction()->setEnabled(false);
    m_movesDock->setVisible(false);
    m_movesDock->toggleViewAction()->setEnabled(false);
    m_setupDock->setVisible(false);
    m_errorDock->setVisible(true);
}

void MainWindow::saveGame()
{
    const QString fileName = QFileDialog::getSaveFileName(this, QString(), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), i18n("Kigo Game Files (*.sgf)"));

    if (!fileName.isEmpty()) {
        if (m_game->save(fileName)) {
            m_gameScene->showMessage(i18n("Game saved..."));
        } else {
            m_gameScene->showMessage(i18n("Unable to save game."));
        }
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
        connect(m_game, &Game::canRedoChanged, m_redoMoveAction, &QAction::setEnabled);
        connect(m_game, &Game::canUndoChanged, m_undoMoveAction, &QAction::setEnabled);

        m_passMoveAction->setEnabled(true);
        m_hintAction->setEnabled(true);

        m_gameView->setInteractive(true);
        m_undoView->setEnabled(true);

        m_gameScene->showPlacementMarker(true);
    } else {
        m_passMoveAction->setEnabled(false);
        m_hintAction->setEnabled(false);

        m_gameView->setInteractive(false);
        m_undoView->setEnabled(false);

        m_gameScene->showPlacementMarker(false);
    }

    connect(m_game, &Game::currentPlayerChanged, this, &MainWindow::playerChanged);
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
    m_startGameAction->setEnabled(false);
    m_finishGameAction->setEnabled(false);

    m_undoView->setEnabled(false);

    //qCDebug(KIGO_LOG) << "Fetching final score from engine ...";
    Score score = m_game->estimatedScore();
    QString name;
    if (score.color() == QLatin1Char('W')) {
        name = i18n("%1 (White)", m_game->whitePlayer().name());
    } else {
        name = i18n("%1 (Black)", m_game->blackPlayer().name());
    }
    // Show a score message that stays visible until the next
    // popup message arrives
    if (score.upperBound() == score.lowerBound()) {
        m_gameScene->showMessage(i18n("%1 won with a score of %2.",
                             name, score.score()), 0);
    } else {
        m_gameScene->showMessage(i18n("%1 won with a score of %2 (bounds: %3 and %4).",
                             name, score.score(), score.upperBound(), score.lowerBound()), 0);
    }
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
    if (m_game->playMove(m_game->currentPlayer())) { // E.g. Pass move
        m_gameScene->showHint(false);
    }
}

void MainWindow::hint()
{
    m_gameScene->showHint(true);
}

void MainWindow::showPreferences()
{
    if (KConfigDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }

    KConfigDialog *dialog = new KConfigDialog(this, QStringLiteral("settings"), Preferences::self());
    dialog->addPage(new GeneralConfig(), i18n("General"), QStringLiteral("preferences-other"));
    dialog->addPage(new KGameThemeSelector(dialog, Preferences::self(), KGameThemeSelector::NewStuffDisableDownload), i18n("Themes"), QStringLiteral("games-config-theme"));
    if (QPushButton *restore = dialog->button(QDialogButtonBox::RestoreDefaults)) {
        restore->hide();
    }
    connect(dialog, &KConfigDialog::settingsChanged, this, &MainWindow::applyPreferences);
    dialog->show();
}

void MainWindow::applyPreferences()
{
    //qCDebug(KIGO_LOG) << "Update settings based on changed configuration...";
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
        QTimer::singleShot(200, this, &MainWindow::generateMove);
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
    m_newGameAction = KStandardGameAction::gameNew(this, &MainWindow::newGame, actionCollection());
    m_loadGameAction = KStandardGameAction::load(
        this, qOverload<>(&MainWindow::loadGame), actionCollection());
    m_getMoreGamesAction = new QAction(QIcon::fromTheme( QStringLiteral( "get-hot-new-stuff") ), i18nc("@action", "Get More Games..." ), this);
    actionCollection()->setDefaultShortcut(m_getMoreGamesAction, Qt::CTRL | Qt::Key_G);
    m_getMoreGamesAction->setToolTip(i18nc("@action", "Get More Games..."));
    connect(m_getMoreGamesAction, &QAction::triggered, this, &MainWindow::getMoreGames);
    actionCollection()->addAction( QStringLiteral( "get_more_games" ), m_getMoreGamesAction);
    m_saveAction = KStandardGameAction::save(this, &MainWindow::saveGame, actionCollection());
    KStandardGameAction::quit(this, &QWidget::close, actionCollection());

    m_startGameAction = new QAction(QIcon::fromTheme( QStringLiteral( "media-playback-start") ), i18nc("@action", "Start Game" ), this);
    actionCollection()->setDefaultShortcut(m_startGameAction, Qt::Key_S);
    m_startGameAction->setToolTip(i18nc("@action", "Start Game"));
    connect(m_startGameAction, &QAction::triggered, this, &MainWindow::startGame);
    actionCollection()->addAction( QStringLiteral( "game_start" ), m_startGameAction);

    m_finishGameAction = new QAction(QIcon::fromTheme( QStringLiteral( "media-playback-stop") ), i18nc("@action", "Finish Game" ), this);
    actionCollection()->setDefaultShortcut(m_finishGameAction,Qt::Key_F);
    m_finishGameAction->setToolTip(i18nc("@action", "Finish Game"));
    connect(m_finishGameAction, &QAction::triggered, this, &MainWindow::finishGame);
    actionCollection()->addAction( QStringLiteral( "game_finish" ), m_finishGameAction);

    // Move menu
    m_undoMoveAction = KStandardGameAction::undo(this, &MainWindow::undo, actionCollection());
    m_redoMoveAction = KStandardGameAction::redo(this, &MainWindow::redo, actionCollection());
    m_passMoveAction = KStandardGameAction::endTurn(this, &MainWindow::pass, actionCollection());
    m_passMoveAction->setText(i18nc("@action:inmenu Move", "Pass Move"));
    actionCollection()->setDefaultShortcut(m_passMoveAction,Qt::Key_P);
    m_hintAction = KStandardGameAction::hint(this, &MainWindow::hint, actionCollection());

    // View menu
    m_moveNumbersAction = new KToggleAction(QIcon::fromTheme( QStringLiteral( "pin") ), i18nc("@action:inmenu View", "Show Move &Numbers" ), this);
    actionCollection()->setDefaultShortcut(m_moveNumbersAction, Qt::Key_N);
    m_moveNumbersAction->setChecked(Preferences::showMoveNumbers());
    connect(m_moveNumbersAction, &KToggleAction::toggled, m_gameScene, &GameScene::showMoveNumbers);
    actionCollection()->addAction( QStringLiteral( "move_numbers" ), m_moveNumbersAction);

    // Settings menu
    KStandardAction::preferences(this, &MainWindow::showPreferences, actionCollection());
}

void MainWindow::setupDockWindows()
{
    // Setup dock
    m_setupDock = new QDockWidget(i18nc("@title:window", "Game Setup"), this);
    m_setupDock->setObjectName( QStringLiteral("setupDock" ));
    m_setupDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_setupWidget = new SetupWidget(m_game, this);
    m_setupDock->setWidget(m_setupWidget);
    connect(m_setupWidget, &SetupWidget::startClicked, this, &MainWindow::startGame);
    addDockWidget(Qt::RightDockWidgetArea, m_setupDock);

    // Game dock
    m_gameDock = new QDockWidget(i18nc("@title:window", "Information"), this);
    m_gameDock->setObjectName( QStringLiteral("gameDock" ));
    m_gameDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto gameWidget = new GameWidget(m_game, this);
    connect(gameWidget, &GameWidget::finishClicked, this, &MainWindow::finishGame);
    m_gameDock->setWidget(gameWidget);
    m_gameDock->toggleViewAction()->setText(i18nc("@title:window", "Information"));
    actionCollection()->setDefaultShortcut(m_gameDock->toggleViewAction(), Qt::Key_I);
    actionCollection()->addAction( QStringLiteral( "show_game_panel" ), m_gameDock->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea, m_gameDock);

    // Move history dock
    m_movesDock = new QDockWidget(i18nc("@title:window", "Moves"), this);
    m_movesDock->setObjectName( QStringLiteral("movesDock" ));
    m_undoView = new QUndoView(m_game->undoStack());
    m_undoView->setEmptyLabel(i18n("No move"));
    m_undoView->setAlternatingRowColors(true);
    m_undoView->setFocusPolicy(Qt::NoFocus);
    m_undoView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_undoView->setSelectionMode(QAbstractItemView::NoSelection);
    m_movesDock->setWidget(m_undoView);
    m_movesDock->toggleViewAction()->setText(i18nc("@title:window", "Moves"));
    actionCollection()->setDefaultShortcut(m_movesDock->toggleViewAction(), Qt::Key_M);
    actionCollection()->addAction( QStringLiteral( "show_moves_panel" ), m_movesDock->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea, m_movesDock);

    m_errorDock = new QDockWidget(i18nc("@title:window", "Error"), this);
    m_errorDock->setObjectName( QStringLiteral("errorDock" ));
    m_errorDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    auto errorWidget = new ErrorWidget(this);
    connect(errorWidget, &ErrorWidget::configureClicked, this, &MainWindow::showPreferences);
    m_errorDock->setWidget(errorWidget);
    addDockWidget(Qt::BottomDockWidgetArea, m_errorDock);
}

bool MainWindow::isBackendWorking()
{
    Game game;
    return game.start(Preferences::engineCommand());
}

} // End of namespace Kigo
