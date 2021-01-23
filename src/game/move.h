/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_MOVE_H
#define KIGO_MOVE_H

#include "player.h"
#include "stone.h"

namespace Kigo {

/**
 * The Move class is a light-weight representation of a Go
 * move (to be) made by a Go player.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.5
 */
class Move
{
public:
    Move(const Player *player, const Stone &stone);
    Move(const Move &other);
    Move &operator=(const Move &other);

    const Player *player() const { return m_player; }
    const Stone &stone() const { return m_stone; }

    bool isValid() const { return m_stone.isValid(); }
    bool isPass() const { return !m_stone.isValid(); }

private:
    const Player *m_player;
    Stone m_stone;
};

QDebug operator<<(QDebug debug, const Move &move);

} // End of namespace Kigo

#endif
