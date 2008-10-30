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
#include "game/oldgoengine.h"
#include "gui/config.h"
#include "gui/graphicsview/gamescene.h"
#include "gui/graphicsview/gameview.h"
#include "gui/screens/setupscreen.h"
#include "gui/screens/gamescreen.h"
#include "gui/screens/messagescreen.h"
#include "preferences.h"

#include <KGameThemeSelector>
#include <KStandardGameAction>
#include <KStandardAction>
#include <KToggleAction>
#include <KAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <KMenuBar>
#include <KFileDialog>

#include <QStackedWidget>
#include <QDir>

namespace KGo {

MainWindow::MainWindow(QWidget *parent, bool startDemo)
    : KXmlGuiWindow(parent)
    , m_mainWidget(new QStackedWidget(this)), m_gameScene(new GameScene)
    , m_messageScreen(0), m_setupScreen(0), m_gameScreen(0)
{
    setCentralWidget(m_mainWidget);
    if (startDemo) {
        setupGUI(KXmlGuiWindow::Save);

        GameView *gameView = new GameView(m_gameScene, this);
        gameView->setInteractive(false);
        m_mainWidget->addWidget(gameView);
        m_mainWidget->setCurrentWidget(gameView);

        //TODO: Implement demo mode
    } else {
        setupActions();
        setupGUI(KXmlGuiWindow::ToolBar | KXmlGuiWindow::Keys | KXmlGuiWindow::Save | KXmlGuiWindow::Create);

        if (!m_gameScene->engine()->startEngine(Preferences::engineCommand())) {
            m_newGameAction->setEnabled(false);
            m_loadGameAction->setEnabled(false);
            m_mainWidget->setCurrentWidget(messageScreen());
        } else {
            m_newGameAction->setEnabled(true);
            m_loadGameAction->setEnabled(true);
            newGame();
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

    setupScreen()->setupNewGame();

    m_mainWidget->setCurrentWidget(setupScreen());
    m_gameScene->showPopupMessage(i18n("Set up and play a new game..."));
}

void MainWindow::loadGame()
{
    QString fileName = KFileDialog::getOpenFileName(KUrl(QDir::homePath()), "*.sgf");
    if (!fileName.isEmpty()) {
        m_saveAsAction->setEnabled(false);
        m_previousMoveAction->setEnabled(false);
        m_passMoveAction->setEnabled(false);
        m_hintAction->setEnabled(false);
        m_moveHistoryAction->setEnabled(true);

        setupScreen()->setupLoadedGame(fileName);

        m_mainWidget->setCurrentWidget(m_setupScreen);
        m_gameScene->showPopupMessage(i18n("Set up and play a loaded game..."));
    }
}

void MainWindow::saveGame()
{
    QString fileName = KFileDialog::getSaveFileName(KUrl(QDir::homePath()), "*.sgf");
    if (!fileName.isEmpty()) {
        if (m_gameScene->engine()->saveGameToSGF(fileName))
            m_gameScene->showPopupMessage(i18n("Game saved to %1", fileName));
        else
            m_gameScene->showPopupMessage(i18n("Unable to save game to %1!", fileName));
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
    m_gameScene->showPopupMessage(i18n("Game started..."));
}

void MainWindow::undo()
{
    m_gameScene->engine()->undoMove();
    m_gameScene->showPopupMessage("");
}

void MainWindow::redo()
{
    //TODO: Implement redo stuff
}

void MainWindow::pass()
{
    m_gameScene->engine()->passMove();
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
    OldGoEngine *engine = m_gameScene->engine();
    if (engine->engineCommand() != Preferences::engineCommand()) {
        kDebug() << "Engine command changed or engine not running, (re)start backend...";
        if (!m_gameScene->engine()->startEngine(Preferences::engineCommand())) {
            m_newGameAction->setEnabled(false);
            m_loadGameAction->setEnabled(false);
            m_mainWidget->setCurrentWidget(messageScreen());
        } else {
            m_newGameAction->setEnabled(true);
            m_loadGameAction->setEnabled(true);
            m_mainWidget->setCurrentWidget(setupScreen());
            m_gameScene->showPopupMessage(i18n("Restarted Go backend..."));
        }
    }
}

MessageScreen *MainWindow::messageScreen()
{
    if (!m_messageScreen) {
        m_messageScreen = new MessageScreen;
        connect(m_messageScreen, SIGNAL(configureButtonClicked()), this, SLOT(showPreferences()));
        connect(m_messageScreen, SIGNAL(quitButtonClicked()), this, SLOT(close()));
        m_mainWidget->addWidget(m_messageScreen);
    }
    return m_messageScreen;
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
