/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    explicit Score(const QString &score = QString());

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
