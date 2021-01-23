/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_PLAYER_H
#define KIGO_PLAYER_H

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
class Player
{
    friend class Game;

public:
    enum class Color {
        White = 1,          ///< The white player
        Black,              ///< The black player
        Invalid
    };

    enum class Type {
        Human = 1,          ///< A human player
        Computer            ///< A computer player
    };

private:
    explicit Player(Color color, Type type = Type::Human);

public:
    Player(const Player &other);
    Player &operator=(const Player &other);

    void setName(const QString &name) { m_name = name; }
    QString name() const { return m_name; }

    bool setStrength(int strength);
    int strength() const { return m_strength; }

    void setColor(Color color) { m_color = color; }
    Color color() const { return m_color; }

    void setType(Type type) { m_type = type; }
    Type type() const { return m_type; }

    bool isWhite() const { return m_color == Color::White; }
    bool isBlack() const { return m_color == Color::Black; }
    bool isValid() const { return m_color != Color::Invalid; }
    bool isHuman() const { return m_type == Type::Human; }
    bool isComputer() const { return m_type == Type::Computer; }

    bool operator==(const Player &other) const;

private:
    QString m_name;
    Color m_color;
    Type m_type;
    int m_strength;
};

QDebug operator<<(QDebug debug, const Player &player);

} // End of namespace Kigo

#endif
