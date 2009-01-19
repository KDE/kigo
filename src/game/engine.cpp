/*
    Copyright 2008 Sascha Peilicke <sasch.pe@gmx.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "engine.h"
#include "score.h"

#include <KDebug>
#include <KLocale>

#include <QApplication>
#include <QFile>

namespace Kigo {

Engine::Engine(QObject *parent)
    : QObject(parent)
    , m_currentMove(0), m_currentPlayer(m_blackPlayer)
    , m_blackPlayer(Player::Black), m_whitePlayer(Player::White)
    , m_komi(0), m_boardSize(-1), m_fixedHandicap(0), m_consecutivePassMoveNumber(0)
{
    connect(&m_process, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(&m_undoStack, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
    connect(&m_undoStack, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
}

Engine::~Engine()
{
    stop();
}

bool Engine::start(const QString &command)
{
    stop();                                   // Close old session if there's one
    m_process.start(command.toLatin1());      // Start new process with provided command
    if (!m_process.waitForStarted()) {        // Blocking wait for process start
        m_response = "Unable to execute command: " + command;
        kDebug() << m_response;
        return false;
    }
    m_engineCommand = command;
    kDebug() << "Engine" << command << "started...";

    // Test if we started a GTP-compatible Go engine
    m_process.write("name\n");
    m_process.waitForReadyRead();
    QString response = m_process.readAllStandardOutput();
    if (response.isEmpty() || !response.startsWith("=")) {
        m_response = "Engine did not respond to GTP command \"name\"";
        kDebug() << m_response;
        stop();
        return false;
    } else {
        m_engineName = m_response;
    }
    kDebug() << "Engine is a GTP-compatible Go engine";

    m_process.write("version\n");
    if (waitResponse())
        m_engineVersion = m_response;
    return true;
}

void Engine::stop()
{
    if (m_process.isOpen()) {
        kDebug() << "Quit GTP engine session";
        m_process.write("quit\n");
        m_process.close();
    }
}

bool Engine::init()
{
    if (!isRunning())
        return false;

    m_process.write("clear_board\n");
    if (waitResponse()) {
        // The board is wiped empty, start again with black player
        setCurrentPlayer(m_blackPlayer);
        m_fixedHandicap = 0;
        m_consecutivePassMoveNumber = 0;
        m_currentMove = 0;
        m_movesList.clear();
        m_undoStack.clear();

        emit changed();
        return true;
    } else
        return false;
}

bool Engine::init(const QString &fileName, int moveNumber)
{
    Q_ASSERT(moveNumber >= 0);
    if (!isRunning() || fileName.isEmpty() || !QFile::exists(fileName))
        return false;

    m_process.write("loadsgf " + fileName.toLatin1() + ' ' + QByteArray::number(moveNumber) + '\n');
    if (waitResponse()) {
        if (m_response.startsWith("white"))         // Check which player is current
            setCurrentPlayer(m_whitePlayer);
        else
            setCurrentPlayer(m_blackPlayer);

        m_process.write("query_boardsize\n");       // Query board size from engine
        if (waitResponse())
            m_boardSize = m_response.toInt();
        m_process.write("get_komi\n");              // Query komi from engine and store it
        if (waitResponse())
            m_komi = m_response.toFloat();
        m_process.write("get_handicap\n");          // Query fixed handicap and store it
        if (waitResponse())
            m_fixedHandicap = m_response.toInt();
        //kDebug() << "Loaded komi is" << m_komi << "and handicap is" << m_fixedHandicap;

        m_consecutivePassMoveNumber = 0;
        m_currentMove = moveNumber;                  // Store move number
        m_movesList.clear();
        m_undoStack.clear();

        emit changed();                             // All done, tell the world!
        return true;
    } else
        return false;
}

bool Engine::save(const QString &fileName)
{
    if (!isRunning() || fileName.isEmpty())
        return false;

    m_process.write("printsgf " + fileName.toLatin1() + '\n');
    return waitResponse();
}

bool Engine::setBoardSize(int size)
{
    Q_ASSERT(size >= 1 && size <= 19);
    if (!isRunning())
        return false;

    m_process.write("boardsize " + QByteArray::number(size) + '\n');
    if (waitResponse()) {
        // Changing size wipes the board, start again with black player.
        setCurrentPlayer(m_blackPlayer);
        m_boardSize = size;
        m_fixedHandicap = 0;
        m_consecutivePassMoveNumber = 0;
        m_currentMove = 0;
        m_movesList.clear();
        m_undoStack.clear();
        emit sizeChanged(size);
        emit changed();
        return true;
    } else
        return false;
}

bool Engine::setKomi(float komi)
{
    Q_ASSERT(komi >= 0);
    if (!isRunning())
        return false;

    m_process.write("komi " + QByteArray::number(komi) + '\n');
    if (waitResponse()) {
        m_komi = komi;
        return true;
    } else
        return false;
}

bool Engine::setFixedHandicap(int handicap)
{
    Q_ASSERT(handicap >= 2 && handicap <= 9);
    if (!isRunning())
        return false;

    if (handicap <= fixedHandicapUpperBound()) {
        m_process.write("fixed_handicap " + QByteArray::number(handicap) + '\n');
        if (waitResponse()) {
            // Black starts with setting his (fixed) handicap as it's first turn
            // which means, white is next.
            setCurrentPlayer(m_whitePlayer);
            m_fixedHandicap = handicap;
            emit changed();
            return true;
        } else
            return false;
    } else {
        kWarning() << "Handicap" << handicap << " not set, it is too high!";
        return false;
    }
}

int Engine::fixedHandicapUpperBound()
{
    switch (m_boardSize) {  // Handcrafted values reflect what GnuGo accepts
        case 7:
        case 8:
        case 10:
        case 12:
        case 14:
        case 16:
        case 18: return 4;
        case 9:
        case 11:
        case 13:
        case 15:
        case 17:
        case 19: return 9;
        default: return 0;
    }
}

bool Engine::playMove(const Move &move, bool undoable)
{
    return playMove(move.player(), move.stone(), undoable);
}

bool Engine::playMove(const Player &player, const Stone &stone, bool undoable)
{
    if (!isRunning())
        return false;

    Player tmp = player;
    if (!tmp.isValid())
        tmp = m_currentPlayer;

    QByteArray msg("play ");                    // The command to be sent
    if (tmp.isWhite())
        msg.append("white ");
    else
        msg.append("black ");
    if (stone.isValid())
        msg.append(stone.toLatin1() + '\n');
    else
        msg.append("pass\n");

    m_process.write(msg);                       // Send command to backend
    if (waitResponse()) {
        if (tmp.isWhite())                     // Determine the next current player
            setCurrentPlayer(m_blackPlayer);
        else
            setCurrentPlayer(m_whitePlayer);

        if (stone.isValid()) {                  // Normal move handling
            m_movesList.append(Move(tmp, stone));
            m_consecutivePassMoveNumber = 0;
        } else {                                // And pass move handling
            m_movesList.append(Move(tmp, Stone::Pass));
            if (m_consecutivePassMoveNumber > 0)
                emit consecutivePassMovesPlayed(m_consecutivePassMoveNumber);
            m_consecutivePassMoveNumber++;
        }
        m_currentMove++;

        if (undoable) {                         // Do undo stuff if desired
            QString undoStr;
            if (tmp.isWhite())
                undoStr = i18n("White ");
            else
                undoStr = i18n("Black ");

            if (stone.isValid())
                undoStr += stone.toLatin1();
            else
                undoStr += i18n("passed");
            //kDebug() << "Push new undo command" << undoStr;
            m_undoStack.push(new QUndoCommand(undoStr));
        }

        emit changed();
        return true;
    } else
        return false;
}

bool Engine::generateMove(const Player &player, bool undoable)
{
    if (!isRunning())
        return false;

    Player tmp = player;
    if (!tmp.isValid())
        tmp = m_currentPlayer;

    if (tmp.isWhite()) {
        m_process.write("level " + QByteArray::number(m_whitePlayer.strength()) + '\n');
        waitResponse(); // Setting level is not mission-critical, no error checking
        m_process.write("genmove white\n");
    } else {
        m_process.write("level " + QByteArray::number(m_blackPlayer.strength()) + '\n');
        waitResponse(); // Setting level is not mission-critical, no error checking
        m_process.write("genmove black\n");
    }
    if (waitResponse(true)) {
        QString undoStr;
        if (tmp.isWhite()) {
            undoStr = i18n("White ");
            setCurrentPlayer(m_blackPlayer);
        } else {
            undoStr = i18n("Black ");
            setCurrentPlayer(m_whitePlayer);
        }

        if (m_response == "PASS") {
            m_currentMove++;
            if (m_consecutivePassMoveNumber > 0)
                emit consecutivePassMovesPlayed(m_consecutivePassMoveNumber);
            m_consecutivePassMoveNumber++;
            undoStr += i18n("pass");
        } else if (m_response == "resign") {
            emit resigned(m_currentPlayer);
            undoStr += i18n("resign");
        } else {
            m_currentMove++;
            m_movesList.append(Move(tmp, Stone(m_response)));
            m_consecutivePassMoveNumber = 0;
            undoStr += m_response;
            emit changed();
        }

        if (undoable) {
            //kDebug() << "Push new undo command" << undoStr;
            m_undoStack.push(new QUndoCommand(undoStr));
        }
        return true;
    } else
        return false;
}

bool Engine::undoMove()
{
    if (!isRunning())
        return false;

    m_process.write("undo\n");
    if (waitResponse()) {
        //TODO: What happens with pass moves ?!?
        /*Move lastMove = m_movesList.takeLast();
        m_currentMove--;
        if (lastMove.second == WhitePlayer)
            setCurrentPlayer(WhitePlayer);
        else
            setCurrentPlayer(BlackPlayer);
        if (m_consecutivePassMoveNumber > 0)
            m_consecutivePassMoveNumber--;
        m_undoStack.undo();*/

        emit changed();
        return true;
    } else
        return false;
}

bool Engine::redoMove()
{
    if (!isRunning())
        return false;

    QString undoStr = m_undoStack.text(m_undoStack.index());

    //TODO: Reimplement this
    /*PlayerColor color;
    if (undoStr.startsWith(i18n("Black "))) {
        color = BlackPlayer;
    } else if (undoStr.startsWith(i18n("White "))) {
        color = WhitePlayer;
    } else {
        kDebug() << "Invalid undo/redo command found:" << undoStr;
        return false;
    }

    undoStr.remove(0, undoStr.indexOf(' ')+1);
    if (undoStr.startsWith(i18n("pass"))) {
        kDebug() << "Redo a pass move for" << color << undoStr;
        playMove(Stone(), color, true);         // E.g. pass move
    } else if (undoStr.startsWith(i18n("resign"))) {
        // Note: Altough it is possible to undo after a resign and redo it,
        //       it is a bit questionable whether this makes sense logically.
        kDebug() << "Redo a resign for" << color << undoStr;
        emit resigned(m_currentPlayer);
    } else {
        kDebug() << "Redo a normal move for" << color << undoStr;
        playMove(Stone(undoStr), color, true);
    }*/
    m_undoStack.redo();
    return false;
}

Move Engine::lastMove() const
{
    Q_ASSERT(!m_movesList.isEmpty());
    /*if (m_movesList.isEmpty())
        return Move(m_currentPlayer, Stone());
    else*/
    return m_movesList.last();
}

QList<Stone> Engine::stones(const Player &player)
{
    QList<Stone> list;
    if (!isRunning())
        return list;

    // Invalid player means all stones
    if (!player.isWhite()) {
        m_process.write("list_stones black\n");
        if (waitResponse() && !m_response.isEmpty()) {
            foreach (const QString &pos, m_response.split(' '))
                list.append(Stone(pos));
        }
    }
    if (!player.isBlack()) {
        m_process.write("list_stones white\n");
        if (waitResponse() && !m_response.isEmpty()) {
            foreach (const QString &pos, m_response.split(' '))
                list.append(Stone(pos));
        }
    }
    return list;
}

QList<Move> Engine::moves(const Player &player)
{
    QList<Move> list;
    if (!isRunning())
        return list;

    if (!player.isValid()) {
        list = m_movesList;
    } else {
        foreach (const Move &move, m_movesList) {
            if (move.player().color() == player.color())
                list.append(move);
        }
    }
    return list;
}

QList<Stone> Engine::liberties(const Stone &stone)
{
    QList<Stone> list;
    if (!isRunning() || !stone.isValid())
        return list;

    m_process.write("findlib " + stone.toLatin1() + '\n');
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

QList<Stone> Engine::bestMoves(const Player &player)
{
    QList<Stone> list;
    if (!isRunning() || !player.isValid())
        return list;

    if (player.isWhite())
        m_process.write("top_moves_white\n");
    else
        m_process.write("top_moves_black\n");
    if (waitResponse(true) && !m_response.isEmpty()) {
        QStringList parts = m_response.split(' ');
        if (parts.size() % 2 == 0)
            for (int i = 0; i < parts.size(); i += 2)
                list.append(Stone(parts[i], QString(parts[i + 1]).toFloat()));
    }
    return list;
}

QList<Stone> Engine::legalMoves(const Player &player)
{
    QList<Stone> list;
    if (!isRunning() || !player.isValid())
        return list;

    if (player.isWhite())
        m_process.write("all_legal white\n");
    else
        m_process.write("all_legal black\n");
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

int Engine::captures(const Player &player)
{
    if (!isRunning() || !player.isValid())
        return 0;

    if (player.isWhite())
        m_process.write("captures white\n");
    else
        m_process.write("captures black\n");
    return waitResponse() ? m_response.toInt() : 0;
}

Engine::FinalState Engine::finalState(const Stone &stone)
{
    if (!isRunning() || !stone.isValid())
        return FinalStateInvalid;

    m_process.write("final_status " + stone.toLatin1() + '\n');
    if (waitResponse()) {
        if (m_response == "alive") return FinalAlive;
        else if (m_response == "dead") return FinalDead;
        else if (m_response == "seki") return FinalSeki;
        else if (m_response == "white_territory") return FinalWhiteTerritory;
        else if (m_response == "blacK_territory") return FinalBlackTerritory;
        else if (m_response == "dame") return FinalDame;
        else return FinalStateInvalid;
    } else
        return FinalStateInvalid;
}

QList<Stone> Engine::finalStates(FinalState state)
{
    QList<Stone> list;
    if (!isRunning() || state == FinalStateInvalid)
        return list;

    QByteArray msg("final_status_list ");
    switch (state) {
        case FinalAlive: msg.append("alive"); break;
        case FinalDead: msg.append("dead"); break;
        case FinalSeki: msg.append("seki"); break;
        case FinalWhiteTerritory: msg.append("white_territory"); break;
        case FinalBlackTerritory: msg.append("black_territory"); break;
        case FinalDame: msg.append("dame"); break;
        case FinalStateInvalid: /* Will never happen */ break;
    }
    msg.append('\n');
    m_process.write(msg);
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

Score Engine::finalScore()
{
    if (!isRunning())
        return Score();

    m_process.write("final_score\n");
    return waitResponse() ? Score(m_response) : Score();
}

Score Engine::estimateScore()
{
    if (!isRunning())
        return Score();

    m_process.write("estimate_score\n");
    return waitResponse() ? Score(m_response) : Score();
}

bool Engine::waitResponse(bool nonBlocking)
{
    if (m_process.state() != QProcess::Running) {   // No GTP connection means no computing fun!
        switch (m_process.error()) {
            case QProcess::FailedToStart: m_response = "No Go engine is running!"; break;
            case QProcess::Crashed: m_response = "The Go engine crashed!"; break;
            case QProcess::Timedout: m_response = "The Go engine timed out!"; break;
            case QProcess::WriteError: m_response = m_process.readAllStandardError(); break;
            case QProcess::ReadError: m_response = m_process.readAllStandardError(); break;
            case QProcess::UnknownError: m_response = "Unknown error!"; break;
        }
        kWarning() << "Command failed:" << m_response;
        return false;
    }

    if (nonBlocking)
        emit waiting(true);

    // Wait for finished command execution. We have to do this untill '\n\n' arives in our
    // input buffer to show that the Go engine is done processing our request. The 'nonBlocking'
    // parameter decides whether we block and wait (suitable for short commands) or if we continue
    // processing events in between to stop the UI from blocking (suitable for longer commands).
    // The latter may introduce flickering.
    m_response.clear();
    do {
        if (nonBlocking) {
            while (m_waitAndProcessEvents) {    // The flag is modified by another slot
                qApp->processEvents();          // called when QProcess signals readyRead()
            }
            m_waitAndProcessEvents = true;
        } else {
            m_process.waitForReadyRead();       // Blocking wait
        }
        m_response += m_process.readAllStandardOutput();
    } while(!m_response.endsWith("\n\n"));

    if (nonBlocking)
        emit waiting(false);

    if (m_response.size() < 1)
        return false;
    QChar tmp = m_response[0];                  // First message character indicates success or error
    m_response.remove(0, 2);                    // Remove the first two chars (e.g. "? " or "= ")
    m_response = m_response.trimmed();          // Remove further whitespaces, newlines, ...
    return tmp != '?';                          // '?' Means the engine didn't understand the query
}

void Engine::readyRead()
{
    m_waitAndProcessEvents = false;
}

void Engine::setCurrentPlayer(const Player &player)
{
    m_currentPlayer = player;
    emit currentPlayerChanged(m_currentPlayer);
}

} // End of namespace Kigo

#include "moc_engine.cpp"
