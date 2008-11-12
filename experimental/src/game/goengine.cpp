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

#include <KDebug>

namespace KGo {

GoEngine::GoEngine()
    : m_whitePlayer(GoPlayer::White, GoPlayer::Human)
    , m_blackPlayer(GoPlayer::Black, GoPlayer::Human)
    , m_currentPlayer(&m_blackPlayer)
{
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    connect(&m_process, SIGNAL(readyRead()), this, SLOT(processResponse()));
}

GoEngine::~GoEngine()
{
    stop(IgnoreRequests);
}

void GoEngine::start(const QString &command)
{
    kDebug() << command;
    stop(IgnoreRequests);
    m_buffer.clear();
    m_requestQueue.clear();
    m_requestNumber = 0;
    m_result.clear();

    m_process.start(command.toLatin1());
    if (!m_process.waitForStarted()) {        // Blocking wait for process start
        /*emit error(GoEngine::FailedToStart);*/
        return;
    }

    // Test if we started a GTP-compatible Go engine
    m_process.write("name\n");
    m_process.waitForReadyRead();
    QString response = m_process.readAllStandardOutput();
    if (response.isEmpty() || !response.startsWith("=")) {
        stop(IgnoreRequests);
        emit error(GoEngine::NotSupported);
        return;
    }

    emit started();
}

void GoEngine::stop(StopOption options)
{
    if (m_process.isOpen()) {
        if (options == FinishRequests) {
            // TODO: Wait till last command is processed
        }
        m_process.write("quit\n");
        m_process.close();
        emit stopped();
    }
}

void GoEngine::send(GoEngine::Command command, const QVariantList &arguments)
{
    kDebug() << command << arguments;
    QByteArray commandString;

    switch (command) {
        case GetEngineName: commandString = "name\n"; break;
        case GetEngineVersion: commandString = "version\n"; break;
        case GetEngineProtocolVersion: commandString = "protocol_version\n"; break;

        case LoadGameFromSGF:
            if (arguments.size() != 2  || arguments[0].type() != QVariant::String || arguments[1].type() != QVariant::Int) {
                emit error(GoEngine::ArgumentMismatch);
                return;
            }
            commandString = "loadsgf " + arguments[0].toByteArray() + ' ' + QByteArray::number(arguments[1].toInt()) + '\n';
            break;
        case SaveGameToSGF:
            if (arguments.size() != 1 || arguments[0].type() != QVariant::String) {
                emit error(GoEngine::ArgumentMismatch);
                return;
            }
            commandString = "printsgf " + arguments[0].toByteArray() + '\n';
            break;

        case GetPlayerType:
        case SetPlayerType:
        case GetPlayerName:
        case SetPlayerName:
        case GetPlayerStrength:
        case SetPlayerStrength:
        case GetCurrentPlayer:
            emit error(GoEngine::NotSupported); return;

        case GetBoardSize: commandString = "query_boardsize\n"; break;
        case SetBoardSize:
            if (arguments.size() != 1 || arguments[0].type() != QVariant::Int) {
                emit error(GoEngine::ArgumentMismatch);
                return;
            }
            commandString = "board_size " + arguments[0].toByteArray() + '\n';
            break;

        case GetKomi:
        case SetKomi:
        case GetFixedHandicap:
        case SetFixedHandicap:
            emit error(GoEngine::NotSupported); return;

        default: emit error(GoEngine::NotSupported); return;
    }

    m_requestQueue.enqueue(QPair<GoEngine::Command, QByteArray>(command, commandString));

    if (m_requestQueue.size() == 1)
        dispatchRequest();
}

QVariant GoEngine::result() const
{
    return m_result;
}

void GoEngine::dispatchRequest()
{
    kDebug();
    if (!m_requestQueue.isEmpty()) {
        QPair<GoEngine::Command, QByteArray> request = m_requestQueue.dequeue();
        m_pending = request.first;
        m_process.write(QByteArray::number(m_requestNumber++) + ' ' + request.second);
    }
}

void GoEngine::processResponse()
{
    kDebug();
    m_buffer += m_process.readAllStandardOutput();
    if (m_buffer.endsWith("\n\n")) {
        kDebug() << "Response completed:" << m_buffer;
        // Decompose result and store into reponse queue

    /*if (m_engineResponse.size() < 1)
        return false;
    QChar tmp = m_engineResponse[0];                // First message character indicates success or error
    m_engineResponse.remove(0, 2);                  // Remove the first two chars (e.g. "? " or "= ")
    m_engineResponse = m_engineResponse.trimmed();  // Remove further whitespaces, newlines, ...*/

        m_buffer.clear();
        dispatchRequest();
        emit resultReady(m_pending);
    }
}

void GoEngine::processError(QProcess::ProcessError processError)
{
    kDebug() << processError;
    switch (processError) {
        case QProcess::FailedToStart: emit error(GoEngine::FailedToStart); break;
        case QProcess::Crashed: emit error(GoEngine::Crashed); break;
        case QProcess::Timedout: emit error(GoEngine::TimedOut); break;
        case QProcess::WriteError: emit error(GoEngine::WriteError); break;
        case QProcess::ReadError: emit error(GoEngine::ReadError); break;
        case QProcess::UnknownError: emit error(GoEngine::UnknownError); break;
    }
}

} // End of namespace KGo

#include "moc_goengine.cpp"
