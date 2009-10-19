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

#include "stone.h"

#include <QDebug>

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
        m_y = stone.mid(1).toInt();
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
    return QString(m_x + QString::number(m_y));
}

QDebug operator<<(QDebug debug, const Stone &stone)
{
    debug.nospace() << "stone at " << stone.x() << "," << stone.y()
                    << " of value " << stone.value();
    return debug;
}

} // End of namespace Kigo
