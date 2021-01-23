/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "move.h"

#include "kigo_debug.h"

namespace Kigo {

Move::Move(const Player *player, const Stone &stone)
    : m_player(player), m_stone(stone)
{
}

Move::Move(const Move &other)
    : m_player(other.m_player), m_stone(other.m_stone)
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
    debug.nospace() << "move of " << move.stone() << " by " << *move.player();
    return debug;
}

} // End of namespace Kigo
