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

#ifndef KIGO_PLAYER_H
#define KIGO_PLAYER_H

#include <QDebug>
#include <QObject>
#include <QString>

namespace Kigo {

/**
 * The Player class holds all basic attributes of a Go player. These mean
 * mostly name, skill and color. Instances are particular to a specific
 * game and can thus only be created by the Go engine (Kigo::Game).
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.5
 */
class Player : public QObject
{
    Q_OBJECT

    friend class Game;

public:
    enum Color {
        White = 1,          ///< The white player
        Black,              ///< The black player
        Invalid
    };

    enum Type {
        Human = 1,          ///< A human player
        Computer            ///< A computer player
    };

private:
    Player(Color color, Type type = Human);
    Player(const Player &other);

public:
    Player &operator=(const Player &other);

    void setName(const QString &name) { m_name = name; }
    QString name() const { return m_name; }

    bool setStrength(int strength);
    int strength() const { return m_strength; }

    void setColor(Color color) { m_color = color; }
    Color color() const { return m_color; }

    void setType(Type type) { m_type = type; }
    Type type() const { return m_type; }

    bool isWhite() const { return m_color == White; }
    bool isBlack() const { return m_color == Black; }
    bool isValid() const { return m_color != Invalid; }
    bool isHuman() const { return m_type == Human; }
    bool isComputer() const { return m_type == Computer; }

    bool operator==(const Player &other);

private:
    QString m_name;
    Color m_color;
    Type m_type;
    int m_strength;
};

QDebug operator<<(QDebug debug, const Player &player);

} // End of namespace Kigo

#endif
