/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "stone.h"

#include "kigo_debug.h"

namespace Kigo {

Stone Stone::Pass = Stone();
Stone Stone::Invalid = Stone();

Stone::Stone(char x, int y, float value)
    : m_x(x), m_y(y), m_value(value)
{
}

Stone::Stone(const QString &stone, float value)
    : m_x(0), m_y(0), m_value(value)
{
    if (stone.size() >= 2) {
        m_x = stone[0].toUpper().toLatin1();
        m_y = QStringView(stone).mid(1).toInt();
    }
}

Stone::Stone(const Stone &other)
    : m_x(other.m_x), m_y(other.m_y), m_value(other.m_value)
{
}

Stone &Stone::operator=(const Stone &other)
{
    m_x = other.m_x;
    m_y = other.m_y;
    m_value = other.m_value;
    return *this;
}

bool Stone::isValid() const
{
    // Go coordinates are somewhat complicated ...
    return m_y >= 1 && m_y <= 19 && m_x >= 'A' && m_x != 'I' && m_x <= 'T';
}

QByteArray Stone::toLatin1() const
{
    return QByteArray(m_x + QByteArray::number(m_y));
}

QString Stone::toString() const
{
    return QString(QLatin1Char(m_x) + QString::number(m_y));
}

QDebug operator<<(QDebug debug, const Stone &stone)
{
    debug.nospace() << "stone at " << stone.x() << "," << stone.y()
                    << " of value " << stone.value();
    return debug;
}

bool operator==(const Stone &stone, const Stone &other)
{
    return (stone.x() == other.x()) && (stone.y() == other.y());
}

} // End of namespace Kigo
