/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "player.h"

#include "kigo_debug.h"

namespace Kigo {

Player::Player(Color color, Type type)
    : m_color(color), m_type(type), m_strength(10)
{
}

Player::Player(const Player &other)
    : m_name(other.m_name), m_color(other.m_color)
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

bool Player::operator==(const Player &other) const
{
    return m_name == other.m_name &&
           m_color == other.m_color &&
           m_type == other.m_type &&
           m_strength == other.m_strength;
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

QDebug operator<<(QDebug debug, const Player &player)
{
    if (player.isWhite()) {
        debug.nospace() << "white";
    } else if (player.isBlack()) {
        debug.nospace() << "black";
    } else {
        debug.nospace() << "invalid";
    }
    debug.nospace() << " player " << player.name();
    return debug;
}

} // End of namespace Kigo
