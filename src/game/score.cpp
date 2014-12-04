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

#include "score.h"

namespace Kigo {

Score::Score(const QString &score)
    : m_color('?'), m_score(0), m_lowerBound(0), m_upperBound(0)
{
    if (score.size() >= 2) {
        if (score[0] == 'W') {
            m_color = 'W';
        } else if (score[0] == 'B') {
            m_color = 'B';
        }
        int i = score.indexOf(' ');
        m_score = score.mid(2, i - 1).toFloat();
        QString upperBound = score.section(' ', 3, 3);
        upperBound.chop(1);
        m_upperBound = upperBound.toFloat();
        QString lowerBound = score.section(' ', 5, 5);
        lowerBound.chop(1);
        m_lowerBound = lowerBound.toFloat();
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
    return m_score >= 0 && (m_color == 'W' || m_color == 'B');
}

QString Score::toString() const
{
    QString ret(m_color + '+');
    ret += QString::number(m_score) + " (" + QString::number(m_lowerBound) + " - " + QString::number(m_upperBound) + ')';
    return ret;
}

} // End of namespace Kigo


