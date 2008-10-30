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

#include "goplayer.h"

namespace KGo {

GoPlayer::GoPlayer(Color color, Type type, const QString &name, int strength)
    : m_color(color), m_type(type), m_name(name), m_strength(strength)
{
    Q_ASSERT(strength >= 1 && strength <= 10);

    if (m_name.isEmpty())
        m_color == White ? m_name = "White Player" : m_name = "Black Player";
}

void GoPlayer::setName(const QString &name)
{
    m_name = name;
    if (m_name.isEmpty())
        m_color == White ? m_name = "White Player" : m_name = "Black Player";
}

void GoPlayer::setStrength(int strength)
{
    Q_ASSERT(strength >= 1 && strength <= 10);

    m_strength = strength;
}

} // End of namespace KGo

#include "moc_goplayer.cpp"
