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

#ifndef KGO_GOENGINE_H
#define KGO_GOENGINE_H

#include "goplayer.h"

#include <QPair>
#include <QQueue>
#include <QByteArray>
#include <QProcess>
#include <QVariant>

namespace KGo {

/**
 * The GoEngine class implements the Go game and acts as a wrapper around a
 * remote Go Game engine implementing the Go Text Protocol (GTP). It uses
 * GTP protocol version 2 and interfaces the engine executable in an
 * asynchronous manor. The best supported engine should (naturally)
 * be GnuGo.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.3
 */
class GoEngine : public QObject
{
    Q_OBJECT

public:
    enum Error {
        EngineNotResponding = 1,        //< Engine does not exist
        EngineNotSupported,             //< Engine does not support GTP

        CommandArgumentMismatch,        //< Wrong parameters used
        CommandNotSupported             //< Command unknown to engine
    };

    enum StopOption {
        FinishRequests = 1,             //< Process all pending commands
        IgnoreRequests                  //< Hard stop, ignore commands
    };

    enum Command {
        GetEngineName = 1,
        GetEngineVersion,
        GetEngineProtocolVersion,

        LoadGameFromSGF,
        SaveGameToSGF,

        GetPlayerType,                  //< Param 'color'
        SetPlayerType,                  //< Param 'color', 'type'
        GetPlayerName,                  //< Param 'color'
        SetPlayerName,                  //< Param 'color', 'name'
        GetPlayerStrength,              //< Param 'color'
        SetPlayerStrength,              //< Param 'color', 'strength'
        GetCurrentPlayer,               //<

        GetBoardSize,
        SetBoardSize,
        GetKomi,
        SetKomi,
        GetFixedHandicap,
        SetFixedHandicap
    };

    GoEngine();
    ~GoEngine();

public slots:
    void start(const QString &command = "gnugo --mode gtp");
    void stop(StopOption op = FinishRequests);
    bool send(GoEngine::Command command, const QVariantList &arguments = QVariantList());
    QVariant nextResult();

signals:
    void started();
    void stopped();
    void error(GoEngine::Error);
    void resultReady(GoEngine::Command);

private:
    GoPlayer m_whitePlayer;
    GoPlayer m_blackPlayer;
    GoPlayer *m_currentPlayer;

    QProcess m_engineProcess;
    QByteArray m_replyBuffer;
    QQueue<QPair<GoEngine::Command, QString> > m_commandQueue;
    QQueue<QPair<GoEngine::Command, QVariant> > m_resultQueue;
};

} // End of namespace KGo

#endif
