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
 * @file This file is part of KGO and defines the class MainWindow,
 *       which acts as the main window of the graphical user interface
 *       for KGo.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_MAINWINDOW_H
#define KGO_MAINWINDOW_H

#include <KXmlGuiWindow>

class QAction;
class QStackedWidget;
class KToggleAction;

namespace KGo {

class Gtp;
class SetupScreen;
class GameScreen;

/**
 * The MainWindow class acts as the main window of the KGo graphical
 * user interface.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    /**
     * Standard Constructor
     *
     * @param parent The parent widget
     * @param startDemo Start in demo mode?
     */
    MainWindow(QWidget *parent = 0, bool startDemo = false);

    /**
     *
     */
    ~MainWindow();

private slots:
    void newGame();                     ///< Configure new game with setupscreen
    void loadGame();                    ///< Configure loaded game with ~
    void saveGame();                    ///< Save the current game state
    void startGame();                   ///< React on setupscreen start button
    void undo();                        ///<
    void redo();                        ///<
    void toggleDemoMode();              ///<
    void showPreferences();             ///< Show settings dialog

private:
    void setupActions();

    Gtp* m_gtp;                         ///<
    SetupScreen *m_setupScreen;         ///< Pointer to the game setup screen
    GameScreen *m_gameScreen;           ///< Pointer to the game playing screen
    bool m_startInDemoMode;             ///< Start application in demo mode

    QAction *m_saveAsAction;            ///< Action to save the current game
    QAction *m_firstMoveAction;         ///< Action to jump to the first move
    QAction *m_previousMoveAction;      ///< Action to jump to the previous move
    QAction *m_nextMoveAction;          ///< Action to jump to the next move
    QAction *m_lastMoveAction;          ///< Action to jump to the last move
    KToggleAction *m_demoAction;        ///< Action to change to demo mode
};

} // End of namespace KGo

#endif
