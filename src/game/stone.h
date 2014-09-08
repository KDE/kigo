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

#ifndef KIGO_STONE_H
#define KIGO_STONE_H

#include <QString>

namespace Kigo {

/**
 * This class represents a stone on a field of the game board.
 */
class Stone
{
public:
    static Stone Pass;      ///< A standard pass move object
    static Stone Invalid;   ///< A standard invalid move object

    /**
     * Standard constructor. Can optionally set from the given coordinates.
     *
     * @param x The x coordinate of the stone
     * @param y The y coordinate of the stone
     * @param value The estimated quality of the move
     */
    explicit Stone(char x = 0, int y = 0, float value = 0);

    /**
     * Constructor to set from a stone given as a string.
     *
     * @param stone The stone as string
     * @param value The estimated quality of the move
     */
    explicit Stone(const QString &stone, float value = 0);

    Stone(const Stone &other);
    Stone &operator=(const Stone &other);

    bool isValid() const;
    QString toString() const;
    QByteArray toLatin1() const;

    char x() const { return m_x; }
    int y() const { return m_y; }
    float value() const { return m_value; }

private:
    char m_x;           ///< The horizontal field coordinate (from 'A' to 'T')
    int m_y;            ///< The vertical field coordinate (from 1 to 19)
    float m_value;      ///< The estimated quality
};

QDebug operator<<(QDebug debug, const Stone &stone);

} // End of namespace Kigo

#endif
