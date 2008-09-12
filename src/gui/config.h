/*******************************************************************
 *
 * Copyright 2008 Sascha Peilicke <sasch.pe@gmx.de>
 *
 * This file is part of the KDE project "KGo"
 *
 * KGo is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KGo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KReversi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *******************************************************************/

/**
 * @file This file is part of KGO and defines the class GeneralConfig,
 *       which displays the general configuration options tab in the
 *       Kgo configuration screen.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_CONFIG_H
#define KGO_CONFIG_H

#include "ui_config.h"

#include <QWidget>

namespace KGo {

/**
 * Represents the general configuration tab in the KGo
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

} // End of namespace KGo

#endif
