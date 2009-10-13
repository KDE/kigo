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

#ifndef KIGO_SCORE_H
#define KIGO_SCORE_H

#include <QObject>
#include <QString>

namespace Kigo {

/**
 * This class represents a Go score for either player. A score can only
 * be created by a Kigo::Game instance. The score will always hold the
 * points of the leading player (together with certain probability bounds)
 * and can thus be only created by the Go engine (Kigo::Game).
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.5
 */
class Score : public QObject
{
    Q_OBJECT

    friend class Game;

private:
    Score(const QString &score = QString());

public:
    Score(const Score &other);
    Score &operator=(const Score &other);

    bool isValid() const;
    QString toString() const;

    QChar color() const { return m_color; }
    float score() const { return m_score; }
    float lowerBound() const { return m_lowerBound; }
    float upperBound() const { return m_upperBound; }

private:
    QChar m_color;          ///< The color of the player which has more points
    float m_score;          ///< The amount of points the player leads with
    float m_lowerBound;     ///< Estimate lower bound
    float m_upperBound;     ///< Estimate upper bound
};

} // End of namespace Kigo

#endif
