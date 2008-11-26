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

#include "mainwindow.h"
#include "game/goengine.h"
#include "gui/config/generalconfig.h"
#include "gui/graphicsview/gamescene.h"
#include "gui/graphicsview/gameview.h"
#include "gui/widgets/gamewidget.h"
#include "gui/widgets/setupwidget.h"
#include "gui/widgets/editwidget.h"
#include "preferences.h"

#include <KAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <KFileDialog>
#include <KGameThemeSelector>
#include <KMenuBar>
#include <KStandardGameAction>
#include <KStandardAction>
#include <KToggleAction>
#include <KToolBar>

#include <QDir>
#include <QDockWidget>
#include <QUndoView>

namespace Kigo {

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent), m_engine(new GoEngine)
    , m_gameScene(new GameScene(m_engine)), m_gameView(new GameView(m_gameScene, this))
{
    setCentralWidget(m_gameView);

    setupActions();
    setupDockWindows();
    setupGUI(KXmlGuiWindow::ToolBar | KXmlGuiWindow::Keys |
             KXmlGuiWindow::Save | KXmlGuiWindow::Create);

    connect(m_engine, SIGNAL(waiting(bool)), this, SLOT(showBusy(bool)));
    connect(m_engine, SIGNAL(canRedoChanged(bool)),
            m_redoMoveAction, SLOT(setEnabled(bool)));
    connect(m_engine, SIGNAL(canUndoChanged(bool)),
            m_undoMoveAction, SLOT(setEnabled(bool)));
    connect(m_engine, SIGNAL(consecutivePassMovesPlayed(int)), this, SLOT(showFinish()));
    connect(m_engine, SIGNAL(playerResigned(GoEngine::PlayerColor)), this, SLOT(finishGame()));

    if (!m_engine->startEngine(Preferences::engineCommand())) {
        m_newGameAction->setEnabled(false);
        m_loadGameAction->setEnabled(false);
        m_editorAction->setEnabled(false);
        //TODO: Show error message
    } else {
        m_newGameAction->setEnabled(true);
        m_loadGameAction->setEnabled(true);
        m_editorAction->setEnabled(true);
        newGame();
    }
}

MainWindow::~MainWindow()
{
    delete m_engine;
}

void MainWindow::newGame()
{
    m_saveAction->setEnabled(false);
    m_undoMoveAction->setEnabled(false);
    m_redoMoveAction->setEnabled(false);
    m_passMoveAction->setEnabled(false);
    m_hintAction->setEnabled(false);
    m_moveNumbersAction->setEnabled(false);
    m_startGameAction->setVisible(true);
    m_finishGameAction->setVisible(false);

    m_gameView->setInteractive(false);

    m_gameDock->setVisible(false);
    m_gameDock->toggleViewAction()->setEnabled(false);
    m_movesDock->setVisible(false);
    m_movesDock->toggleViewAction()->setEnabled(false);
    m_editDock->setVisible(false);
    m_setupDock->setVisible(true);

    m_setupWidget->newGame();
    m_gameScene->showPopupMessage(i18n("Setup a new game..."));
}

void MainWindow::loadGame()
{
    QString fileName = KFileDialog::getOpenFileName(KUrl(QDir::homePath()), "*.sgf");
    if (!fileName.isEmpty()) {
        m_saveAction->setEnabled(false);
        m_undoMoveAction->setEnabled(false);
        m_redoMoveAction->setEnabled(false);
        m_passMoveAction->setEnabled(false);
        m_hintAction->setEnabled(false);
        m_moveNumbersAction->setEnabled(true);
        m_startGameAction->setVisible(true);
        m_finishGameAction->setVisible(false);

        m_gameView->setInteractive(false);

        m_gameDock->setVisible(false);
        m_gameDock->toggleViewAction()->setEnabled(false);
        m_movesDock->setVisible(false);
        m_movesDock->toggleViewAction()->setEnabled(false);
        m_editDock->setVisible(false);
        m_setupDock->setVisible(true);

        m_setupWidget->loadedGame(fileName);
        m_gameScene->showPopupMessage(i18n("Setup a loaded game..."));
    } else {
        m_gameScene->showPopupMessage(i18n("Unable to load game..."));
        //Note: New game implied here
    }
}

void MainWindow::editGame()
{
    m_saveAction->setEnabled(true);
    m_undoMoveAction->setEnabled(false);
    m_redoMoveAction->setEnabled(false);
    m_passMoveAction->setEnabled(false);
    m_hintAction->setEnabled(false);
    m_moveNumbersAction->setEnabled(true);

    m_startGameAction->setVisible(false);
    m_finishGameAction->setVisible(false);

    m_gameView->setInteractive(true);

    m_setupDock->setVisible(false);
    m_gameDock->setVisible(false);
    m_gameDock->toggleViewAction()->setEnabled(false);
    m_movesDock->setVisible(false);
    m_movesDock->toggleViewAction()->setEnabled(false);
    m_editDock->setVisible(true);

    kDebug() << "TODO: Implement edit mode";

    m_gameScene->showPopupMessage(i18n("Editor started..."));
}

void MainWindow::saveGame()
{
    QString fileName = KFileDialog::getSaveFileName(KUrl(QDir::homePath()), "*.sgf");

    //TODO: Don't forgot to commit all set values back to go engine so that
    //      they are included in the savegame!

    if (!fileName.isEmpty()) {
        if (m_engine->saveGameToSGF(fileName))
            m_gameScene->showPopupMessage(i18n("Game saved..."));
        else
            m_gameScene->showPopupMessage(i18n("Unable to save game!"));
    }
}

void MainWindow::startGame()
{
    m_saveAction->setEnabled(true);
    m_undoMoveAction->setEnabled(true);
    m_passMoveAction->setEnabled(true);
    m_hintAction->setEnabled(true);
    m_moveNumbersAction->setEnabled(true);
    m_startGameAction->setVisible(false);
    m_finishGameAction->setVisible(false);

    m_setupWidget->commit();
    m_gameView->setInteractive(true);

    m_setupDock->setVisible(false);
    m_editDock->setVisible(false);
    m_gameDock->setVisible(true);
    m_gameDock->toggleViewAction()->setEnabled(true);
    m_movesDock->setVisible(true);
    m_movesDock->toggleViewAction()->setEnabled(true);

    m_gameScene->showPopupMessage(i18n("Game started..."));
}

void MainWindow::finishGame()
{
    m_undoMoveAction->setEnabled(false);
    m_passMoveAction->setEnabled(false);
    m_hintAction->setEnabled(false);
    m_moveNumbersAction->setEnabled(false);
    m_startGameAction->setVisible(false);
    m_finishGameAction->setVisible(false);
    m_gameView->setInteractive(false);

    kDebug() << "TODO: Implement finishing games";
}

void MainWindow::undo()
{
    if (m_engine->undoMove()) {
        m_gameScene->showPopupMessage("Undone move");
    }
}

void MainWindow::redo()
{
    if (m_engine->redoMove()) {
        m_gameScene->showPopupMessage("Redone move");
    }
}

void MainWindow::pass()
{
    if (m_engine->passMove()) {
        m_gameScene->showPopupMessage("Passed move");
    }
}

void MainWindow::hint()
{
    m_gameScene->showHint(true);
}

void MainWindow::showPreferences()
{
    if (KConfigDialog::showDialog("settings"))
        return;

    KConfigDialog *dialog = new KConfigDialog(this, "settings", Preferences::self());
    dialog->addPage(new GeneralConfig(), i18n("General"), "preferences-other");
    dialog->addPage(new KGameThemeSelector(dialog, Preferences::self()), i18n("Themes"),
                    "games-config-theme");
    dialog->setHelp(QString(),"Kigo");
    connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updatePreferences()));
    dialog->show();
}

void MainWindow::updatePreferences()
{
    kDebug() << "Update settings based on changed configuration...";
    m_gameScene->showLabels(Preferences::showBoardLabels());

    // Restart the Go engine if the engine command was changed by the user.
    if (m_engine->engineCommand() != Preferences::engineCommand()) {
        kDebug() << "Engine command changed or engine not running, (re)start backend...";
        if (!m_engine->startEngine(Preferences::engineCommand())) {
            m_newGameAction->setEnabled(false);
            m_loadGameAction->setEnabled(false);
            m_editorAction->setEnabled(false);
            //TODO: Show error message
        } else {
            m_newGameAction->setEnabled(true);
            m_loadGameAction->setEnabled(true);
            m_editorAction->setEnabled(true);
            newGame();
        }
    }
}

void MainWindow::showBusy(bool busy)
{
    menuBar()->setDisabled(busy);
    toolBar()->setDisabled(busy);                       // mainToolBar
    toolBar("moveToolBar")->setDisabled(busy);
    toolBar("gameToolBar")->setDisabled(busy);
    if (busy)
        m_gameScene->showPopupMessage(i18n("The computer is thinking..."), 0);
}

void MainWindow::showFinish()
{
    m_finishGameAction->setVisible(true);
}

void MainWindow::setupActions()
{
    // Game menu
    m_newGameAction = KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    m_loadGameAction = KStandardGameAction::load(this, SLOT(loadGame()), actionCollection());
    m_saveAction = KStandardGameAction::save(this, SLOT(saveGame()), actionCollection());
    m_editorAction = new KAction(KIcon("games-config-board"), i18nc("@action:inmenu Game", "&Editor"), this);
    m_editorAction->setShortcut(Qt::CTRL | Qt::Key_E);
    connect(m_editorAction, SIGNAL(triggered(bool)), this, SLOT(editGame()));
    actionCollection()->addAction("game_edit", m_editorAction);
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    m_startGameAction = new KAction(KIcon("media-playback-start"), i18nc("@action", "Start game"), this);
    m_startGameAction->setShortcut(Qt::Key_S);
    connect(m_startGameAction, SIGNAL(triggered(bool)), this, SLOT(startGame()));
    actionCollection()->addAction("game_start", m_startGameAction);

    m_finishGameAction = new KAction(KIcon("media-playback-stop"), i18nc("@action", "Finish game"), this);
    m_finishGameAction->setShortcut(Qt::Key_F);
    connect(m_finishGameAction, SIGNAL(triggered(bool)), this, SLOT(finishGame()));
    actionCollection()->addAction("game_finish", m_finishGameAction);

    // Move menu
    m_undoMoveAction = KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
    m_redoMoveAction = KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
    m_passMoveAction = KStandardGameAction::endTurn(this, SLOT(pass()), actionCollection());
    m_passMoveAction->setText(i18nc("@action:inmenu Move", "Pass move"));
    m_passMoveAction->setShortcut(Qt::Key_P);
    m_hintAction = KStandardGameAction::hint(this, SLOT(hint()), actionCollection());

    // View menu
    m_moveNumbersAction = new KToggleAction(KIcon("lastmoves"), i18nc("@action:inmenu View", "Show move &numbers"), this);
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
    m_setupDock = new QDockWidget(i18nc("@title:window", "Game setup"), this);
    m_setupDock->setObjectName("setupDock");
    m_setupDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_setupWidget = new SetupWidget(m_engine, this);
    m_setupDock->setWidget(m_setupWidget);
    //m_setupDock->toggleViewAction()->setText(i18nc("@title:window", "Game setup"));
    //m_setupDock->toggleViewAction()->setShortcut(Qt::Key_S);
    //actionCollection()->addAction("show_setup_panel", m_setupDock->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea, m_setupDock);

    // Editor dock
    m_editDock = new QDockWidget(i18nc("@title:window", "Editor"), this);
    m_editDock->setObjectName("editDock");
    m_editDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_editDock->setWidget(new EditWidget(m_engine, this));
    //m_editDock->toggleViewAction()->setText(i18nc("@title:window", "Editor"));
    //m_editDock->toggleViewAction()->setShortcut(Qt::Key_E);
    //actionCollection()->addAction("show_edit_panel", m_editDock->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea, m_editDock);

    // Game dock
    m_gameDock = new QDockWidget(i18nc("@title:window", "Game information"), this);
    m_gameDock->setObjectName("gameDock");
    m_gameDock->setWidget(new GameWidget(m_engine, this));
    m_gameDock->toggleViewAction()->setText(i18nc("@title:window", "Game information"));
    m_gameDock->toggleViewAction()->setShortcut(Qt::Key_G);
    actionCollection()->addAction("show_game_panel", m_gameDock->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea, m_gameDock);

    // Move history dock
    m_movesDock = new QDockWidget(i18nc("@title:window", "Move history"), this);
    m_movesDock->setObjectName("movesDock");
    QUndoView *undoView = new QUndoView(m_engine->undoStack());
    undoView->setEmptyLabel(i18n("No move"));
    undoView->setEnabled(false);
    m_movesDock->setWidget(undoView);
    m_movesDock->toggleViewAction()->setText(i18nc("@title:window", "Move history"));
    m_movesDock->toggleViewAction()->setShortcut(Qt::Key_M);
    actionCollection()->addAction("show_moves_panel", m_movesDock->toggleViewAction());
    addDockWidget(Qt::RightDockWidgetArea, m_movesDock);
}

} // End of namespace Kigo

#include "moc_mainwindow.cpp"
