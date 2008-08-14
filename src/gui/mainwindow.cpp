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
 * @file This file is part of KGO and implements the class MainWindow,
 *       which acts as the main window of the graphical user interface
 *       for KGo.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */

#include "mainwindow.h"
#include "game/goengine.h"
#include "gui/config.h"
#include "gui/graphicsview/gamescene.h"
#include "gui/graphicsview/gameview.h"
#include "gui/screens/setupscreen.h"
#include "gui/screens/gamescreen.h"
#include "gui/screens/errorscreen.h"
#include "preferences.h"

#include <KGameThemeSelector>
#include <KStandardGameAction>
#include <KStandardAction>
#include <KToggleAction>
#include <KAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <KStatusBar>
#include <KMenuBar>
#include <KFileDialog>

#include <QStackedWidget>
#include <QDir>

namespace KGo {

MainWindow::MainWindow(QWidget *parent, bool startDemo)
    : KXmlGuiWindow(parent)
    , m_mainWidget(new QStackedWidget(this)), m_gameScene(new GameScene)
    , m_errorScreen(0), m_setupScreen(0), m_gameScreen(0)
{
    setCentralWidget(m_mainWidget);

    if (startDemo) {
        setupGUI();

        GameView *gameView = new GameView(m_gameScene, this);
        gameView->setInteractive(false);
        m_mainWidget->addWidget(gameView);
        m_mainWidget->setCurrentWidget(gameView);

        //TODO: Implement demo mode
    } else {
        setupActions();
        setupGUI();
        statusBar()->showMessage(i18n("Welcome to KGo, the KDE Go board game"), 5000);

        connect(m_gameScene, SIGNAL(statusMessage(const QString &)), statusBar(), SLOT(showMessage(const QString &)));

        if (!m_gameScene->engine()->start(Preferences::engineCommand())) {
            m_newGameAction->setEnabled(false);
            m_loadGameAction->setEnabled(false);
            m_mainWidget->setCurrentWidget(errorScreen());
            m_errorScreen->setErrorMessage(m_gameScene->engine()->lastResponse());
        } else {
            m_newGameAction->setEnabled(true);
            m_loadGameAction->setEnabled(true);
            m_mainWidget->setCurrentWidget(setupScreen());
        }
    }
}

void MainWindow::newGame()
{
    m_saveAsAction->setEnabled(false);
    m_previousMoveAction->setEnabled(false);
    m_passMoveAction->setEnabled(false);
    m_hintAction->setEnabled(false);
    m_moveHistoryAction->setEnabled(false);
    m_setupScreen->setupNewGame();
    m_mainWidget->setCurrentWidget(setupScreen());
    statusBar()->showMessage(i18n("Play a new game..."), 3000);
}

void MainWindow::loadGame()
{
    m_saveAsAction->setEnabled(false);
    m_previousMoveAction->setEnabled(true);
    m_passMoveAction->setEnabled(false);
    m_hintAction->setEnabled(false);
    m_moveHistoryAction->setEnabled(false);
    QString fileName = KFileDialog::getOpenFileName(KUrl(QDir::homePath()), "*.sgf");
    if (!fileName.isEmpty()) {
        setupScreen()->setupLoadedGame(fileName);
        m_mainWidget->setCurrentWidget(m_setupScreen);
        statusBar()->showMessage(i18n("Play a loaded game..."), 3000);
    }
}

void MainWindow::saveGame()
{
    QString fileName = KFileDialog::getSaveFileName(KUrl(QDir::homePath()), "*.sgf");
    if (!fileName.isEmpty()) {
        if (m_gameScene->engine()->saveSgf(fileName))
            statusBar()->showMessage(i18n("Game saved to %1", fileName), 3000);
        else
            statusBar()->showMessage(i18n("Unable to save game to %1!", fileName), 3000);
    }
}

void MainWindow::startGame()
{
    m_saveAsAction->setEnabled(true);
    m_previousMoveAction->setEnabled(true);
    m_passMoveAction->setEnabled(true);
    m_hintAction->setEnabled(true);
    m_moveHistoryAction->setEnabled(true);
    // The GameScene should be configured and just be waiting for further input
    // so we only need to show the GameScreen and allow direct user-interaction
    m_mainWidget->setCurrentWidget(gameScreen());
    statusBar()->showMessage(i18n("Game started..."), 3000);
}

void MainWindow::undo()
{
    m_gameScene->engine()->undoMove();
}

void MainWindow::redo()
{
}

void MainWindow::pass()
{
    m_gameScene->engine()->passMove();
}

void MainWindow::hint()
{
    m_gameScene->showHint(true);
}

void MainWindow::toggleDemoMode()
{
}

void MainWindow::showPreferences()
{
    if (KConfigDialog::showDialog("settings"))
        return;

    KConfigDialog *dialog = new KConfigDialog(this, "settings", Preferences::self());
    dialog->addPage(new GeneralConfig(), i18n("General"), "preferences-other");
    dialog->addPage(new KGameThemeSelector(dialog, Preferences::self()), i18n("Themes"), "games-config-theme");
    dialog->setHelp(QString(),"KGo");
    connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updatePreferences()));
    dialog->show();
}

void MainWindow::updatePreferences()
{
    kDebug() << "Update settings based on changed configuration...";
    m_gameScene->showLabels(Preferences::showBoardLabels());

    // Restart the Go engine if the engine command was changed by the user.
    GoEngine *engine = m_gameScene->engine();
    if (engine->command() != Preferences::engineCommand()) {
        kDebug() << "Engine command changed or engine not running, (re)start engine...";
        if (!m_gameScene->engine()->start(Preferences::engineCommand())) {
            m_newGameAction->setEnabled(false);
            m_loadGameAction->setEnabled(false);
            m_mainWidget->setCurrentWidget(errorScreen());
            m_errorScreen->setErrorMessage(m_gameScene->engine()->lastResponse());
        } else {
            m_newGameAction->setEnabled(true);
            m_loadGameAction->setEnabled(true);
            m_mainWidget->setCurrentWidget(setupScreen());
        }
    }
}

ErrorScreen *MainWindow::errorScreen()
{
    if (!m_errorScreen) {
        m_errorScreen = new ErrorScreen;
        connect(m_errorScreen, SIGNAL(configureButtonClicked()), this, SLOT(showPreferences()));
        connect(m_errorScreen, SIGNAL(quitButtonClicked()), this, SLOT(close()));
        m_mainWidget->addWidget(m_errorScreen);
    }
    return m_errorScreen;
}

SetupScreen *MainWindow::setupScreen()
{
    if (!m_setupScreen) {
        m_setupScreen = new SetupScreen(m_gameScene);
        connect(m_setupScreen, SIGNAL(startClicked()), this, SLOT(startGame()));
        m_mainWidget->addWidget(m_setupScreen);
    }
    return m_setupScreen;
}

GameScreen *MainWindow::gameScreen()
{
    if (!m_gameScreen) {
        m_gameScreen = new GameScreen(m_gameScene);
        m_mainWidget->addWidget(m_gameScreen);
    }
    return m_gameScreen;
}

void MainWindow::setupActions()
{
    // Game menu
    m_newGameAction = KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    m_newGameAction->setEnabled(false);
    m_loadGameAction = KStandardGameAction::load(this, SLOT(loadGame()), actionCollection());
    m_loadGameAction->setEnabled(false);
    m_saveAsAction = KStandardGameAction::saveAs(this, SLOT(saveGame()), actionCollection());
    m_saveAsAction->setEnabled(false);
    m_editGameAction = new KAction(KIcon("games-config-board"), i18n("&Edit game"), this);
    m_editGameAction->setShortcut(i18n("Ctrl+E"));
    m_editGameAction->setEnabled(false);
    actionCollection()->addAction("game_edit", m_editGameAction);
    //TODO: Connect edit game action with board editor
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    // Move menu
    m_previousMoveAction = KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
    m_previousMoveAction->setEnabled(false);
    m_nextMoveAction = KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
    m_nextMoveAction->setEnabled(false);
    m_passMoveAction = KStandardGameAction::endTurn(this, SLOT(pass()), actionCollection());
    m_passMoveAction->setText(i18n("Pass move"));
    m_passMoveAction->setEnabled(false);
    m_hintAction = KStandardGameAction::hint(this, SLOT(hint()), actionCollection());
    m_hintAction->setEnabled(false);

    // View menu
    m_moveHistoryAction = new KToggleAction(KIcon("lastmoves"), i18n("&Move history"), this);
    m_moveHistoryAction->setShortcut(i18n("M"));
    m_moveHistoryAction->setEnabled(false);
    m_moveHistoryAction->setChecked(Preferences::showMoveHistory());
    actionCollection()->addAction("move_history", m_moveHistoryAction);
    connect(m_moveHistoryAction, SIGNAL(toggled(bool)), m_gameScene, SLOT(showMoveHistory(bool)));

    // Settings menu
    KStandardAction::preferences(this, SLOT(showPreferences()), actionCollection());
}

} // End of namespace KGo

#include "moc_mainwindow.cpp"
