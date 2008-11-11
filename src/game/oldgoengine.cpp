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

#include "oldgoengine.h"

#include <KDebug>

#include <QFile>

namespace KGo {

OldGoEngine::Stone::Stone(const QString &stone)
    : m_x(0), m_y(0)
{
    if (stone.size() >= 2) {
        m_x = stone[0].toUpper().toLatin1();
        m_y = stone.mid(1).toInt();
    }
}

bool OldGoEngine::Stone::isValid() const
{
    // Go coordinates are somewhat complicated ...
    return m_y >= 1 && m_y <= 19 && m_x >= 'A' && m_x != 'I' && m_x <= 'T';
}

QByteArray OldGoEngine::Stone::toLatin1() const
{
    QByteArray msg;
    msg.append(m_x);
    msg.append(QByteArray::number(m_y));
    return msg;
}

QString OldGoEngine::Stone::toString() const
{
    return QString(m_x + QString::number(m_y));
}

////////////////////////////////////////////////////////////////////

OldGoEngine::Score::Score(const QString &scoreString)
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

bool OldGoEngine::Score::isValid() const
{
    return m_score >= 0 && (m_player == WhitePlayer || m_player == BlackPlayer);
}

QString OldGoEngine::Score::toString() const
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

OldGoEngine::OldGoEngine()
    : m_currentPlayer(BlackPlayer)
    , m_whitePlayerType(HumanPlayer), m_whitePlayerStrength(10)
    , m_blackPlayerType(HumanPlayer), m_blackPlayerStrength(10)
    , m_komi(0), m_fixedHandicap(0), m_moveNumber(0), m_consecutivePassMoveNumber(0)
{
}

OldGoEngine::~OldGoEngine()
{
    quitEngine();
}

bool OldGoEngine::startEngine(const QString &command)
{
    quitEngine();                                   // Close old session if there's one
    m_engineProcess.start(command.toLatin1());      // Start new process with provided command
    if (!m_engineProcess.waitForStarted()) {        // Blocking wait for process start
        m_engineResponse = "Unable to execute command: " + command;
        kDebug() << m_engineResponse;
        return false;
    }
    m_engineCommand = command;                      // Save for retrieval
    kDebug() << "Engine" << m_engineCommand << "started...";

    // Test if we started a GTP-compatible Go engine
    m_engineProcess.write("name\n");
    m_engineProcess.waitForReadyRead();
    QString response = m_engineProcess.readAllStandardOutput();
    if (response.isEmpty() || !response.startsWith("=")) {
        m_engineResponse = "Engine did not respond to GTP command \"name\"";
        kDebug() << m_engineResponse;
        quitEngine();
        return false;
    }
    kDebug() << "Engine is a GTP-compatible Go engine";

    // Check for supported GTP protocol version
    if (engineProtocol() < 0) {
        m_engineResponse = "Engine does not support correct GTP version";
        kDebug() << m_engineResponse;
        quitEngine();
        return false;
    }
    kDebug() << "Engine supports correct GTP version";
    return true;
}

void OldGoEngine::quitEngine()
{
    if (m_engineProcess.isOpen()) {
        kDebug() << "Quit GTP engine session";
        m_engineProcess.write("quit\n");
        m_engineProcess.close();
    }
}

QString OldGoEngine::engineName()
{
    if (!isRunning())
        return QString();

    m_engineProcess.write("name\n");
    return waitResponse() ? m_engineResponse : QString();
}

int OldGoEngine::engineProtocol()
{
    if (!isRunning())
        return -1;

    m_engineProcess.write("protocol_version\n");
    return waitResponse() ? m_engineResponse.toInt() : -1;
}

QString OldGoEngine::engineVersion()
{
    if (!isRunning())
        return QString();

    m_engineProcess.write("version\n");
    return waitResponse() ? m_engineResponse : QString();
}

bool OldGoEngine::loadGameFromSGF(const QString &fileName, int moveNumber)
{
    Q_ASSERT(moveNumber >= 0);
    if (!isRunning() || fileName.isEmpty() || !QFile::exists(fileName))
        return false;

    m_engineProcess.write("loadsgf " + fileName.toLatin1() + ' ' + QByteArray::number(moveNumber) + '\n');
    if (waitResponse()) {
        m_moveNumber = moveNumber;                      // Store move number
        if (m_engineResponse.startsWith("white"))       // Check which player is current
            setCurrentPlayer(WhitePlayer);
        else if (m_engineResponse.startsWith("black"))
            setCurrentPlayer(BlackPlayer);
        else
            setCurrentPlayer(InvalidPlayer);

        m_engineProcess.write("get_komi\n");            // Query komi from engine and store it
        if (waitResponse())
            m_komi = m_engineResponse.toFloat();
        m_engineProcess.write("get_handicap\n");        // Query fixed handicap and store it
        if (waitResponse())
            m_fixedHandicap = m_engineResponse.toInt();
        kDebug() << "Loaded komi is" << m_komi << "and handicap is" << m_fixedHandicap;

        emit boardChanged();                            // All done, tell the world!
        return true;
    } else
        return false;
}

bool OldGoEngine::saveGameToSGF(const QString &fileName)
{
    if (!isRunning() || fileName.isEmpty())
        return false;

    m_engineProcess.write("printsgf " + fileName.toLatin1() + '\n');
    return waitResponse();
}

bool OldGoEngine::setBoardSize(int size)
{
    Q_ASSERT(size >= 1 && size <= 19);
    if (!isRunning())
        return false;

    m_engineProcess.write("boardsize " + QByteArray::number(size) + '\n');
    if (waitResponse()) {
        // Changing size wipes the board, start again with black player.
        setCurrentPlayer(BlackPlayer);
        m_fixedHandicap = 0;
        m_moveNumber = 0;
        emit boardSizeChanged(size);
        emit boardChanged();
        return true;
    } else
        return false;
}

int OldGoEngine::boardSize()
{
    if (!isRunning())
        return -1;

    m_engineProcess.write("query_boardsize\n");
    return waitResponse() ? m_engineResponse.toInt() : -1;
}

bool OldGoEngine::clearBoard()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("clear_board\n");
    if (waitResponse()) {
        //: The board is wiped empty, start again with black player
        setCurrentPlayer(BlackPlayer);
        m_fixedHandicap = 0;
        m_moveNumber = 0;
        emit boardChanged();
        return true;
    } else
        return false;
}

bool OldGoEngine::setKomi(float komi)
{
    Q_ASSERT(komi >= 0);
    if (!isRunning())
        return false;

    m_engineProcess.write("komi " + QByteArray::number(komi) + '\n');
    if (waitResponse()) {
        m_komi = komi;
        return true;
    } else
        return false;
}

void OldGoEngine::setPlayerStrength(PlayerColor color, int strength)
{
    switch (color) {
        case WhitePlayer: m_whitePlayerStrength = strength; break;
        case BlackPlayer: m_blackPlayerStrength = strength; break;
        case InvalidPlayer: break;
    }
}

int OldGoEngine::playerStrength(PlayerColor color) const
{
    if (color == WhitePlayer)
        return m_whitePlayerStrength;
    else if (color == BlackPlayer)
        return m_blackPlayerStrength;
    else
        return -1;
}

void OldGoEngine::setPlayerType(PlayerColor color, PlayerType type)
{
    switch (color) {
        case WhitePlayer: m_whitePlayerType = type; break;
        case BlackPlayer: m_blackPlayerType = type; break;
        case InvalidPlayer: break;
    }
}

OldGoEngine::PlayerType OldGoEngine::playerType(PlayerColor color) const
{
    if (color == WhitePlayer)
        return m_whitePlayerType;
    else if (color == BlackPlayer)
        return m_blackPlayerType;
    else
        return HumanPlayer;
}

bool OldGoEngine::setFixedHandicap(int handicap)
{
    Q_ASSERT(handicap >= 2 && handicap <= 9);
    if (!isRunning())
        return false;

    if (handicap <= fixedHandicapMax()) {
        m_engineProcess.write("fixed_handicap " + QByteArray::number(handicap) + '\n');
        if (waitResponse()) {
            // Black starts with setting his (fixed) handicap as it's first turn
            // which means, white is next.
            setCurrentPlayer(WhitePlayer);
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

int OldGoEngine::fixedHandicapMax()
{
    if (!isRunning())
        return 0;

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

bool OldGoEngine::playMove(const Stone &field, PlayerColor color)
{
    if (!isRunning() || !field.isValid())
        return false;
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("play ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1() + '\n');
    m_engineProcess.write(msg);
    if (waitResponse()) {
        color == WhitePlayer ? setCurrentPlayer(BlackPlayer) : setCurrentPlayer(WhitePlayer);
        m_moveNumber++;
        m_consecutivePassMoveNumber = 0;
        emit boardChanged();
        return true;
    } else
        return false;
}

bool OldGoEngine::passMove(PlayerColor color)
{
    if (!isRunning())
        return false;
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("play ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append("pass\n");
    m_engineProcess.write(msg);
    if (waitResponse()) {
        color == WhitePlayer ? setCurrentPlayer(BlackPlayer) : setCurrentPlayer(WhitePlayer);
        m_moveNumber++;
        if (m_consecutivePassMoveNumber > 0)
            emit consecutivePassMovesPlayed(m_consecutivePassMoveNumber);
        m_consecutivePassMoveNumber++;
        emit boardChanged();
        return true;
    } else
        return false;
}

bool OldGoEngine::generateMove(PlayerColor color)
{
    if (!isRunning())
        return false;
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("genmove ");
    if (color == WhitePlayer) {
        m_engineProcess.write("level " + QByteArray::number(m_whitePlayerStrength) + '\n');
        waitResponse(); // Setting level is not mission-critical, no error checking
        msg.append("white\n");
    } else {
        m_engineProcess.write("level " + QByteArray::number(m_blackPlayerStrength) + '\n');
        waitResponse(); // Setting level is not mission-critical, no error checking
        msg.append("black\n");
    }
    m_engineProcess.write(msg);
    if (waitResponse()) {
        color == WhitePlayer ? setCurrentPlayer(BlackPlayer) : setCurrentPlayer(WhitePlayer);

        if (m_engineResponse == "PASS") {
            m_moveNumber++;
            if (m_consecutivePassMoveNumber > 0)
                emit consecutivePassMovesPlayed(m_consecutivePassMoveNumber);
            m_consecutivePassMoveNumber++;
        } else if (m_engineResponse == "resign") {
            emit playerResigned(m_currentPlayer);
        } else {
            m_moveNumber++;
            m_consecutivePassMoveNumber = 0;
            emit boardChanged();
        }
        return true;
    } else
        return false;
}

bool OldGoEngine::undoMove()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("last_move\n");
    if (waitResponse()) {
        QString lastMove = m_engineResponse;
        m_engineProcess.write("undo\n");
        if (waitResponse()) {
            if (lastMove.startsWith("white"))
                setCurrentPlayer(WhitePlayer);
            else if (lastMove.startsWith("black"))
                setCurrentPlayer(BlackPlayer);
            m_moveNumber--;
            if (m_consecutivePassMoveNumber > 0)
                m_consecutivePassMoveNumber--;
            emit boardChanged();
        }
        return true;
    } else
        return false;
}

/*bool OldGoEngine::tryMove(const Stone &field, PlayerColor color)
{
    if (!isRunning() || !field.isValid())
        return false;
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("trymove ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1() + '\n');
    m_engineProcess.write(msg);
    if (waitResponse()) {
        color == WhitePlayer ? setCurrentPlayer(BlackPlayer) : setCurrentPlayer(WhitePlayer);
        emit boardChanged();
        return true;
    } else
        return false;
}

bool OldGoEngine::popGo()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("popgo\n");
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}*/

OldGoEngine::PlayerColor OldGoEngine::currentPlayer() const
{
    return m_currentPlayer;
}

QPair<OldGoEngine::Stone, OldGoEngine::PlayerColor> OldGoEngine::lastMove()
{
    QPair<Stone, PlayerColor> pair(Stone(), InvalidPlayer);
    if (!isRunning())
        return pair;

    m_engineProcess.write("last_move\n");
    if (waitResponse()) {
        if (m_engineResponse.startsWith("white")) {
            pair.first = Stone(m_engineResponse.split(' ')[1]);
            pair.second = WhitePlayer;
        } else if (m_engineResponse.startsWith("black")) {
            pair.first = Stone(m_engineResponse.split(' ')[1]);
            pair.second = BlackPlayer;
        }
    }
    return pair;
}

OldGoEngine::FieldStatus OldGoEngine::whatColor(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return InvalidField;

    m_engineProcess.write("color " + field.toLatin1() + '\n');
    if (waitResponse()) {
        if (m_engineResponse == "white") return WhiteField;
        else if (m_engineResponse == "black") return BlackField;
        else if (m_engineResponse == "empty") return EmptyField;
        else return InvalidField;
    } else
        return InvalidField;
}

QList<OldGoEngine::Stone> OldGoEngine::listStones(PlayerColor color)
{
    QList<Stone> list;
    if (!isRunning() || color == InvalidPlayer)
        return list;

    QByteArray msg("list_stones ");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_engineProcess.write(msg);
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        foreach (const QString &pos, m_engineResponse.split(' '))
            list.append(Stone(pos));
    }
    return list;
}

QList<QPair<OldGoEngine::Stone, OldGoEngine::PlayerColor> > OldGoEngine::moveHistory()
{
    QList<QPair<OldGoEngine::Stone, OldGoEngine::PlayerColor> > list;
    if (!isRunning())
        return list;

    m_engineProcess.write("move_history\n");
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        foreach (const QString &entry, m_engineResponse.split('\n')) {
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

QList<OldGoEngine::Stone> OldGoEngine::moveHistory(PlayerColor color)
{
    QList<Stone> list;
    if (!isRunning() || color == InvalidPlayer)
        return list;

    m_engineProcess.write("move_history\n");
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        foreach (const QString &entry, m_engineResponse.split('\n')) {
            QStringList parts = entry.split(' ');
            if (parts.size() == 2)
                if ((color == WhitePlayer && parts[0] == "white") || (color == BlackPlayer && parts[0] == "black"))
                    list.prepend(Stone(parts[1]));
        }
    }
    return list;
}

int OldGoEngine::countLiberties(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return -1;

    m_engineProcess.write("countlib " + field.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse.toInt() : -1;
}

QList<OldGoEngine::Stone> OldGoEngine::findLiberties(const Stone &field)
{
    QList<OldGoEngine::Stone> list;
    if (!isRunning() || !field.isValid())
        return list;

    m_engineProcess.write("findlib " + field.toLatin1() + '\n');
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        foreach (const QString &entry, m_engineResponse.split(' '))
            list.append(OldGoEngine::Stone(entry));
    }
    return list;
}

bool OldGoEngine::isLegal(const Stone &field, PlayerColor color)
{
    if (!isRunning() || !field.isValid())
        return false;
    if (color == InvalidPlayer)
        color = m_currentPlayer;

    QByteArray msg("is_legal ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1() + '\n');
    m_engineProcess.write(msg);
    return waitResponse() && m_engineResponse == "1";
}

QList<QPair<OldGoEngine::Stone, float> > OldGoEngine::topMoves(PlayerColor color)
{
    QList<QPair<Stone, float> > list;
    if (!isRunning() || color == InvalidPlayer)
        return list;

    if (color == WhitePlayer)
        m_engineProcess.write("top_moves_white\n");
    else
        m_engineProcess.write("top_moves_black\n");
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        QStringList parts = m_engineResponse.split(' ');
        if (parts.size() % 2 == 0)
            for (int i = 0; i < parts.size(); i += 2)
                list.append(QPair<Stone, float>(Stone(parts[i]), QString(parts[i + 1]).toFloat()));
    }
    return list;
}

QList<OldGoEngine::Stone> OldGoEngine::legalMoves(PlayerColor color)
{
    QList<OldGoEngine::Stone> list;
    if (!isRunning() || color == InvalidPlayer)
        return list;

    QByteArray msg("all_legal ");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_engineProcess.write(msg);
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        foreach (const QString &entry, m_engineResponse.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

int OldGoEngine::captures(PlayerColor color)
{
    if (!isRunning() || color == InvalidPlayer)
        return 0;

    QByteArray msg("captures ");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_engineProcess.write(msg);
    return waitResponse() ? m_engineResponse.toInt() : 0;
}

QString OldGoEngine::attack(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return QString();

    m_engineProcess.write("attack " + field.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse : QString();
}

QString OldGoEngine::defend(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return QString();

    m_engineProcess.write("defend " + field.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse : QString();
}

bool OldGoEngine::increaseDepths()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("increase_depths\n");
    return waitResponse();
}

bool OldGoEngine::decreaseDepths()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("decrease_depths\n");
    return waitResponse();
}

QString OldGoEngine::owlAttack(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return QString();

    m_engineProcess.write("owl_attack " + field.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse : QString();
}

QString OldGoEngine::owlDefense(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return QString();

    m_engineProcess.write("owl_defend " + field.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse : QString();
}

QString OldGoEngine::evalEye(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return QString();

    m_engineProcess.write("eval_eye " + field.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse : QString();
}

OldGoEngine::DragonStatus OldGoEngine::dragonStatus(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return UnknownDragon;

    m_engineProcess.write("dragon_status " + field.toLatin1() + '\n');
    if (waitResponse()) {
        if (m_engineResponse == "alive") return AliveDragon;
        else if (m_engineResponse == "critical") return CriticalDragon;
        else if (m_engineResponse == "dead") return DeadDragon;
        else if (m_engineResponse == "unknown") return UnknownDragon;
        else return UnknownDragon;  // Should never happen
    } else
        return UnknownDragon;
}

bool OldGoEngine::isSameDragon(const Stone &field1, const Stone &field2)
{
    if (!isRunning() || !field1.isValid() || !field2.isValid())
        return false;

    m_engineProcess.write("same_dragon " + field1.toLatin1() + ' ' + field2.toLatin1() + '\n');
    return waitResponse() && m_engineResponse == "1";
}

QString OldGoEngine::dragonData(const Stone &field)
{
    if (!isRunning())
        return QString();

    QByteArray msg("dragon_data ");
    if (field.isValid())
        msg.append(field.toLatin1());
    msg.append('\n');
    m_engineProcess.write(msg);
    return waitResponse() ? m_engineResponse : QString();
}

OldGoEngine::FinalState OldGoEngine::finalStatus(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return FinalStateInvalid;

    m_engineProcess.write("final_status " + field.toLatin1() + '\n');
    if (waitResponse()) {
        if (m_engineResponse == "alive") return FinalAlive;
        else if (m_engineResponse == "dead") return FinalDead;
        else if (m_engineResponse == "seki") return FinalSeki;
        else if (m_engineResponse == "white_territory") return FinalWhiteTerritory;
        else if (m_engineResponse == "blacK_territory") return FinalBlackTerritory;
        else if (m_engineResponse == "dame") return FinalDame;
        else return FinalStateInvalid;
    } else
        return FinalStateInvalid;
}

QList<OldGoEngine::Stone> OldGoEngine::finalStatusList(FinalState state)
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
    m_engineProcess.write(msg);
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        foreach (const QString &entry, m_engineResponse.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

OldGoEngine::Score OldGoEngine::finalScore()
{
    if (!isRunning())
        return Score();

    m_engineProcess.write("final_score\n");
    return waitResponse() ? Score(m_engineResponse) : Score();
}

OldGoEngine::Score OldGoEngine::estimateScore()
{
    if (!isRunning())
        return Score();

    m_engineProcess.write("estimate_score\n");
    return waitResponse() ? Score(m_engineResponse) : Score();
}

int OldGoEngine::getLifeNodeCounter()
{
    if (!isRunning())
        return -1;

    m_engineProcess.write("get_life_node_counter\n");
    return waitResponse() ? m_engineResponse.toInt() : -1;
}

bool OldGoEngine::resetLifeNodeCounter()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("reset_life_node_counter\n");
    return waitResponse();
}

int OldGoEngine::getOwlNodeCounter()
{
    if (!isRunning())
        return -1;

    m_engineProcess.write("get_owl_node_counter\n");
    return waitResponse() ? m_engineResponse.toInt() : -1;
}

bool OldGoEngine::resetOwlNodeCounter()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("reset_owl_node_counter\n");
    return waitResponse();
}

int OldGoEngine::getReadingNodeCounter()
{
    if (!isRunning())
        return -1;

    m_engineProcess.write("get_reading_node_counter\n");
    return waitResponse() ? m_engineResponse.toInt() : -1;
}

bool OldGoEngine::resetReadingNodeCounter()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("reset_reading_node_counter\n");
    return waitResponse();
}

int OldGoEngine::getTryMoveCounter()
{
    if (!isRunning())
        return -1;

    m_engineProcess.write("get_trymove_counter\n");
    return waitResponse() ? m_engineResponse.toInt() : -1;
}

bool OldGoEngine::resetTryMoveCounter()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("reset_trymove_counter\n");
    return waitResponse();
}

bool OldGoEngine::showBoard()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("showboard\n");
    return waitResponse();
}

bool OldGoEngine::dumpStack()
{
    if (!isRunning())
        return false;

    m_engineProcess.write("dump_stack\n");
    return waitResponse();
}

int OldGoEngine::wormCutStone(const Stone &field)
{
    if (!isRunning() || !field.isValid())
        return -1;

    m_engineProcess.write("worm_cutstone " + field.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse.toInt() : -1;
}

QString OldGoEngine::wormData(const Stone &field)
{
    if (!isRunning())
        return QString();

    QByteArray msg("worm_data ");
    if (field.isValid())
        msg.append(field.toLatin1());
    msg.append('\n');
    m_engineProcess.write(msg);
    return waitResponse() ? m_engineResponse : QString();
}

QList<OldGoEngine::Stone> OldGoEngine::wormStones(const Stone &field)
{
    QList<Stone> list;
    if (!isRunning() || !field.isValid())
        return list;

    m_engineProcess.write("worm_stones " + field.toLatin1() + '\n');
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        foreach (const QString &entry, m_engineResponse.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

bool OldGoEngine::tuneMoveOrdering(int parameters)
{
    if (!isRunning())
        return false;

    m_engineProcess.write("tune_move_ordering " + QByteArray::number(parameters) + '\n');
    return waitResponse();
}

QList<QString> OldGoEngine::help()
{
    QList<QString> list;
    if (!isRunning())
        return list;

    m_engineProcess.write("help\n");
    if (waitResponse() && !m_engineResponse.isEmpty()) {
        foreach (const QString &entry, m_engineResponse.split('\n'))
            list.append(entry);
    }
    return list;
}

bool OldGoEngine::reportUncertainty(bool enabled)
{
    if (!isRunning())
        return false;

    QByteArray msg("report_uncertainty ");
    msg.append(enabled ? "on" : "off");
    msg.append('\n');
    m_engineProcess.write(msg);
    return waitResponse();
}

QString OldGoEngine::shell(const QString &command)
{
    if (!isRunning() || command.isEmpty())
        return QString();

    m_engineProcess.write(command.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse : QString();
}

bool OldGoEngine::knownCommand(const QString &command)
{
    if (!isRunning() || command.isEmpty())
        return false;

    m_engineProcess.write("known_command " + command.toLatin1() + '\n');
    return waitResponse();
}

QString OldGoEngine::echo(const QString &command)
{
    if (!isRunning() || command.isEmpty())
        return QString();

    m_engineProcess.write("echo " + command.toLatin1() + '\n');
    return waitResponse() ? m_engineResponse : QString();
}

bool OldGoEngine::waitResponse()
{
    if (m_engineProcess.state() != QProcess::Running) {   // No GTP connection means no computing fun!
        switch (m_engineProcess.error()) {
            case QProcess::FailedToStart: m_engineResponse = "No Go engine is running!"; break;
            case QProcess::Crashed: m_engineResponse = "The Go engine crashed!"; break;
            case QProcess::Timedout: m_engineResponse = "The Go engine timed out!"; break;
            case QProcess::WriteError: m_engineResponse = m_engineProcess.readAllStandardError(); break;
            case QProcess::ReadError: m_engineResponse = m_engineProcess.readAllStandardError(); break;
            case QProcess::UnknownError: m_engineResponse = "Unknown error!"; break;
        }
        kWarning() << "Command failed:" << m_engineResponse;
        return false;
    }

    // Block and wait till command execution finished. We have to do this till '\n\n' arives in our
    // input buffer to show that the Go engine is done processing our request.
    m_engineResponse.clear();
    do {
        //qApp->processEvents();
        m_engineProcess.waitForReadyRead();
        m_engineResponse += m_engineProcess.readAllStandardOutput();
    } while(!m_engineResponse.endsWith("\n\n"));

    if (m_engineResponse.size() < 1)
        return false;
    QChar tmp = m_engineResponse[0];                // First message character indicates success or error
    m_engineResponse.remove(0, 2);                  // Remove the first two chars (e.g. "? " or "= ")
    m_engineResponse = m_engineResponse.trimmed();  // Remove further whitespaces, newlines, ...
    return tmp != '?';                              // '?' Means the engine didn't understand the query
}

void OldGoEngine::setCurrentPlayer(PlayerColor color)
{
    m_currentPlayer = color;
    emit currentPlayerChanged(m_currentPlayer);
}

} // End of namespace KGo

#include "moc_oldgoengine.cpp"
