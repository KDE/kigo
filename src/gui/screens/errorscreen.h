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
 * @file This file is part of KGO and defines the class ErrorScreen,
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_ERRORSCREEN_H
#define KGO_ERRORSCREEN_H

#include "ui_errorscreen.h"

#include <QWidget>

namespace KGo {

/**
 * This error screen is shown if the game was unable to start the
 * configured Go engine. A warning is shown and the user is asked
 * to (re)configure the Go engine in the configuration dialog.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.2
 */
class ErrorScreen : public QWidget, private Ui::ErrorScreen
{
    Q_OBJECT

public:
    /**
     *
     */
    explicit ErrorScreen(QWidget *parent = 0);

signals:
    void configureClicked();
    void quitClicked();
};

} // End of namespace KGo

#endif
