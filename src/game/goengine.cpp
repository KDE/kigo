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
 * @file This file is part of KGO and implements the classes GoEngine, GoEngine::Stone
 *       and GoEngine::Score, which together implement a Go Text Protocol (GTP)
 *       interface to communicate with Go engines supporting GTP protocol
 *       version 2.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#include "goengine.h"

#include <KDebug>

#include <QFile>

namespace KGo {

GoEngine::Stone::Stone(const QString &stone)
    : m_x(0), m_y(0)
{
    if (stone.size() >= 2) {
        m_x = stone[0].toUpper().toLatin1();
        m_y = stone.mid(1).toInt();
    }
}

bool GoEngine::Stone::isValid() const
{
    // Go coordinates are somewhat complicated ...
    return m_y >= 1 && m_y <= 19 && m_x >= 'A' && m_x != 'I' && m_x <= 'T';
}

QByteArray GoEngine::Stone::toLatin1() const
{
    QByteArray msg;
    msg.append(m_x);
    msg.append(QByteArray::number(m_y));
    return msg;
}

QString GoEngine::Stone::toString() const
{
    return QString(m_x + QString::number(m_y));
}

////////////////////////////////////////////////////////////////////

GoEngine::Score::Score(const QString &scoreString)
    : m_player(InvalidPlayer)
    , m_score(0), m_lowerBound(0), m_upperBound(0)
{
    if (scoreString.size() >= 2) {
        if (scoreString[0] == 'W')
            m_player = WhitePlayer;
        else if (scoreString[0] == 'B')
            m_player = BlackPlayer;
        else
            m_player = InvalidPlayer;
        int i = scoreString.indexOf(' ');
        m_score = scoreString.mid(2, i - 1).toFloat();
        QString upperBound = scoreString.section(' ', 3, 3);
        upperBound.chop(1);
        m_upperBound = upperBound.toFloat();
        QString lowerBound = scoreString.section(' ', 5, 5);
        lowerBound.chop(1);
        m_lowerBound = lowerBound.toFloat();
    }
}

bool GoEngine::Score::isValid() const
{
    return m_score >= 0 && (m_player == WhitePlayer || m_player == BlackPlayer);
}

QString GoEngine::Score::toString() const
{
    QString ret;
    if (m_player == WhitePlayer)
        ret += "W+";
    else if (m_player == BlackPlayer)
        ret += "B+";
    else
        ret += "?+";
    ret += QString::number(m_score) + " (" + QString::number(m_lowerBound) + " - " + QString::number(m_upperBound) + ')';
    return ret;
}

////////////////////////////////////////////////////////////////////

GoEngine::GoEngine()
    : m_currentPlayer(BlackPlayer)
    , m_whitePlayerType(HumanPlayer), m_whitePlayerStrength(10)
    , m_blackPlayerType(HumanPlayer), m_blackPlayerStrength(10)
    , m_komi(0), m_fixedHandicap(0), m_moveNumber(0)
{
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), SIGNAL(error(QProcess::ProcessError)));
}

GoEngine::~GoEngine()
{
    quit();
}

bool GoEngine::start(const QString &command)
{
    quit();                                         // Close old session if there's one
    m_process.start(command.toLatin1());            // Start new process with provided command
    if (!m_process.waitForStarted()) {              // Blocking wait for process start
        m_response = "Execute Go engine command \"" + command + "\" failed!";
        return false;
    }
    m_engineCommand = command;                      // Save for retrieval

    kDebug() << "Starting new GTP engine session...";
    if (protocolVersion() <= 0) {                   // Check for supported GTP protocol version
        kDebug() << "Protocol version error";
        quit();
        return false;
    }
    return true;
}

void GoEngine::quit()
{
    if (m_process.isOpen()) {
        kDebug() << "Quit GTP engine session";
        m_process.write("quit\n");
        m_process.close();
    }
}

bool GoEngine::loadSgf(const QString &fileName, int moveNumber)
{
    Q_ASSERT(moveNumber >= 0);

    if (fileName.isEmpty() || !QFile::exists(fileName))
        return false;

    m_process.write("loadsgf " + fileName.toLatin1() + ' ' + QByteArray::number(moveNumber) + '\n');
    if (waitResponse()) {
        //TODO: Set current player based on last turn in sgf file
        //TODO: Check whether handicap was placed
        changeCurrentPlayer(InvalidPlayer);
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::saveSgf(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    m_process.write("printsgf " + fileName.toLatin1() + '\n');
    return waitResponse();
}

QString GoEngine::engineName()
{
    m_process.write("name\n");
    return waitResponse() ? m_response : QString();
}

int GoEngine::protocolVersion()
{
    m_process.write("protocol_version\n");
    return waitResponse() ? m_response.toInt() : -1;
}

QString GoEngine::engineVersion()
{
    m_process.write("version\n");
    return waitResponse() ? m_response : QString();
}

bool GoEngine::setBoardSize(int size)
{
    Q_ASSERT(size >= 1 && size <= 19);

    m_process.write("boardsize " + QByteArray::number(size) + '\n');
    if (waitResponse()) {
        // Changing size wipes the board, start again with black player.
        changeCurrentPlayer(BlackPlayer);
        m_fixedHandicap = 0;
        emit boardSizeChanged(size);
        emit boardChanged();
        return true;
    } else
        return false;
}

int GoEngine::boardSize()
{
    m_process.write("query_boardsize\n");
    return waitResponse() ? m_response.toInt() : -1;
}

bool GoEngine::clearBoard()
{
    m_process.write("clear_board\n");
    if (waitResponse()) {
        //: The board is wiped empty, start again with black player
        changeCurrentPlayer(BlackPlayer);
        m_fixedHandicap = 0;
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::setKomi(float komi)
{
    Q_ASSERT(komi >= 0);

    m_process.write("komi " + QByteArray::number(komi) + '\n');
    if (waitResponse()) {
        m_komi = komi;
        return true;
    } else
        return false;
}

void GoEngine::setPlayerStrength(PlayerColor color, int strength)
{
    switch (color) {
        case WhitePlayer: m_whitePlayerStrength = strength; break;
        case BlackPlayer: m_blackPlayerStrength = strength; break;
        case InvalidPlayer: break;
    }
}

int GoEngine::playerStrength(PlayerColor color) const
{
    if (color == WhitePlayer)
        return m_whitePlayerStrength;
    else if (color == BlackPlayer)
        return m_blackPlayerStrength;
    else
        return -1;
}

void GoEngine::setPlayerType(PlayerColor color, PlayerType type)
{
    switch (color) {
        case WhitePlayer: m_whitePlayerType = type; break;
        case BlackPlayer: m_blackPlayerType = type; break;
        case InvalidPlayer: break;
    }
}

GoEngine::PlayerType GoEngine::playerType(PlayerColor color) const
{
    if (color == WhitePlayer)
        return m_whitePlayerType;
    else if (color == BlackPlayer)
        return m_blackPlayerType;
    else
        return HumanPlayer;
}

bool GoEngine::setFixedHandicap(int handicap)
{
    Q_ASSERT(handicap >= 2 && handicap <= 9);

    if (handicap <= fixedHandicapMax()) {
        m_process.write("fixed_handicap " + QByteArray::number(handicap) + '\n');
        if (waitResponse()) {
            // Black starts with setting his (fixed) handicap as it's first turn
            // which means, white is next.
            changeCurrentPlayer(WhitePlayer);
            m_fixedHandicap = handicap;
            emit boardChanged();
            return true;
        } else
            return false;
    } else {
        kWarning() << "Handicap" << handicap << " not set, it is too high!";
        return false;
    }
}

int GoEngine::fixedHandicapMax()
{
    // These values are handcrafted and reflect what GnuGo accepts
    switch (boardSize()) {
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

bool GoEngine::playMove(const Stone &field, PlayerColor color)
{
    if (!field.isValid())
        return false;
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("play ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1() + '\n');
    m_process.write(msg);
    if (waitResponse()) {
        color == WhitePlayer ? changeCurrentPlayer(BlackPlayer) : changeCurrentPlayer(WhitePlayer);
        m_moveNumber++;
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::passMove(PlayerColor color)
{
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("play ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append("pass\n");
    m_process.write(msg);
    if (waitResponse()) {
        color == WhitePlayer ? changeCurrentPlayer(BlackPlayer) : changeCurrentPlayer(WhitePlayer);
        m_moveNumber++;
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::generateMove(PlayerColor color)
{
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("genmove ");
    if (color == WhitePlayer) {
        m_process.write("level " + QByteArray::number(m_whitePlayerStrength) + '\n');
        waitResponse(); // Setting level is not mission-critical, no error checking
        msg.append("white\n");
    } else {
        m_process.write("level " + QByteArray::number(m_blackPlayerStrength) + '\n');
        waitResponse(); // Setting level is not mission-critical, no error checking
        msg.append("black\n");
    }
    m_process.write(msg);
    if (waitResponse()) {
        color == WhitePlayer ? changeCurrentPlayer(BlackPlayer) : changeCurrentPlayer(WhitePlayer);
        m_moveNumber++;
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::undoMove(int i)
{
    Q_ASSERT(i >= 0);

    m_process.write("undo " + QByteArray::number(i) + '\n');
    if (waitResponse()) {
        m_process.write("last_move\n");
        if (waitResponse()) {
            if (m_response.startsWith("white")) {
                changeCurrentPlayer(WhitePlayer);
            } else if (m_response.startsWith("black")) {
                changeCurrentPlayer(BlackPlayer);
            } else {
                // No last move means we're at the beginning of the game. The current player
                // depends on whether there black set a fixed handicap (white next) or not.
                if (m_fixedHandicap > 0)
                    changeCurrentPlayer(WhitePlayer);
                else
                    changeCurrentPlayer(BlackPlayer);
            }
        }
        m_moveNumber--;
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::tryMove(const Stone &field, PlayerColor color)
{
    if (!field.isValid())
        return false;
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("trymove ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1() + '\n');
    m_process.write(msg);
    if (waitResponse()) {
        color == WhitePlayer ? changeCurrentPlayer(BlackPlayer) : changeCurrentPlayer(WhitePlayer);
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::popGo()
{
    m_process.write("popgo\n");
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

GoEngine::PlayerColor GoEngine::currentPlayer() const
{
    return m_currentPlayer;
}

QPair<GoEngine::Stone, GoEngine::PlayerColor> GoEngine::lastMove()
{
    QPair<Stone, PlayerColor> pair(Stone(), InvalidPlayer);

    m_process.write("last_move\n");
    if (waitResponse()) {
        if (m_response.startsWith("white")) {
            pair.first = Stone(m_response.split(' ')[1]);
            pair.second = WhitePlayer;
        } else if (m_response.startsWith("black")) {
            pair.first = Stone(m_response.split(' ')[1]);
            pair.second = BlackPlayer;
        }
    }
    return pair;
}

GoEngine::FieldStatus GoEngine::whatColor(const Stone &field)
{
    if (!field.isValid())
        return InvalidField;

    m_process.write("color " + field.toLatin1() + '\n');
    if (waitResponse()) {
        if (m_response == "white") return WhiteField;
        else if (m_response == "black") return BlackField;
        else if (m_response == "empty") return EmptyField;
        else return InvalidField;
    } else
        return InvalidField;
}

QList<GoEngine::Stone> GoEngine::listStones(PlayerColor color)
{
    QList<Stone> list;
    if (color == InvalidPlayer)
        return list;

    QByteArray msg("list_stones ");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &pos, m_response.split(' '))
            list.append(Stone(pos));
    }
    return list;
}

QList<QPair<GoEngine::Stone, GoEngine::PlayerColor> > GoEngine::moveHistory()
{
    QList<QPair<GoEngine::Stone, GoEngine::PlayerColor> > list;

    m_process.write("move_history\n");
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split('\n')) {
            QStringList parts = entry.split(' ');
            if (parts.size() == 2) {
                if(parts[0] == "white")
                    list.prepend(QPair<Stone, PlayerColor>(Stone(parts[1]), WhitePlayer));
                else if (parts[0] == "black")
                    list.prepend(QPair<Stone, PlayerColor>(Stone(parts[1]), BlackPlayer));
            }
        }
    }
    return list;
}

QList<GoEngine::Stone> GoEngine::moveHistory(PlayerColor color)
{
    QList<Stone> list;
    if (color == InvalidPlayer)
        return list;

    m_process.write("move_history\n");
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split('\n')) {
            QStringList parts = entry.split(' ');
            if (parts.size() == 2)
                if (color == WhitePlayer && parts[0] == "white" || color == BlackPlayer && parts[0] == "black")
                    list.prepend(Stone(parts[1]));
        }
    }
    return list;
}

int GoEngine::countLiberties(const Stone &field)
{
    if (!field.isValid())
        return -1;

    m_process.write("countlib " + field.toLatin1() + '\n');
    return waitResponse() ? m_response.toInt() : -1;
}

QList<GoEngine::Stone> GoEngine::findLiberties(const Stone &field)
{
    QList<GoEngine::Stone> list;
    if (!field.isValid())
        return list;

    m_process.write("findlib " + field.toLatin1() + '\n');
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split(' '))
            list.append(GoEngine::Stone(entry));
    }
    return list;
}

bool GoEngine::isLegal(const Stone &field, PlayerColor color)
{
    if (!field.isValid())
        return false;
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("is_legal ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1() + '\n');
    m_process.write(msg);
    return waitResponse() && m_response == "1";
}

QList<QPair<GoEngine::Stone, float> > GoEngine::topMoves(PlayerColor color)
{
    QList<QPair<Stone, float> > list;
    if (color == InvalidPlayer)
        return list;

    if (color == WhitePlayer)
        m_process.write("top_moves_white\n");
    else
        m_process.write("top_moves_black\n");
    if (waitResponse() && !m_response.isEmpty()) {
        QStringList parts = m_response.split(' ');
        if (parts.size() % 2 == 0)
            for (int i = 0; i < parts.size(); i += 2)
                list.append(QPair<Stone, float>(Stone(parts[i]), QString(parts[i + 1]).toFloat()));
    }
    return list;
}

QList<GoEngine::Stone> GoEngine::legalMoves(PlayerColor color)
{
    QList<GoEngine::Stone> list;
    if (color == InvalidPlayer)
        return list;

    QByteArray msg("all_legal ");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

int GoEngine::captures(PlayerColor color)
{
    if (color == InvalidPlayer)
        return 0;

    QByteArray msg("captures ");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    return waitResponse() ? m_response.toInt() : 0;
}

QString GoEngine::attack(const Stone &field)
{
    if (!field.isValid())
        return QString();

    m_process.write("attack " + field.toLatin1() + '\n');
    return waitResponse() ? m_response : QString();
}

QString GoEngine::defend(const Stone &field)
{
    if (!field.isValid())
        return QString();

    m_process.write("defend " + field.toLatin1() + '\n');
    return waitResponse() ? m_response : QString();
}

bool GoEngine::increaseDepths()
{
    m_process.write("increase_depths\n");
    return waitResponse();
}

bool GoEngine::decreaseDepths()
{
    m_process.write("decrease_depths\n");
    return waitResponse();
}

QString GoEngine::owlAttack(const Stone &field)
{
    if (!field.isValid())
        return QString();

    m_process.write("owl_attack " + field.toLatin1() + '\n');
    return waitResponse() ? m_response : QString();
}

QString GoEngine::owlDefense(const Stone &field)
{
    if (!field.isValid())
        return QString();

    m_process.write("owl_defend " + field.toLatin1() + '\n');
    return waitResponse() ? m_response : QString();
}

QString GoEngine::evalEye(const Stone &field)
{
    if (!field.isValid())
        return QString();

    m_process.write("eval_eye " + field.toLatin1() + '\n');
    return waitResponse() ? m_response : QString();
}

GoEngine::DragonStatus GoEngine::dragonStatus(const Stone &field)
{
    if (!field.isValid())
        return UnknownDragon;

    m_process.write("dragon_status " + field.toLatin1() + '\n');
    if (waitResponse()) {
        if (m_response == "alive") return AliveDragon;
        else if (m_response == "critical") return CriticalDragon;
        else if (m_response == "dead") return DeadDragon;
        else if (m_response == "unknown") return UnknownDragon;
        else return UnknownDragon;  // Should never happen
    } else
        return UnknownDragon;
}

bool GoEngine::isSameDragon(const Stone &field1, const Stone &field2)
{
    if (!field1.isValid() || !field2.isValid())
        return false;

    m_process.write("same_dragon " + field1.toLatin1() + ' ' + field2.toLatin1() + '\n');
    return waitResponse() && m_response == "1";
}

QString GoEngine::dragonData(const Stone &field)
{
    QByteArray msg("dragon_data ");
    if (field.isValid())
        msg.append(field.toLatin1());
    msg.append('\n');
    m_process.write(msg);
    return waitResponse() ? m_response : QString();
}

GoEngine::FinalState GoEngine::finalStatus(const Stone &field)
{
    if (!field.isValid())
        return FinalStateInvalid;

    m_process.write("final_status " + field.toLatin1() + '\n');
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

QList<GoEngine::Stone> GoEngine::finalStatusList(FinalState state)
{
    QList<Stone> list;
    if (state == FinalStateInvalid)
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

GoEngine::Score GoEngine::finalScore()
{
    m_process.write("final_score\n");
    return waitResponse() ? Score(m_response) : Score();
}

GoEngine::Score GoEngine::estimateScore()
{
    m_process.write("estimate_score\n");
    return waitResponse() ? Score(m_response) : Score();
}

int GoEngine::getLifeNodeCounter()
{
    m_process.write("get_life_node_counter\n");
    return waitResponse() ? m_response.toInt() : -1;
}

bool GoEngine::resetLifeNodeCounter()
{
    m_process.write("reset_life_node_counter\n");
    return waitResponse();
}

int GoEngine::getOwlNodeCounter()
{
    m_process.write("get_owl_node_counter\n");
    return waitResponse() ? m_response.toInt() : -1;
}

bool GoEngine::resetOwlNodeCounter()
{
    m_process.write("reset_owl_node_counter\n");
    return waitResponse();
}

int GoEngine::getReadingNodeCounter()
{
    m_process.write("get_reading_node_counter\n");
    return waitResponse() ? m_response.toInt() : -1;
}

bool GoEngine::resetReadingNodeCounter()
{
    m_process.write("reset_reading_node_counter\n");
    return waitResponse();
}

int GoEngine::getTryMoveCounter()
{
    m_process.write("get_trymove_counter\n");
    return waitResponse() ? m_response.toInt() : -1;
}

bool GoEngine::resetTryMoveCounter()
{
    m_process.write("reset_trymove_counter\n");
    return waitResponse();
}

bool GoEngine::showBoard()
{
    m_process.write("showboard\n");
    return waitResponse();
}

bool GoEngine::dumpStack()
{
    m_process.write("dump_stack\n");
    return waitResponse();
}

int GoEngine::wormCutStone(const Stone &field)
{
    if (!field.isValid())
        return -1;

    m_process.write("worm_cutstone " + field.toLatin1() + '\n');
    return waitResponse() ? m_response.toInt() : -1;
}

QString GoEngine::wormData(const Stone &field)
{
    QByteArray msg("worm_data ");
    if (field.isValid())
        msg.append(field.toLatin1());
    msg.append('\n');
    m_process.write(msg);
    return waitResponse() ? m_response : QString();
}

QList<GoEngine::Stone> GoEngine::wormStones(const Stone &field)
{
    QList<Stone> list;
    if (!field.isValid())
        return list;

    m_process.write("worm_stones " + field.toLatin1() + '\n');
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

bool GoEngine::tuneMoveOrdering(int parameters)
{
    m_process.write("tune_move_ordering " + QByteArray::number(parameters) + '\n');
    return waitResponse();
}

QList<QString> GoEngine::help()
{
    m_process.write("help\n");
    QList<QString> list;
    if (waitResponse() && !m_response.isEmpty()) {
        foreach (const QString &entry, m_response.split('\n'))
            list.append(entry);
    }
    return list;
}

bool GoEngine::reportUncertainty(bool enabled)
{
    QByteArray msg("report_uncertainty ");
    msg.append(enabled ? "on" : "off");
    msg.append('\n');
    m_process.write(msg);
    return waitResponse();
}

QString GoEngine::shell(const QString &command)
{
    if (command.isEmpty())
        return QString();

    m_process.write(command.toLatin1() + '\n');
    return waitResponse() ? m_response : QString();
}

bool GoEngine::knownCommand(const QString &command)
{
    if (command.isEmpty())
        return false;

    m_process.write("known_command " + command.toLatin1() + '\n');
    return waitResponse();
}

QString GoEngine::echo(const QString &command)
{
    if (command.isEmpty())
        return QString();

    m_process.write("echo " + command.toLatin1() + '\n');
    return waitResponse() ? m_response : QString();
}

bool GoEngine::waitResponse()
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

    // Block and wait till command execution finished. We have to do this till '\n\n' arives in our
    // input buffer to show that the Go engine is done processing our request.
    m_response.clear();
    emit waitStarted();
    do {
        m_process.waitForReadyRead();
        m_response += m_process.readAllStandardOutput();
    } while(!m_response.endsWith("\n\n"));
    emit waitFinished();

    if (m_response.size() < 1)
        return false;
    QChar tmp = m_response[0];                      // First message character indicates success or error
    m_response.remove(0, 2);                        // Remove the first two chars (e.g. "? " or "= ")
    m_response = m_response.trimmed();              // Remove further whitespaces, newlines, ...
    return tmp != '?';                              // '?' Means the engine didn't understand the query
}

void GoEngine::changeCurrentPlayer(PlayerColor color)
{
    m_currentPlayer = color;
    emit currentPlayerChanged(m_currentPlayer);
}

} // End of namespace KGo

#include "moc_goengine.cpp"
