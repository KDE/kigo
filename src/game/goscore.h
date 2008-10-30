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

#ifndef KGO_GOSCORE_H
#define KGO_GOSCORE_H

#include "goplayer.h"

#include <QString>

namespace KGo {

class GoEngine;

class GoScore
{
public:
    GoPlayer::Color leader() const { return m_leader; }
    float score() const { return m_score; }
    float lowerBound() const { return m_lowerBound; }
    float upperBound() const { return m_upperBound; }

    QString toString() const;

private:
    GoScore(const QString &scoreString);

    GoPlayer::Color m_leader;       ///< The player which has more points
    float m_score;                  ///< The amount of points the player leads with
    float m_lowerBound;             ///< Estimate lower bound
    float m_upperBound;             ///< Estimate upper bound

    friend class GoEngine;
};

} // End of namespace KGo

#endif
