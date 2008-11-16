/***************************************************************************
 *   Copyright (C) 2008 by Sascha Peilicke <sasch.pe@gmx.de>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KGO_GOPLAYER_H
#define KGO_GOPLAYER_H

#include <QString>
#include <QObject>

namespace Kigo {

class GoEngine;

/**
 *
 */
class GoPlayer : public QObject
{
    Q_OBJECT

public:
    enum Color {
        White = 1,          ///< The white player
        Black               ///< The black player
    };

    enum Type {
        Human = 1,          ///<
        Computer            ///<
    };

    Color color() const { return m_color; }

    void setName(const QString &name);
    QString name() const { return m_name; }

    void setType(Type type) { m_type = type; }
    Type type() const { return m_type; }

    void setStrength(int strength);
    int strength() const { return m_strength; }

private:
    GoPlayer(Color color, Type type, const QString &name = QString(), int strength = 10);

    const Color m_color;
    Type m_type;
    QString m_name;
    int m_strength;

    friend class GoEngine;
};

} // End of namespace Kigo

#endif
