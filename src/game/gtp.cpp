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
 * @file This file is part of KGO and implements the classes Gtp, Gtp::Stone
 *       and Gtp::Score, which together implement a Go Text Protocol (GTP)
 *       interface to communicate with Go engines supporting GTP protocol
 *       version 2.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#include "gtp.h"

#include <QFile>
#include <QApplication>

#include <KDebug>

namespace KGo {

Gtp::Stone::Stone(const QString &stone)
    : m_x(stone[0].toLatin1())
    , m_y(stone.mid(1).toInt())
{
}

bool Gtp::Stone::isValid() const
{
    if (m_y >= 1 && m_y <= 19 && ((m_x >= 'a' && m_x < 't') || (m_x >= 'A' && m_x <= 'T')))
        return true;
    else
        return false;
}

QByteArray Gtp::Stone::toLatin1() const
{
    QByteArray msg;
    msg.append(m_x);
    msg.append(m_y);
    return msg;
}

QString Gtp::Stone::toString() const
{
    return QString("%1%2").arg(m_x).arg(m_y);
}

Gtp::Score::Score(const QString &scoreString)
{
    if (scoreString[0] == 'W')
        m_player = PlayerWhite;
    else
        m_player = PlayerBlack;
    int i = scoreString.indexOf(' ');
    m_score = scoreString.mid(2, i - 1).toInt();
    //m_upperBound = scoreString.mid(
    //TODO: Implement Score class, the bounds should be considered optional and can
    // be the same as the score if none given
}

bool Gtp::Score::isValid() const
{
    return m_score >= 0 ? true : false;
}

QString Gtp::Score::toString() const
{
    return QString("%1+%2 (upper bound: %3, lower: %4").arg(m_player == PlayerWhite ? "W" : "B").arg(m_score).arg(m_upperBound).arg(m_lowerBound);
}

////////////////////////////////////////////////////////////////////

Gtp::Gtp()
{
    connect(&m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
    connect(&m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStandardError()));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), SIGNAL(error(QProcess::ProcessError)));
}

Gtp::~Gtp()
{
    quit();
}

bool Gtp::openSession(const QString &command)
{
    quit();                                         // Close old session if there's one
    m_process.start(command.toLatin1());            // Start new process with provided command

    if (!m_process.waitForStarted())                // NOTE: Blocking wait for process start
        return false;

    if (protocolVersion()) {
        clearBoard();                               // Start with blank board
    } else {
        quit();
        m_response = "Protocol version error";
        return false;
    }
    return true;
}

void Gtp::quit()
{
    if (m_process.isOpen()) {
        m_process.write("quit\n");
        m_process.close();
    }
}

bool Gtp::loadSgf(const QString &fileName, int moveNumber)
{
    Q_ASSERT(moveNumber >= 0);
    if (fileName.isEmpty() || !QFile::exists(fileName))
        return false;

    QByteArray msg;
    msg.append("loadsgf ");
    msg.append(fileName.toLatin1());
    msg.append(" ");
    msg.append(moveNumber);
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::saveSgf(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    QByteArray msg;
    msg.append("printsgf ");
    msg.append(fileName.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

QString Gtp::name()
{
    QByteArray msg;
    msg.append("name\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

int Gtp::protocolVersion()
{
    QByteArray msg;
    msg.append("protocol_version\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

QString Gtp::version()
{
    QByteArray msg;
    msg.append("version\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

bool Gtp::setBoardSize(int size)
{
    Q_ASSERT(size >= 1 && size <= 19);

    QByteArray msg;
    msg.append("boardsize ");
    msg.append(size);
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

int Gtp::boardSize()
{
    QByteArray msg;
    msg.append("query_boardsize\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool Gtp::clearBoard()
{
    QByteArray msg;
    msg.append("clear_board\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::setKomi(float komi)
{
    Q_ASSERT(komi >= 0);

    QByteArray msg;
    msg.append("komi ");
    msg.append(komi);
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::setLevel(int level)
{
    Q_ASSERT(level >= 1 && level <= 10);

    QByteArray msg;
    msg.append("level ");
    msg.append(level);
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::setFixedHandicap(int handicap)
{
    Q_ASSERT(handicap >= 0 && handicap <= 9);

    QByteArray msg;
    msg.append("fixed_handicap ");
    msg.append(handicap);
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::playMove(PlayerColor color, const Stone &field)
{
    if (!field.isValid())
        return false;

    QByteArray msg;
    msg.append("play ");
    if (color == PlayerWhite)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::passMove(PlayerColor color)
{
    QByteArray msg;
    msg.append("play ");
    if (color == PlayerWhite)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append("pass\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::generateMove(PlayerColor color)
{
    QByteArray msg;
    msg.append("genmove ");
    if(color == PlayerWhite)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::undoMove(int i)
{
    Q_ASSERT(i >= 0);
    QByteArray msg;
    msg.append("undo ");
    msg.append(i);
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::tryMove(PlayerColor color, const Stone &field)
{
    if (!field.isValid())
        return false;

    QByteArray msg;
    msg.append("trymove ");
    if (color == PlayerWhite)
        msg.append("white ");
    else
        msg.append("black ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::popGo()
{
    QByteArray msg;
    msg.append("popgo\n");
    m_process.write(msg);
    return waitResponse();
}

Gtp::FieldStatus Gtp::whatColor(const Stone &field)
{
    if (!field.isValid())
        return FieldInvalid;

    QByteArray msg;
    msg.append("color ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        if (m_response == "white")            return FieldWhite;
        else if (m_response == "black")        return FieldBlack;
        else if (m_response == "empty")        return FieldEmpty;
        else return FieldInvalid;
    } else
        return FieldInvalid;
}

int Gtp::countLiberties(const Stone &field)
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

QList<Gtp::Stone> Gtp::findLiberties(const Stone &field)
{
    QList<Gtp::Stone> list;
    if (!field.isValid())
        return list;

    QByteArray msg;
    msg.append("findlib ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    waitResponse();

    foreach (QString entry, m_response.split(' '))
        list.append(Gtp::Stone(entry));
    return list;
}

bool Gtp::isLegal(PlayerColor color, const Stone &field)
{
    if (!field.isValid())
        return false;

    QByteArray msg;
    msg.append("is_legal ");
    if (color == PlayerWhite)
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

QString Gtp::topMoves(PlayerColor color)
{
    QByteArray msg;
    msg.append("top_moves_");
    if (color == PlayerWhite)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response;
    else
        return QString();
}

QList<Gtp::Stone> Gtp::legalMoves(PlayerColor color)
{
    QByteArray msg;
    msg.append("all_legal ");
    if (color == PlayerWhite)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    waitResponse();

    QList<Gtp::Stone> list;
    foreach (QString entry, m_response.split(' '))
        list.append(Gtp::Stone(entry));
    return list;
}

int Gtp::captures(PlayerColor color)
{
    QByteArray msg;
    msg.append("captures ");
    if (color == PlayerWhite)
        msg.append("white\n");
    else
        msg.append("black\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return 0;
}

QString Gtp::attack(const Stone &field)
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

QString Gtp::defend(const Stone &field)
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

bool Gtp::increaseDepths()
{
    QByteArray msg;
    msg.append("increase_depths\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::decreaseDepths()
{
    QByteArray msg;
    msg.append("decrease_depths\n");
    m_process.write(msg);
    return waitResponse();
}

QString Gtp::owlAttack(const Stone &field)
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

QString Gtp::owlDefense(const Stone &field)
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

QString Gtp::evalEye(const Stone &field)
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

Gtp::DragonStatus Gtp::dragonStatus(const Stone &field)
{
    if (!field.isValid())
        return DragonInvalid;

    QByteArray msg;
    msg.append("dragon_status ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        if (m_response == "alive")             return DragonAlive;
        else if (m_response == "critical")    return DragonCritical;
        else if (m_response == "dead")        return DragonDead;
        else if (m_response == "unknown")    return DragonUnknown;
        else return DragonInvalid;    /* should never happen */
    } else
        return DragonInvalid;
}

bool Gtp::sameDragon(const Stone &field1, const Stone &field2)
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

QString Gtp::dragonData(const Stone &field)
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

Gtp::FinalState Gtp::finalStatus(const Stone &field)
{
    if (!field.isValid())
        return StateInvalid;

    QByteArray msg;
    msg.append("final_status ");
    msg.append(field.toLatin1());
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        if (m_response == "alive")                    return Alive;
        else if (m_response == "dead")                return Dead;
        else if (m_response == "seki")                return Seki;
        else if (m_response == "white_territory")    return WhiteTerritory;
        else if (m_response == "blacK_territory")    return BlackTerritory;
        else if (m_response == "dame")                return Dame;
        else return StateInvalid;
    } else
        return StateInvalid;
}

QList<Gtp::Stone> Gtp::finalStatusList(FinalState state)
{
    QList<Stone> list;
    if (state == StateInvalid)
        return list;

    QByteArray msg;
    msg.append("final_status_list ");
    switch (state) {
        case Alive: msg.append("alive"); break;
        case Dead: msg.append("dead"); break;
        case Seki: msg.append("seki"); break;
        case WhiteTerritory: msg.append("white_territory"); break;
        case BlackTerritory: msg.append("black_territory"); break;
        case Dame: msg.append("dame"); break;
        case StateInvalid: /* Will never happen */ break;
    }
    msg.append("\n");
    m_process.write(msg);
    if (waitResponse()) {
        foreach (QString entry, m_response.split(' '))
            list.append(Stone(entry));
    }
    return list;
}

Gtp::Score Gtp::finalScore()
{
    QByteArray msg;
    msg.append("final_score\n");
    m_process.write(msg);
    if (waitResponse())
        return Score(m_response);
    else
        return Score();
}

Gtp::Score Gtp::estimateScore()
{
    QByteArray msg;
    msg.append("estimate_score\n");
    m_process.write(msg);
    if (waitResponse())
        return Score(m_response);
    else
        return Score();
}

int Gtp::getLifeNodeCounter()
{
    QByteArray msg;
    msg.append("get_life_node_counter\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool Gtp::resetLifeNodeCounter()
{
    QByteArray msg;
    msg.append("reset_life_node_counter\n");
    m_process.write(msg);
    return waitResponse();
}

int Gtp::getOwlNodeCounter()
{
    QByteArray msg;
    msg.append("get_owl_node_counter\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool Gtp::resetOwlNodeCounter()
{
    QByteArray msg;
    msg.append("reset_owl_node_counter\n");
    m_process.write(msg);
    return waitResponse();
}

int Gtp::getReadingNodeCounter()
{
    QByteArray msg;
    msg.append("get_reading_node_counter\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool Gtp::resetReadingNodeCounter()
{
    QByteArray msg;
    msg.append("reset_reading_node_counter\n");
    m_process.write(msg);
    return waitResponse();
}

int Gtp::getTryMoveCounter()
{
    QByteArray msg;
    msg.append("get_trymove_counter\n");
    m_process.write(msg);
    if (waitResponse())
        return m_response.toInt();
    else
        return -1;
}

bool Gtp::resetTryMoveCounter()
{
    QByteArray msg;
    msg.append("reset_trymove_counter\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::showBoard()
{
    QByteArray msg;
    msg.append("showboard\n");
    m_process.write(msg);
    return waitResponse();
}

bool Gtp::dumpStack()
{
    QByteArray msg;
    msg.append("dump_stack\n");
    m_process.write(msg);
    return waitResponse();
}

int Gtp::wormCutStone(const Stone &field)
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

QString Gtp::wormData(const Stone &field)
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

QList<Gtp::Stone> Gtp::wormStones(const Stone &field)
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

bool Gtp::tuneMoveOrdering(int parameters)
{
    QByteArray msg;
    msg.append("tune_move_ordering ");
    msg.append(parameters);
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

QList<QString> Gtp::help()
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

bool Gtp::reportUncertainty(bool enabled)
{
    QByteArray msg;
    msg.append("report_uncertainty ");
    msg.append(enabled ? "on" : "off");
    msg.append("\n");
    m_process.write(msg);
    return waitResponse();
}

QString Gtp::shell(const QString &command)
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

bool Gtp::knownCommand(const QString &command)
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

QString Gtp::echo(const QString &command)
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

void Gtp::readStandardOutput()
{
    m_response = m_process.readAllStandardOutput();
}

void Gtp::readStandardError()
{
    kDebug() << "Error occured:" << m_process.readAllStandardError();
}

bool Gtp::waitResponse()
{
    QString tmp = m_response;
    do {                                            // Process some events till our response arrives
        qApp->processEvents();                      // to not slow down this thread too much
    } while (m_response.length() == 0 || m_response == tmp);

    tmp = m_response[0];
    m_response.remove(0, 2);                        // Remove the first two chars (e.g. "? " or "= "
    m_response.remove(m_response.size() - 2, 2);    // Remove the two trailing newlines

    emit ready();
    if (tmp == "?")                                 // '?' Means the engine didn't understand the query
        return false;
    else
        return true;
}

} // End of namespace KGo

#include "moc_gtp.cpp"
