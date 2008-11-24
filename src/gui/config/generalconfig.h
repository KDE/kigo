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

#ifndef KIGO_CONFIG_H
#define KIGO_CONFIG_H

#include "ui_generalconfig.h"

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
    explicit GeneralConfig(QWidget *parent = 0);

private slots:
    void updateEngineCommand();
};

} // End of namespace Kigo

#endif
