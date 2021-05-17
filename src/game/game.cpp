/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "game.h"
#include "score.h"

#include <KLocalizedString>

#include <QApplication>
#include "kigo_debug.h"
#include <QFile>

namespace Kigo {

class UndoCommand : public QUndoCommand
{
    public:

    enum class MoveType { Stone, Passed, Resigned };

    UndoCommand(Player *player, MoveType moveType, const QString &undoStr)
    : QUndoCommand(undoStr), m_player(player), m_moveType(moveType)
    {}

    Player *player () const { return m_player; }
    MoveType moveType () const { return m_moveType; }

    private:

    Player *m_player;
    MoveType m_moveType;
};

Game::Game(QObject *parent)
    : QObject(parent)
    , m_currentMove(0), m_lastUndoIndex(0), m_currentPlayer(&m_blackPlayer)
    , m_blackPlayer(Player::Color::Black), m_whitePlayer(Player::Color::White)
    , m_komi(4.5), m_boardSize(19), m_fixedHandicap(5), m_consecutivePassMoveNumber(0)
    , m_gameFinished(false)
{
    connect(&m_process, &QProcess::readyRead, this, &Game::readyRead);
    connect(&m_undoStack, &QUndoStack::canRedoChanged, this, &Game::canRedoChanged);
    connect(&m_undoStack, &QUndoStack::canUndoChanged, this, &Game::canUndoChanged);
    connect(&m_undoStack, &QUndoStack::indexChanged, this, &Game::undoIndexChanged);
}

Game::~Game()
{
    stop();
}

bool Game::start(const QString &command)
{
    stop();                                   // Close old session if there's one
    QStringList splitArguments = QProcess::splitCommand(command);
    if (!splitArguments.isEmpty()) {
        const QString prog = splitArguments.takeFirst();
        m_process.start(prog, splitArguments);
    }
    if (!m_process.waitForStarted()) {        // Blocking wait for process start
        m_response = QLatin1String("Unable to execute command: ") + command;
        //qCDebug(KIGO_LOG) << m_response;
        return false;
    }
    m_engineCommand = command;
    ////qCDebug(KIGO_LOG) << "Game" << command << "started...";

    // Test if we started a GTP-compatible Go game
    m_process.write("name\n");
    m_process.waitForReadyRead();
    const QString response = m_process.readAllStandardOutput();
    if (response.isEmpty() || !response.startsWith(QLatin1Char('='))) {
        m_response = QStringLiteral("Game did not respond to GTP command \"name\"");
        //qCDebug(KIGO_LOG) << m_response;
        stop();
        return false;
    } else {
        m_engineName = m_response;
    }
    //qCDebug(KIGO_LOG) << "Game is a GTP-compatible Go game";

    m_process.write("version\n");
    if (waitResponse()) {
        m_engineVersion = m_response;
    }
    return true;
}

void Game::stop()
{
    if (m_process.isOpen()) {
        m_process.write("quit\n");
        m_process.close();
    }
}

bool Game::init()
{
    if (!isRunning()) {
        return false;
    }

    //qCDebug(KIGO_LOG) << "Init game!";

    m_process.write("clear_board\n");
    if (waitResponse()) {
        // The board is wiped empty, start again with black player
        setCurrentPlayer(m_blackPlayer);
        m_fixedHandicap = 0;
        m_consecutivePassMoveNumber = 0;
        m_currentMove = 0;
        m_gameFinished = false;
        m_movesList.clear();
        m_undoStack.clear();

        Q_EMIT boardChanged();
        return true;
    } else {
        return false;
    }
}

bool Game::init(const QString &fileName, int moveNumber)
{
    Q_ASSERT(moveNumber >= 0);
    if (!isRunning() || fileName.isEmpty() || !QFile::exists(fileName)) {
        return false;
    }

    m_process.write("loadsgf " + fileName.toLatin1() + ' ' + QByteArray::number(moveNumber) + '\n');
    if (waitResponse()) {
        if (m_response.startsWith(QLatin1String("white"))) { // Check which player is current
            setCurrentPlayer(m_whitePlayer);
        } else {
            setCurrentPlayer(m_blackPlayer);
        }

        m_process.write("query_boardsize\n");       // Query board size from game
        if (waitResponse()) {
            m_boardSize = m_response.toInt();
        }
        m_process.write("get_komi\n");              // Query komi from game and store it
        if (waitResponse()) {
            m_komi = m_response.toFloat();
        }
        m_process.write("get_handicap\n");          // Query fixed handicap and store it
        if (waitResponse()) {
            m_fixedHandicap = m_response.toInt();
        }
        //qCDebug(KIGO_LOG) << "Loaded komi is" << m_komi << "and handicap is" << m_fixedHandicap;

        m_consecutivePassMoveNumber = 0;
        m_currentMove = moveNumber;
        m_gameFinished = false;
        m_movesList.clear();
        m_undoStack.clear();

        Q_EMIT boardSizeChanged(m_boardSize);
        Q_EMIT boardChanged();                             // All done, tell the world!
        return true;
    } else {
        return false;
    }
}

bool Game::save(const QString &fileName)
{
    if (!isRunning() || fileName.isEmpty()) {
        return false;
    }

    m_process.write("printsgf " + fileName.toLatin1() + '\n');
    return waitResponse();
}

bool Game::setBoardSize(int size)
{
    Q_ASSERT(size >= 1 && size <= 19);
    if (!isRunning()) {
        return false;
    }

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
        Q_EMIT boardSizeChanged(size);
        Q_EMIT boardChanged();
        return true;
    } else {
        return false;
    }
}

bool Game::setKomi(float komi)
{
    Q_ASSERT(komi >= 0);
    if (!isRunning()) {
        return false;
    }

    m_process.write("komi " + QByteArray::number(komi) + '\n');
    if (waitResponse()) {
        m_komi = komi;
        return true;
    } else {
        return false;
    }
}

bool Game::setFixedHandicap(int handicap)
{
    Q_ASSERT(handicap >= 2 && handicap <= 9);
    if (!isRunning()) {
        return false;
    }

    if (handicap <= fixedHandicapUpperBound()) {
        m_process.write("fixed_handicap " + QByteArray::number(handicap) + '\n');
        if (waitResponse()) {
            // Black starts with setting his (fixed) handicap as it's first turn
            // which means, white is next.
            setCurrentPlayer(m_whitePlayer);
            m_fixedHandicap = handicap;
            Q_EMIT boardChanged();
            return true;
        } else {
            return false;
        }
    } else {
        //qCWarning(KIGO_LOG) << "Handicap" << handicap << " not set, it is too high!";
        return false;
    }
}

int Game::fixedHandicapUpperBound()
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

bool Game::playMove(const Move &move, bool undoable)
{
    return playMove(*move.player(), move.stone(), undoable);
}

bool Game::playMove(const Player &player, const Stone &stone, bool undoable)
{
    if (!isRunning()) {
        return false;
    }

    const Player *tmp = &player;
    if (!tmp->isValid()) {
        //qCDebug(KIGO_LOG) << "Invalid player argument, using current player!";
        tmp = m_currentPlayer;
    }

    QByteArray msg("play ");                    // The command to be sent
    if (tmp->isWhite()) {
        msg.append("white ");
    } else {
        msg.append("black ");
    }
    if (stone.isValid()) {
        msg.append(stone.toLatin1() + '\n');
    } else {
        msg.append("pass\n");
    }

    m_process.write(msg);                       // Send command to backend
    if (waitResponse()) {
        if (stone.isValid()) {                  // Normal move handling
            m_movesList.append(Move(tmp, stone));
            m_consecutivePassMoveNumber = 0;
        } else {                                // And pass move handling
            m_movesList.append(Move(tmp, Stone::Pass));
            Q_EMIT passMovePlayed(*m_currentPlayer);
            if (m_consecutivePassMoveNumber > 0) {
                Q_EMIT consecutivePassMovesPlayed(m_consecutivePassMoveNumber);
            }
            m_consecutivePassMoveNumber++;
        }
        m_currentMove++;

        if (undoable) {                         // Do undo stuff if desired
            Player *playerTemp;
            UndoCommand::MoveType moveType;
            QString undoStr;
            if (tmp->isWhite()) {
                playerTemp = &m_whitePlayer;
                if (stone.isValid()) {
                    moveType = UndoCommand::MoveType::Stone;
                    undoStr = i18nc("%1 stone coordinate", "White %1", stone.toString());
                } else {
                    moveType = UndoCommand::MoveType::Passed;
                    undoStr = i18n("White passed");
                }
            } else {
                playerTemp = &m_blackPlayer;
                if (stone.isValid()) {
                    moveType = UndoCommand::MoveType::Stone;
                    undoStr = i18nc("%1 stone coordinate", "Black %1", stone.toString());
                } else {
                    moveType = UndoCommand::MoveType::Passed;
                    undoStr = i18n("Black passed");
                }
            }
            //qCDebug(KIGO_LOG) << "Push new undo command" << undoStr;
            m_undoStack.push(new UndoCommand(playerTemp, moveType, undoStr));
        }

        if (tmp->isWhite()) {                   // Determine the next current player
            setCurrentPlayer(m_blackPlayer);
        } else {
            setCurrentPlayer(m_whitePlayer);
        }

        Q_EMIT boardChanged();
        return true;
    } else {
        return false;
    }
}

bool Game::generateMove(const Player &player, bool undoable)
{
    if (!isRunning()) {
        return false;
    }
    const Player *tmp = &player;
    if (!tmp->isValid()) {
        //qCDebug(KIGO_LOG) << "Invalid player argument, using current player!";
        tmp = m_currentPlayer;
    }

    if (tmp->isWhite()) {
        m_process.write("level " + QByteArray::number(m_whitePlayer.strength()) + '\n');
        waitResponse(); // Setting level is not mission-critical, no error checking
        m_process.write("genmove white\n");
    } else {
        m_process.write("level " + QByteArray::number(m_blackPlayer.strength()) + '\n');
        waitResponse(); // Setting level is not mission-critical, no error checking
        m_process.write("genmove black\n");
    }
    if (waitResponse(true)) {
        bool boardChange = false;
        Player *playerTemp;
        UndoCommand::MoveType moveType;
        QString undoStr;

        if (tmp->isWhite()) {
            playerTemp = &m_whitePlayer;
        } else {
            playerTemp = &m_blackPlayer;
        }
        if (m_response == QLatin1String("PASS")) {
            m_currentMove++;
            Q_EMIT passMovePlayed(*m_currentPlayer);
            if (m_consecutivePassMoveNumber > 0) {
                Q_EMIT consecutivePassMovesPlayed(m_consecutivePassMoveNumber);
                m_gameFinished = true;
            }
            m_consecutivePassMoveNumber++;
            moveType = UndoCommand::MoveType::Passed;
            if (tmp->isWhite()) {
                undoStr = i18n("White passed");
            } else {
                undoStr = i18n("Black passed");
            }
        } else if (m_response == QLatin1String("resign")) {
            Q_EMIT resigned(*m_currentPlayer);
            m_gameFinished = true;
            moveType = UndoCommand::MoveType::Resigned;
            if (tmp->isWhite()) {
                undoStr = i18n("White resigned");
            } else {
                undoStr = i18n("Black resigned");
            }
        } else {
            m_currentMove++;
            m_movesList.append(Move(tmp, Stone(m_response)));
            m_consecutivePassMoveNumber = 0;
            moveType = UndoCommand::MoveType::Stone;
            if (tmp->isWhite()) {
                undoStr = i18nc("%1 response from Go engine", "White %1", m_response);
            } else {
                undoStr = i18nc("%1 response from Go engine", "Black %1", m_response);
            }
            boardChange = true;
        }

        if (undoable) {
            //qCDebug(KIGO_LOG) << "Push new undo command" << undoStr;
            m_undoStack.push(new UndoCommand(playerTemp, moveType, undoStr));
        }
        if (tmp->isWhite()) {
            setCurrentPlayer(m_blackPlayer);
        } else {
            setCurrentPlayer(m_whitePlayer);
        }
        if (boardChange) {
            Q_EMIT boardChanged();
        }
        return true;
    } else {
        return false;
    }
}

bool Game::undoMove()
{
    if (!isRunning()) {
        return false;
    }

    m_process.write("undo\n");
    if (waitResponse()) {
        Move lastMove = m_movesList.takeLast();
        m_currentMove--;
        if (lastMove.player()->isComputer()) {
            // Do one more undo
            m_process.write("undo\n");
            if (waitResponse()) {
                lastMove = m_movesList.takeLast();
                m_currentMove--;
            }
            m_undoStack.undo();
        }
        if (lastMove.player()->isWhite()) {
            setCurrentPlayer(m_whitePlayer);
        } else {
            setCurrentPlayer(m_blackPlayer);
        }
        if (m_consecutivePassMoveNumber > 0) {
            m_consecutivePassMoveNumber--;
        }
        //TODO: What happens with pass moves deeper in the history?
        m_undoStack.undo();

        Q_EMIT boardChanged();
        return true;
    } else {
        return false;
    }
}

bool Game::redoMove()
{
    if (!isRunning()) {
        return false;
    }

    const UndoCommand *undoCmd = static_cast<const UndoCommand*>(m_undoStack.command(m_undoStack.index()));

    const Player *player = undoCmd->player();

    if (undoCmd->moveType() == UndoCommand::MoveType::Passed) {
        //qCDebug(KIGO_LOG) << "Redo a pass move for" << player << undoCmd->text();
        playMove(*player, Stone(), false);         // E.g. pass move
    } else if (undoCmd->moveType() == UndoCommand::MoveType::Resigned) {
        // Note: Although it is possible to undo after a resign and redo it,
        //       it is a bit questionable whether this makes sense logically.
        //qCDebug(KIGO_LOG) << "Redo a resign for" << player << undoCmd->text();
        Q_EMIT resigned(*player);
        m_gameFinished = true;
        //Q_EMIT resigned(*m_currentPlayer);
    } else {
        //qCDebug(KIGO_LOG) << "Redo a normal move for" << player << undoCmd->text();
        playMove(*player, Stone(undoCmd->text()), false);
    }
    m_undoStack.redo();
    return false;
}

Move Game::lastMove() const
{
    Q_ASSERT(!m_movesList.isEmpty());
    return m_movesList.last();
}

int Game::moveCount()
{
    if (!isRunning()) {
        return 0;
    }

    m_process.write("move_history\n");          // Query fixed handicap and store it
    if (waitResponse()) {
        return m_response.count(QLatin1Char('\n')) + 1;
    }
    return 0;
}

QList<Stone> Game::stones(const Player &player)
{
    QList<Stone> list;
    if (!isRunning()) {
        return list;
    }

    // Invalid player means all stones
    if (!player.isWhite()) {
        m_process.write("list_stones black\n");
        if (waitResponse() && !m_response.isEmpty()) {
            const auto positions = m_response.split(QLatin1Char(' '));
            for (const QString &pos : positions) {
                list.append(Stone(pos));
            }
        }
    }
    if (!player.isBlack()) {
        m_process.write("list_stones white\n");
        if (waitResponse() && !m_response.isEmpty()) {
            const auto positions = m_response.split(QLatin1Char(' '));
            for (const QString &pos : positions) {
                list.append(Stone(pos));
            }
        }
    }
    return list;
}

QList<Move> Game::moves(const Player &player)
{
    QList<Move> list;
    if (!isRunning()) {
        return list;
    }

    if (!player.isValid()) {
        list = m_movesList;
    } else {
        for (const Move &move : qAsConst(m_movesList)) {
            if (move.player()->color() == player.color()) {
                list.append(move);
            }
        }
    }
    return list;
}

QList<Stone> Game::liberties(const Stone &stone)
{
    QList<Stone> list;
    if (!isRunning() || !stone.isValid()) {
        return list;
    }

    m_process.write("findlib " + stone.toLatin1() + '\n');
    if (waitResponse() && !m_response.isEmpty()) {
        const auto entries = m_response.split(QLatin1Char(' '));
        for (const QString &entry : entries) {
            list.append(Stone(entry));
        }
    }
    return list;
}

QList<Stone> Game::bestMoves(const Player &player)
{
    QList<Stone> list;
    if (!isRunning() || !player.isValid()) {
        return list;
    }

    if (player.isWhite()) {
        m_process.write("top_moves_white\n");
    } else {
        m_process.write("top_moves_black\n");
    }
    if (waitResponse(true) && !m_response.isEmpty()) {
        const QStringList parts = m_response.split(QLatin1Char(' '));
        if (parts.size() % 2 == 0) {
            for (int i = 0; i < parts.size(); i += 2) {
                list.append(Stone(parts[i], QString(parts[i + 1]).toFloat()));
            }
        }
    }
    return list;
}

QList<Stone> Game::legalMoves(const Player &player)
{
    QList<Stone> list;
    if (!isRunning() || !player.isValid()) {
        return list;
    }

    if (player.isWhite()) {
        m_process.write("all_legal white\n");
    } else {
        m_process.write("all_legal black\n");
    }
    if (waitResponse() && !m_response.isEmpty()) {
        const auto entries = m_response.split(QLatin1Char(' '));
        for (const QString &entry : entries) {
            list.append(Stone(entry));
        }
    }
    return list;
}

int Game::captures(const Player &player)
{
    if (!isRunning() || !player.isValid()) {
        return 0;
    }

    if (player.isWhite()) {
        m_process.write("captures white\n");
    } else {
        m_process.write("captures black\n");
    }
    return waitResponse() ? m_response.toInt() : 0;
}

Game::FinalState Game::finalState(const Stone &stone)
{
    if (!isRunning() || !stone.isValid()) {
        return FinalState::FinalStateInvalid;
    }

    m_process.write("final_status " + stone.toLatin1() + '\n');
    if (waitResponse()) {
        if (m_response == QLatin1String("alive")) { return FinalState::FinalAlive; }
        else if (m_response == QLatin1String("dead")) { return FinalState::FinalDead; }
        else if (m_response == QLatin1String("seki")) { return FinalState::FinalSeki; }
        else if (m_response == QLatin1String("white_territory")) { return FinalState::FinalWhiteTerritory; }
        else if (m_response == QLatin1String("blacK_territory")) { return FinalState::FinalBlackTerritory; }
        else if (m_response == QLatin1String("dame")) { return FinalState::FinalDame; }
        else { return FinalState::FinalStateInvalid; }
    } else {
        return FinalState::FinalStateInvalid;
    }
}

QList<Stone> Game::finalStates(FinalState state)
{
    QList<Stone> list;
    if (!isRunning() || state == FinalState::FinalStateInvalid) {
        return list;
    }

    QByteArray msg("final_status_list ");
    switch (state) {
        case FinalState::FinalAlive: msg.append("alive"); break;
        case FinalState::FinalDead: msg.append("dead"); break;
        case FinalState::FinalSeki: msg.append("seki"); break;
        case FinalState::FinalWhiteTerritory: msg.append("white_territory"); break;
        case FinalState::FinalBlackTerritory: msg.append("black_territory"); break;
        case FinalState::FinalDame: msg.append("dame"); break;
        case FinalState::FinalStateInvalid: /* Will never happen */ break;
    }
    msg.append('\n');
    m_process.write(msg);
    if (waitResponse() && !m_response.isEmpty()) {
        const auto entries = m_response.split(QLatin1Char(' '));
        for (const QString &entry : entries) {
            list.append(Stone(entry));
        }
    }
    return list;
}

Score Game::finalScore()
{
    if (!isRunning()) {
        return Score();
    }

    m_process.write("final_score\n");
    return waitResponse() ? Score(m_response) : Score();
}

Score Game::estimatedScore()
{
    if (!isRunning()) {
        return Score();
    }

    m_process.write("estimate_score\n");
    return waitResponse() ? Score(m_response) : Score();
}

bool Game::waitResponse(bool nonBlocking)
{
    if (m_process.state() != QProcess::Running) {   // No GTP connection means no computing fun!
        switch (m_process.error()) {
            case QProcess::FailedToStart: m_response = QStringLiteral("No Go game is running!"); break;
            case QProcess::Crashed: m_response = QStringLiteral("The Go game crashed!"); break;
            case QProcess::Timedout: m_response = QStringLiteral("The Go game timed out!"); break;
            case QProcess::WriteError: m_response = m_process.readAllStandardError(); break;
            case QProcess::ReadError: m_response = m_process.readAllStandardError(); break;
            case QProcess::UnknownError: m_response = QStringLiteral("Unknown error!"); break;
        }
        qCWarning(KIGO_LOG) << "Command failed:" << m_response;
        return false;
    }

    if (nonBlocking) {
        Q_EMIT waiting(true);
    }

    // Wait for finished command execution. We have to do this untill '\n\n' (or '\r\n\r\n' or
    // '\r\r' in case of MS Windows and Mac, respectively) arrives in our input buffer to show
    // that the Go game is done processing our request. The 'nonBlocking' parameter decides
    // whether we block and wait (suitable for short commands) or if we continue processing
    // events in between to stop the UI from blocking (suitable for longer commands).
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
    } while (!m_response.endsWith(QLatin1String("\r\r")) &&
             !m_response.endsWith(QLatin1String("\n\n")) &&
             !m_response.endsWith(QLatin1String("\r\n\r\n")));

    if (nonBlocking) {
        Q_EMIT waiting(false);
    }

    if (m_response.size() < 1) {
        return false;
    }
    QChar tmp = m_response[0];                  // First message character indicates success or error
    m_response.remove(0, 2);                    // Remove the first two chars (e.g. "? " or "= ")
    m_response = m_response.trimmed();          // Remove further whitespaces, newlines, ...
    return tmp != QLatin1Char('?');                          // '?' Means the game didn't understand the query
}

void Game::gameSetup()
{
    Q_EMIT boardInitialized();
}

void Game::readyRead()
{
    m_waitAndProcessEvents = false;
}

void Game::undoIndexChanged(int index)
{
    m_lastUndoIndex = index;
}

void Game::setCurrentPlayer(Player &player)
{
    m_currentPlayer = &player;
    Q_EMIT currentPlayerChanged(*m_currentPlayer);
}

} // End of namespace Kigo
