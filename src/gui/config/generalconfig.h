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

#ifndef KIGO_GENERALCONFIG_H
#define KIGO_GENERALCONFIG_H

#include <ui_generalconfig.h>

#include <QWidget>

namespace Kigo {

/**
 * Represents the general configuration tab in the Kigo
 * configuration screen.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class GeneralConfig : public QWidget, private Ui::GeneralConfig
{
    Q_OBJECT

public:
    /**
     * Standard Constructor. Sets up the loaded user interface.
     */
    GeneralConfig(QWidget *parent = 0);

private slots:
    void updateEngineCommand();
};

} // End of namespace Kigo

#endif
