/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "score.h"

namespace Kigo {

Score::Score(const QString &score)
    : m_color(QLatin1Char('?')), m_score(0), m_lowerBound(0), m_upperBound(0)
{
    if (score.size() >= 2) {
        if (score[0] == QLatin1Char('W')) {
            m_color = QLatin1Char('W');
        } else if (score[0] == QLatin1Char('B')) {
            m_color = QLatin1Char('B');
        }
        int i = score.indexOf(QLatin1Char(' '));
        m_score = score.mid(2, i - 1).toFloat();
        QString newUpperBound = score.section(QLatin1Char(' '), 3, 3);
        newUpperBound.chop(1);
        m_upperBound = newUpperBound.toFloat();
        QString newLowerBound = score.section(QLatin1Char(' '), 5, 5);
        newLowerBound.chop(1);
        m_lowerBound = newLowerBound.toFloat();
    }
}

Score::Score(const Score &other)
    : QObject(), m_color(other.m_color), m_score(other.m_score)
    , m_lowerBound(other.m_lowerBound), m_upperBound(other.m_upperBound)
{
}

Score &Score::operator=(const Score &other)
{
    m_color = other.m_color;
    m_score = other.m_score;
    m_lowerBound = other.m_lowerBound;
    m_upperBound = other.m_upperBound;
    return *this;
}

bool Score::isValid() const
{
    return m_score >= 0 && (m_color == QLatin1Char('W') || m_color == QLatin1Char('B'));
}

QString Score::toString() const
{
    QString ret(m_color + '+');
    ret += QString::number(m_score) + " (" + QString::number(m_lowerBound) + " - " + QString::number(m_upperBound) + ')';
    return ret;
}

} // End of namespace Kigo

#include "moc_score.cpp"
