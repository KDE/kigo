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
#include "preferences.h"
#include "game/goengine.h"
#include "gui/gamescene.h"
#include "gui/setupscreen.h"
#include "gui/gamescreen.h"
#include "gui/config.h"

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
    , m_gameScene(new GameScene)
    , m_setupScreen(new SetupScreen(m_gameScene, this))
    , m_gameScreen(new GameScreen(m_gameScene, this))
    , m_startInDemoMode(startDemo)
{
    // Handle the start game whish the user entered in the setup screen
    connect(m_setupScreen, SIGNAL(startClicked()), this, SLOT(startGame()));

    QStackedWidget *mainWidget = new QStackedWidget;
    mainWidget->addWidget(m_setupScreen);
    mainWidget->addWidget(m_gameScreen);
    mainWidget->setCurrentWidget(m_setupScreen);
    setCentralWidget(mainWidget);

    setupActions();
    setupGUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupActions()
{
    // Game menu
    KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    KStandardGameAction::load(this, SLOT(loadGame()), actionCollection());
    m_saveAsAction = KStandardGameAction::saveAs(this, SLOT(saveGame()), actionCollection());
    m_saveAsAction->setEnabled(false);
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    // Move menu
    //TODO: Integrate move forward/backward stuff
    m_previousMoveAction = KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
    m_previousMoveAction->setEnabled(false);
    m_nextMoveAction = KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
    m_nextMoveAction->setEnabled(false);
    m_endTurnAction = KStandardGameAction::endTurn(this, SLOT(endTurn()), actionCollection());
    m_endTurnAction->setEnabled(false);
    m_demoAction = KStandardGameAction::demo(this, SLOT(toggleDemoMode()), actionCollection());
    m_demoAction->setEnabled(false);

    // Settings menu
    KStandardAction::preferences(this, SLOT(showPreferences()), actionCollection());
}

void MainWindow::newGame()
{
    m_saveAsAction->setEnabled(false);
    m_endTurnAction->setEnabled(false);
    m_setupScreen->setupNewGame();
    qobject_cast<QStackedWidget *>(centralWidget())->setCurrentWidget(m_setupScreen);
}

void MainWindow::loadGame()
{
    m_saveAsAction->setEnabled(false);
    m_endTurnAction->setEnabled(false);
    QString fileName = KFileDialog::getOpenFileName(KUrl(QDir::homePath()), "*.sgf");
    if (!fileName.isEmpty()) {
        m_setupScreen->setupLoadedGame(fileName, true);
        qobject_cast<QStackedWidget *>(centralWidget())->setCurrentWidget(m_setupScreen);
    }
}

void MainWindow::saveGame()
{
    QString fileName = KFileDialog::getSaveFileName(KUrl(QDir::homePath()), "*.sgf");
    if (!fileName.isEmpty())
        m_gameScene->engine()->saveSgf(fileName);
}

void MainWindow::startGame()
{
    m_saveAsAction->setEnabled(true);
    m_endTurnAction->setEnabled(true);
    //NOTE: The GameScene should be configured and just be waiting for further input
    //      so we only need to show the GameScreen and allow direct user-interaction
    qobject_cast<QStackedWidget *>(centralWidget())->setCurrentWidget(m_gameScreen);
}

void MainWindow::undo()
{
    kDebug() << "Redo";
}

void MainWindow::redo()
{
    kDebug() << "Undo";
}

void MainWindow::endTurn()
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
    kDebug() <<"Update settings based on changed configuration";
    m_gameScene->showLabels(Preferences::showBoardLabels());
}

} // End of namespace KGo

#include "moc_mainwindow.cpp"
