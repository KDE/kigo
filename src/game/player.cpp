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

#include "player.h"

namespace Kigo {

Player::Player(Color color, Type type, int strength)
    : m_color(color), m_type(type), m_strength(strength)
{
}

Player::Player(const Player &other)
    : QObject(), m_name(other.m_name), m_color(other.m_color)
    , m_type(other.m_type), m_strength(other.m_strength)
{
}

Player &Player::operator=(const Player &other)
{
    m_name = other.m_name;
    m_color = other.m_color;
    m_type = other.m_type;
    m_strength = other.m_strength;
    return *this;
}

bool Player::operator==(const Player &other)
{
    return m_name == other.m_name &&
           m_color == other.m_color &&
           m_type == other.m_type &&
           m_strength == other.m_type;
}

bool Player::setStrength(int strength)
{
    if (strength > 0 && strength < 11) {
        m_strength = strength;
        return true;
    } else {
        return false;
    }
}

QString Player::toString() const
{
    return m_name + " [" + (m_color == White ? "White" : "Black") +
           ':' + QByteArray::number(m_strength) + ']';
}

} // End of namespace Kigo

#include "moc_player.cpp"
