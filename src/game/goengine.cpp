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

#include <QFile>
#include <QApplication>

#include <KDebug>

namespace KGo {

GoEngine::Stone::Stone(const QString &stone)
    : m_x(stone[0].toLatin1())
    , m_y(stone.mid(1).toInt())
{
}

bool GoEngine::Stone::isValid() const
{
    if (m_y >= 1 && m_y <= 19 && ((m_x >= 'a' && m_x < 't') || (m_x >= 'A' && m_x <= 'T')))
        return true;
    else
        return false;
}

QByteArray GoEngine::Stone::toLatin1() const
{
    QByteArray msg;
    msg.append(m_x);
    msg.append(m_y);
    return msg;
}

QString GoEngine::Stone::toString() const
{
    return QString("%1%2").arg(m_x).arg(m_y);
}

GoEngine::Score::Score(const QString &scoreString)
{
    if (scoreString[0] == 'W')
        m_player = WhitePlayer;
    else
        m_player = BlackPlayer;
    int i = scoreString.indexOf(' ');
    m_score = scoreString.mid(2, i - 1).toInt();
    //m_upperBound = scoreString.mid(
    //TODO: Implement Score class, the bounds should be considered optional and can
    // be the same as the score if none given
}

bool GoEngine::Score::isValid() const
{
    return m_score >= 0 ? true : false;
}

QString GoEngine::Score::toString() const
{
    return QString("%1+%2 (upper bound: %3, lower: %4").arg(m_player == WhitePlayer ? "W" : "B").arg(m_score).arg(m_upperBound).arg(m_lowerBound);
}

////////////////////////////////////////////////////////////////////

GoEngine::GoEngine()
{
    connect(&m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
    connect(&m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStandardError()));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), SIGNAL(error(QProcess::ProcessError)));
}

GoEngine::~GoEngine()
{
    quit();
}

bool GoEngine::run(const QString &command)
{
    quit();                                         // Close old session if there's one
    m_process.start(command.toLatin1());            // Start new process with provided command

    if (!m_process.waitForStarted())                // NOTE: Blocking wait for process start
        return false;

    kDebug() << "Run new GTP engine session";

    if (protocolVersion() == 2) {                   // We support only GTP version 2 for now
        clearBoard();                               // Start with blank board
    } else {
        quit();
        m_response = "Protocol version error";
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

    kDebug() << "Attempting to load move" << moveNumber << "from" << fileName;

    QByteArray msg;
    msg.append("loadsgf ");
    msg.append(fileName.toLatin1());
    msg.append(" ");
    msg.append(QString::number(moveNumber));
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::saveSgf(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    kDebug() << "Attempting to save game to" << fileName;

    QByteArray msg;
    msg.append("printsgf ");
    msg.append(fileName.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

QString GoEngine::name()
{
    QByteArray msg;
    msg.append("name\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

int GoEngine::protocolVersion()
{
    QByteArray msg;
    msg.append("protocol_version\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

QString GoEngine::version()
{
    QByteArray msg;
    msg.append("version\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

bool GoEngine::setBoardSize(int size)
{
    Q_ASSERT(size >= 1 && size <= 19);
    kDebug() << "Set board size to"  << size;

    QByteArray msg;
    msg.append("boardsize ");
    msg.append(QString::number(size));
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

int GoEngine::boardSize()
{
    QByteArray msg;
    msg.append("query_boardsize\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool GoEngine::clearBoard()
{
    kDebug() << "Clear board";

    QByteArray msg;
    msg.append("clear_board\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::setKomi(float komi)
{
    Q_ASSERT(komi >= 0);
    kDebug() << "Set komi to" << komi;

    QByteArray msg;
    msg.append("komi ");
    msg.append(QString::number(komi));
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool GoEngine::setLevel(int level)
{
    Q_ASSERT(level >= 1 && level <= 10);
    kDebug() << "Set difficulty level to" << level;

    QByteArray msg;
    msg.append("level ");
    msg.append(QString::number(level));
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool GoEngine::setFixedHandicap(int handicap)
{
    Q_ASSERT(handicap >= 0 && handicap <= 9);
    kDebug() << "Set fixed handicap to" << handicap;

    QByteArray msg;
    msg.append("fixed_handicap ");
    msg.append(QString::number(handicap));
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::playMove(PlayerColor color, const Stone &field)
{
    if (!field.isValid() || !color == WhitePlayer || color == BlackPlayer)
        return false;

    QByteArray msg;
    msg.append("play ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::passMove(PlayerColor color)
{
    if (!color == WhitePlayer || color == BlackPlayer)
        return false;

    QByteArray msg;
    msg.append("play ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append("pass\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::generateMove(PlayerColor color)
{
    if (!color == WhitePlayer || color == BlackPlayer)
        return false;

    QByteArray msg;
    msg.append("genmove ");
    if(color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::undoMove(int i)
{
    Q_ASSERT(i >= 0);
    QByteArray msg;
    msg.append("undo ");
    msg.append(QString::number(i));
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::tryMove(PlayerColor color, const Stone &field)
{
    if (!field.isValid() || !color == WhitePlayer || color == BlackPlayer)
        return false;

    QByteArray msg;
    msg.append("trymove ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

bool GoEngine::popGo()
{
    QByteArray msg;
    msg.append("popgo\n");
    m_process.write(msg);
    if (waitResponse()) {
        emit boardChanged();
        return true;
    } else
        return false;
}

QPair<GoEngine::PlayerColor, GoEngine::Stone> GoEngine::lastMove()
{
    QByteArray msg;
    msg.append("last_move\n");
    m_process.write(msg);
    QPair<PlayerColor, Stone> pair(InvalidPlayer, Stone());
    if (waitResponse()) {
        if (m_response.startsWith("white")) {
            pair.first = WhitePlayer;
            pair.second = Stone(m_response.split(" ")[1]);
        } else if (m_response.startsWith("black")) {
            pair.first = BlackPlayer;
            pair.second = Stone(m_response.split(" ")[1]);
        }
    }
    return pair;
}

GoEngine::FieldStatus GoEngine::whatColor(const Stone &field)
{
    if (!field.isValid())
        return InvalidField;

    QByteArray msg;
    msg.append("color ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
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
    QList<GoEngine::Stone> list;
    if (!color == WhitePlayer || color == BlackPlayer)
        return list;

    QByteArray msg;
    msg.append("list_stones ");
    if (color == WhitePlayer)
        msg.append("white");
    else
        msg.append("black");
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        foreach (QString pos, m_response.split(" "))
            list.append(Stone(pos));
    }
    return list;
}

int GoEngine::countLiberties(const Stone &field)
{
    if (!field.isValid())
        return -1;

    QByteArray msg;
    msg.append("countlib ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

QList<GoEngine::Stone> GoEngine::findLiberties(const Stone &field)
{
    QList<GoEngine::Stone> list;
    if (!field.isValid())
        return list;

    QByteArray msg;
    msg.append("findlib ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    waitResponse();

    foreach (QString entry, m_response.split(' '))
        list.append(GoEngine::Stone(entry));
    return list;
}

bool GoEngine::isLegal(PlayerColor color, const Stone &field)
{
    if (!field.isValid() || !color == WhitePlayer || color == BlackPlayer)
        return false;

    QByteArray msg;
    msg.append("is_legal ");
    if (color == WhitePlayer)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    waitResponse();
    if (m_response == "1")
        return true;
    else
        return false;
}

QString GoEngine::topMoves(PlayerColor color)
{
    if (!color == WhitePlayer || color == BlackPlayer)
        return QString();

    QByteArray msg;
    msg.append("top_moves_");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

QList<GoEngine::Stone> GoEngine::legalMoves(PlayerColor color)
{
    QList<GoEngine::Stone> list;
    if (!color == WhitePlayer || color == BlackPlayer)
        return list;

    QByteArray msg;
    msg.append("all_legal ");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    if (waitResponse()) {
        foreach (QString entry, m_response.split(' '))
            list.append(GoEngine::Stone(entry));
    }
    return list;
}

int GoEngine::captures(PlayerColor color)
{
    if (!color == WhitePlayer || color == BlackPlayer)
        return 0;

    QByteArray msg;
    msg.append("captures ");
    if (color == WhitePlayer)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return 0;
}

QString GoEngine::attack(const Stone &field)
{
    if (!field.isValid())
        return QString();

    QByteArray msg;
    msg.append("attack ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

QString GoEngine::defend(const Stone &field)
{
    if (!field.isValid())
        return QString();

    QByteArray msg;
    msg.append("defend ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

bool GoEngine::increaseDepths()
{
    QByteArray msg;
    msg.append("increase_depths\n");
    m_process.write(msg);
    return waitResponse();
}

bool GoEngine::decreaseDepths()
{
    QByteArray msg;
    msg.append("decrease_depths\n");
    m_process.write(msg);
    return waitResponse();
}

QString GoEngine::owlAttack(const Stone &field)
{
    if (!field.isValid())
        return QString();

    QByteArray msg;
    msg.append("owl_attack ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

QString GoEngine::owlDefense(const Stone &field)
{
    if (!field.isValid())
        return QString();

    QByteArray msg;
    msg.append("owl_defend ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

QString GoEngine::evalEye(const Stone &field)
{
    if (!field.isValid())
        return QString();

    QByteArray msg;
    msg.append("eval_eye ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

GoEngine::DragonStatus GoEngine::dragonStatus(const Stone &field)
{
    if (!field.isValid())
        return UnknownDragon;

    QByteArray msg;
    msg.append("dragon_status ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        if (m_response == "alive") return AliveDragon;
        else if (m_response == "critical") return CriticalDragon;
        else if (m_response == "dead") return DeadDragon;
        else if (m_response == "unknown") return UnknownDragon;
        else return UnknownDragon;    /* should never happen */
    } else
        return UnknownDragon;
}

bool GoEngine::sameDragon(const Stone &field1, const Stone &field2)
{
    QByteArray msg;
    msg.append("same_dragon ");
    msg.append(field1.toLatin1());
    msg.append(" ");
    msg.append(field2.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse() && m_response == "1")
        return true;
    else
        return false;
}

QString GoEngine::dragonData(const Stone &field)
{
    QByteArray msg;
    msg.append("dragon_data ");
    if (field.isValid())
        msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

GoEngine::FinalState GoEngine::finalStatus(const Stone &field)
{
    if (!field.isValid())
        return FinalStateInvalid;

    QByteArray msg;
    msg.append("final_status ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
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

    QByteArray msg;
    msg.append("final_status_list ");
    switch (state) {
        case FinalAlive: msg.append("alive"); break;
        case FinalDead: msg.append("dead"); break;
        case FinalSeki: msg.append("seki"); break;
        case FinalWhiteTerritory: msg.append("white_territory"); break;
        case FinalBlackTerritory: msg.append("black_territory"); break;
        case FinalDame: msg.append("dame"); break;
        case FinalStateInvalid: /* Will never happen */ break;
    }
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        foreach (QString entry, m_response.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

GoEngine::Score GoEngine::finalScore()
{
    QByteArray msg;
    msg.append("final_score\n");
    m_process.write(msg);
    if (waitResponse())
        return Score(m_response);
    else
        return Score();
}

GoEngine::Score GoEngine::estimateScore()
{
    QByteArray msg;
    msg.append("estimate_score\n");
    m_process.write(msg);
    if (waitResponse())
        return Score(m_response);
    else
        return Score();
}

int GoEngine::getLifeNodeCounter()
{
    QByteArray msg;
    msg.append("get_life_node_counter\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool GoEngine::resetLifeNodeCounter()
{
    QByteArray msg;
    msg.append("reset_life_node_counter\n");
    m_process.write(msg);
    return waitResponse();
}

int GoEngine::getOwlNodeCounter()
{
    QByteArray msg;
    msg.append("get_owl_node_counter\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool GoEngine::resetOwlNodeCounter()
{
    QByteArray msg;
    msg.append("reset_owl_node_counter\n");
    m_process.write(msg);
    return waitResponse();
}

int GoEngine::getReadingNodeCounter()
{
    QByteArray msg;
    msg.append("get_reading_node_counter\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool GoEngine::resetReadingNodeCounter()
{
    QByteArray msg;
    msg.append("reset_reading_node_counter\n");
    m_process.write(msg);
    return waitResponse();
}

int GoEngine::getTryMoveCounter()
{
    QByteArray msg;
    msg.append("get_trymove_counter\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool GoEngine::resetTryMoveCounter()
{
    QByteArray msg;
    msg.append("reset_trymove_counter\n");
    m_process.write(msg);
    return waitResponse();
}

bool GoEngine::showBoard()
{
    QByteArray msg;
    msg.append("showboard\n");
    m_process.write(msg);
    return waitResponse();
}

bool GoEngine::dumpStack()
{
    QByteArray msg;
    msg.append("dump_stack\n");
    m_process.write(msg);
    return waitResponse();
}

int GoEngine::wormCutStone(const Stone &field)
{
    if (!field.isValid())
        return -1;

    QByteArray msg;
    msg.append("worm_cutstone ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

QString GoEngine::wormData(const Stone &field)
{
    QByteArray msg;
    msg.append("worm_data ");
    if (field.isValid())
        msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

QList<GoEngine::Stone> GoEngine::wormStones(const Stone &field)
{
    QList<Stone> list;
    if (!field.isValid())
        return list;

    QByteArray msg;
    msg.append("worm_stones ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        foreach (QString entry, m_response.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

bool GoEngine::tuneMoveOrdering(int parameters)
{
    QByteArray msg;
    msg.append("tune_move_ordering ");
    msg.append(QString::number(parameters));
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

QList<QString> GoEngine::help()
{
    QByteArray msg;
    msg.append("help\n");
    m_process.write(msg);
    QList<QString> list;
    if (waitResponse()) {
        foreach (QString entry, m_response.split('\n'))
            list.append(entry);
    }
    return list;
}

bool GoEngine::reportUncertainty(bool enabled)
{
    QByteArray msg;
    msg.append("report_uncertainty ");
    msg.append(enabled ? "on" : "off");
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

QString GoEngine::shell(const QString &command)
{
    if (command.isEmpty())
        return QString();

    QByteArray msg;
    msg.append(command.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

bool GoEngine::knownCommand(const QString &command)
{
    if (command.isEmpty())
        return false;

    QByteArray msg;
    msg.append("known_command ");
    msg.append(command.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

QString GoEngine::echo(const QString &command)
{
    if (command.isEmpty())
        return QString();

    QByteArray msg;
    msg.append("echo ");
    msg.append(command.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

void GoEngine::readStandardOutput()
{
    m_response = m_process.readAllStandardOutput(); // Reponse arrived, fetch all stdin contents
}

void GoEngine::readStandardError()
{
    kWarning() << "Go engine I/O error occured:\n" << m_process.readAllStandardError();
}

bool GoEngine::waitResponse()
{
    if (!m_process.isOpen()) {                      // No GTP connection means no computing fun!
        kWarning() << "Go engine command failed because no GTP session is running!";
        return false;
    }

    QString tmp = m_response;
    do {                                            // Process some events till our response arrives
        qApp->processEvents();                      // to not slow down this thread too much
    } while (m_response.length() == 0 || m_response == tmp);

    tmp = m_response[0];
    m_response.remove(0, 2);                        // Remove the first two chars (e.g. "? " or "= "
    m_response.remove(m_response.size() - 2, 2);    // Remove the two trailing newlines

    kDebug() << "Response" << m_response;

    emit ready();
    if (tmp == "?")                                 // '?' Means the engine didn't understand the query
        return false;
    else
        return true;
}

} // End of namespace KGo

#include "moc_goengine.cpp"
