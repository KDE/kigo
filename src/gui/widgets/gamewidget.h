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

#ifndef KIGO_GAMEWIDGET_H
#define KIGO_GAMEWIDGET_H

#include "ui_gamewidget.h"

#include <QWidget>

namespace Kigo {

class GoEngine;

/**
 * @see GoEngine
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.5
 */
class GameWidget : public QWidget, private Ui::GameWidget
{
    Q_OBJECT

public:
    /**
     *
     */
    explicit GameWidget(GoEngine *engine, QWidget *parent = 0);

public slots:
    void init();
    void update();

private:
    GoEngine *m_engine;
};

} // End of namespace Kigo

#endif
