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

#include "goengine.h"

namespace KGo {

GoEngine::GoEngine()
    : m_whitePlayer(GoPlayer::White, GoPlayer::Human)
    , m_blackPlayer(GoPlayer::Black, GoPlayer::Human)
    , m_currentPlayer(&m_blackPlayer)
{
}

GoEngine::~GoEngine()
{
    stop(IgnoreRequests);
}

void GoEngine::start(const QString &command)
{
    stop(IgnoreRequests);
    m_replyBuffer.clear();
    m_commandQueue.clear();
    m_resultQueue.clear();

    m_engineProcess.start(command.toLatin1());
    if (!m_engineProcess.waitForStarted()) {        // Blocking wait for process start
        emit error(GoEngine::EngineNotResponding);
        return;
    }

    // Test if we started a GTP-compatible Go engine
    m_engineProcess.write("name\n");
    m_engineProcess.waitForReadyRead();
    QString response = m_engineProcess.readAllStandardOutput();
    if (response.isEmpty() || !response.startsWith("=")) {
        emit error(GoEngine::EngineNotSupported);
        stop(IgnoreRequests);
        return;
    }

    emit started();
}

void GoEngine::stop(StopOption op)
{
    if (m_engineProcess.isOpen()) {
        if (op == FinishRequests) {
            // TODO: Wait till last command is processed
        }
        m_engineProcess.write("quit\n");
        m_engineProcess.close();
        emit stopped();
    }
}

bool GoEngine::send(GoEngine::Command command, const QVariantList &arguments)
{
    QString commandString;

    switch (command) {
        case GetEngineName: commandString = "name\n"; break;
        case GetEngineVersion: commandString = "version\n"; break;
        case GetEngineProtocolVersion: commandString = "protocol_version\n"; break;

        case LoadGameFromSGF:
            if (arguments.size() != 2  || arguments[0].type() != QVariant::String || arguments[1].type() != QVariant::Int) {
                emit error(GoEngine::CommandArgumentMismatch);
                return false;
            }
            commandString = "loadsgf " + arguments[0].toString() + ' ' + arguments[1].toInt() + '\n';
            break;
        case SaveGameToSGF:
            if (arguments.size() != 1 || arguments[0].type() != QVariant::String) {
                emit error(GoEngine::CommandArgumentMismatch);
                return false;
            }
            commandString = "printsgf " + arguments[0].toString() + '\n';
            break;

        case GetPlayerType:
        case SetPlayerType:
        case GetPlayerName:
        case SetPlayerName:
        case GetPlayerStrength:
        case SetPlayerStrength:
        case GetCurrentPlayer:
            emit error(GoEngine::CommandNotSupported); return false;

        case GetBoardSize: commandString = "query_boardsize\n"; break;
        case SetBoardSize:
            if (arguments.size() != 1 || arguments[0].type() != QVariant::Int) {
                emit error(GoEngine::CommandArgumentMismatch);
                return false;
            }
            commandString = "board_size " + arguments[0].toByteArray() + '\n';
            break;

        case GetKomi:
        case SetKomi:
        case GetFixedHandicap:
        case SetFixedHandicap:
            emit error(GoEngine::CommandNotSupported); return false;

        default: emit error(GoEngine::CommandNotSupported); return false;
    }

    m_commandQueue.enqueue(QPair<GoEngine::Command, QString>(command, commandString));
    return true;
}

QVariant GoEngine::nextResult()
{
    //TODO: Implement
    return QVariant();
}

} // End of namespace KGo

#include "moc_goengine.cpp"
