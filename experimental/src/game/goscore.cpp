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

#include "goscore.h"

namespace Kigo {

GoScore::GoScore(const QString &scoreString)
    : m_score(0), m_lowerBound(0), m_upperBound(0)
{
    if (scoreString.size() >= 2) {
        if (scoreString[0] == 'W')
            m_leader = GoPlayer::White;
        else if (scoreString[0] == 'B')
            m_leader = GoPlayer::Black;
        /*else
            m_leader = GoPlayer::Unknown;*/

        //TODO: Check this Unkown stuff, better to get rid of it

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

QString GoScore::toString() const
{
    QString ret;
    if (m_leader == GoPlayer::White)
        ret += "W+";
    else if (m_leader == GoPlayer::Black)
        ret += "B+";
    else
        ret += "?+";
    ret += QString::number(m_score) + " (" + QString::number(m_lowerBound) + " - " + QString::number(m_upperBound) + ')';
    return ret;
}

} // End of namespace Kigo
