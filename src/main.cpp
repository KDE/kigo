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

#include "gui/mainwindow.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KDebug>
#include <KApplication>

/**
 * This namespace collects all classes related to KGo, the Go board game.
 */
namespace KGo { /* This is only a Doxygen stub */ }

/**
 * The standard C/C++ program entry point. Application 'about' data
 * is created here and command line options are configured and checked
 * if the user invokes the application. If everything is set up, the
 * method displays the main window and jumps into the Qt event loop.
 */
int main(int argc, char *argv[])
{
    KAboutData aboutData("kgo", 0, ki18n("KGo"), "v0.3",
            ki18n("KDE Go Board Game"),    KAboutData::License_GPL_V3,
            ki18n("Copyright (c) 2008 Sascha Peilicke"));
    aboutData.addAuthor(ki18n("Sascha Peilicke"), ki18n("Original author"), "sasch.pe@gmx.de");
    //aboutData.setHomepage("http://games.kde.org/kgo");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("demo", ki18n("Start with a demo game playing"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
    KGlobal::locale()->insertCatalog("libkdegames");

    if (app.isSessionRestored()) {
        kDebug() << "Restore last session";
        RESTORE(KGo::MainWindow)
    } else {
        kDebug() << "Start new session";
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        KGo::MainWindow *mainWin = new KGo::MainWindow(0, args->isSet("demo"));
        mainWin->show();
        args->clear();
    }
    return app.exec();
}
