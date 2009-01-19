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
class Move : public QObject
{
    Q_OBJECT

public:
    Move(const Player &player, const Stone &stone);
    Move(const Move &other);
    Move &operator=(const Move &other);

    const Player &player() const { return m_player; }
    const Stone &stone() const { return m_stone; }

    bool isValid() const { return m_stone.isValid(); }
    bool isPass() const { return !m_stone.isValid(); }

private:
    Player &m_player;
    Stone &m_stone;
};

} // End of namespace Kigo

#endif
