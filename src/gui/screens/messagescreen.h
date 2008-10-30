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

#ifndef KGO_ERRORSCREEN_H
#define KGO_ERRORSCREEN_H

#include "ui_messagescreen.h"

#include <QWidget>

namespace KGo {

/**
 * This message screen is shown if the game was unable to start the
 * configured Go engine. A warning is shown and the user is asked
 * to (re)configure the Go engine in the configuration dialog.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.2
 */
class MessageScreen : public QWidget, private Ui::MessageScreen
{
    Q_OBJECT

public:
    /**
     *
     */
    explicit MessageScreen(QWidget *parent = 0);

public slots:
    void setMessageMessage(const QString &errorMessage);

signals:
    void configureButtonClicked();
    void quitButtonClicked();
};

} // End of namespace KGo

#endif
