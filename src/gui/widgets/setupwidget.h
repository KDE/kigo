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

#ifndef KIGO_SETUPWIDGET_H
#define KIGO_SETUPWIDGET_H

#include "ui_setupwidget.h"

#include <QWidget>

namespace Kigo {

class GoEngine;

/**
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.5
 */
class SetupWidget : public QWidget, private Ui::SetupWidget
{
    Q_OBJECT

public:
    SetupWidget(GoEngine *engine, QWidget *parent = 0);
    ~SetupWidget();

public slots:
    /**
     *
     */
    void newGame();

    /**
     * @param fileName The SGF file to load the game from
     */
    void loadedGame(const QString &fileName);

    /**
     * To actually commit the configured values to the Go backend,
     * this method has to be called.
     */
    void commit();

private slots:
    void on_startMoveSpinBox_valueChanged(int);
    void on_sizeGroupBox_changed(int);
    void on_sizeOtherSpinBox_valueChanged(int);
    void on_handicapGroupBox_toggled(bool);
    void on_handicapSpinBox_valueChanged(int);

private:
    void updateHandicapBox();
    void loadSettings();
    void saveSettings();

    GoEngine *m_engine;
    QString m_lastFileName;
    int m_lastFixedHandicap;
};

} // End of namespace Kigo

#endif
