/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

bool operator==(const Stone &stone, const Stone &other);

} // End of namespace Kigo

#endif
