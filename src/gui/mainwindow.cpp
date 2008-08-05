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
#include <KFileDialog>

#include <QStackedWidget>
#include <QDir>

namespace KGo {

MainWindow::MainWindow(QWidget *parent, bool startDemo)
    : KXmlGuiWindow(parent)
    , m_mainWidget(new QStackedWidget(this)), m_gameScene(new GameScene)
    , m_errorScreen(0), m_setupScreen(0), m_gameScreen(0)
    , m_startInDemoMode(startDemo)
{
    setCentralWidget(m_mainWidget);
    setupActions();
    setupGUI();
    statusBar()->showMessage(i18n("Welcome to KGo"), 3000);

    connect(m_gameScene, SIGNAL(statusMessage(const QString &)), statusBar(), SLOT(showMessage(const QString &)));

    if (!m_gameScene->engine()->run(Preferences::engineCommand())) {
        m_newGameAction->setEnabled(false);
        m_loadGameAction->setEnabled(false);
        m_mainWidget->setCurrentWidget(errorScreen());
        m_errorScreen->setErrorMessage(m_gameScene->engine()->response());
    } else {
        m_newGameAction->setEnabled(true);
        m_loadGameAction->setEnabled(true);
        m_mainWidget->setCurrentWidget(setupScreen());
    }
}

void MainWindow::newGame()
{
    m_saveAsAction->setEnabled(false);
    m_passAction->setEnabled(false);
    m_setupScreen->setupNewGame();
    m_mainWidget->setCurrentWidget(setupScreen());
    statusBar()->showMessage(i18n("Play a new game..."), 3000);
}

void MainWindow::loadGame()
{
    m_saveAsAction->setEnabled(false);
    m_passAction->setEnabled(false);
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
    m_passAction->setEnabled(true);
    // The GameScene should be configured and just be waiting for further input
    // so we only need to show the GameScreen and allow direct user-interaction
    m_mainWidget->setCurrentWidget(gameScreen());
    statusBar()->showMessage(i18n("Game started..."), 3000);
}

void MainWindow::undo()
{
    kDebug() << "Redo";
}

void MainWindow::redo()
{
    kDebug() << "Undo";
}

void MainWindow::pass()
{
    kDebug() << "End turn";
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
    if (engine->engineCommand() != Preferences::engineCommand()) {
        kDebug() << "Engine command changed or engine not running, (re)start engine...";
        if (!m_gameScene->engine()->run(Preferences::engineCommand())) {
            m_newGameAction->setEnabled(false);
            m_loadGameAction->setEnabled(false);
            m_mainWidget->setCurrentWidget(errorScreen());
            m_errorScreen->setErrorMessage(m_gameScene->engine()->response());
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
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    // Move menu
    m_previousMoveAction = KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
    m_previousMoveAction->setEnabled(false);
    m_nextMoveAction = KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
    m_nextMoveAction->setEnabled(false);
    m_passAction = KStandardGameAction::endTurn(this, SLOT(pass()), actionCollection());
    m_passAction->setText(i18n("Pass move"));
    m_passAction->setEnabled(false);
    m_demoAction = KStandardGameAction::demo(this, SLOT(toggleDemoMode()), actionCollection());
    m_demoAction->setEnabled(false);

    // Settings menu
    KStandardAction::preferences(this, SLOT(showPreferences()), actionCollection());
}

} // End of namespace KGo

#include "moc_mainwindow.cpp"
