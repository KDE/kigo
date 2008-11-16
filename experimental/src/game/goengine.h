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

namespace Kigo {

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
        FailedToStart = 1,              //< See QProcess::ProcessError
        Crashed,                        //< See QProcess::ProcessError
        TimedOut,                       //< See QProcess::ProcessError
        WriteError,                     //< See QProcess::ProcessError
        ReadError,                      //< See QProcess::ProcessError
        UnknownError,                   //< See QProcess::ProcessError
        NotSupported,                   //< Engine does not support the command
        ArgumentMismatch                //< Wrong parameters for command used
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
    virtual ~GoEngine();

    QVariant result() const;

public slots:
    void start(const QString &command = "gnugo --mode gtp");
    void stop(StopOption options = FinishRequests);
    void send(GoEngine::Command command, const QVariantList &arguments = QVariantList());

signals:
    void started();
    void stopped();
    void error(GoEngine::Error);
    void resultReady(GoEngine::Command);

private slots:
    void dispatchRequest();
    void processResponse();
    void processError(QProcess::ProcessError);

private:
    QProcess m_process;
    QByteArray m_buffer;

    QQueue<QPair<GoEngine::Command, QByteArray> > m_requestQueue;
    quint32 m_requestNumber;
    GoEngine::Command m_pending;
    QVariant m_result;

    GoPlayer m_whitePlayer;
    GoPlayer m_blackPlayer;
    GoPlayer *m_currentPlayer;
};

} // End of namespace Kigo

#endif
