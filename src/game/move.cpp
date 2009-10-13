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

#include "move.h"

#include <KDebug>

namespace Kigo {

Move::Move(const Player *player, const Stone &stone)
    : m_player(player), m_stone(stone)
{
}

Move::Move(const Move &other)
    : QObject(), m_player(other.m_player), m_stone(other.m_stone)
{
}

Move &Move::operator=(const Move &other)
{
    m_player = other.m_player;
    m_stone = other.m_stone;
    return *this;
}

QDebug operator<<(QDebug debug, const Move &move)
{
    debug.nospace() << "(Move " << move.stone() << " by " << *move.player() << ")";
    return debug;
}

} // End of namespace Kigo

#include "moc_move.cpp"
