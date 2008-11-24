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

#include "infowidget.h"
#include "game/goengine.h"

#include <QFile>
#include <QTextStream>

namespace Kigo {

InfoWidget::InfoWidget(GoEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    setupUi(this);
}

void InfoWidget::update()
{
    //TODO: This update method takes only info from the goengine
}

void InfoWidget::update(const QString & fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QRegExp re;

    // Parse additional game information from SGF file
    re.setPattern("EV\\[([\\w ]+)\\]");             // Capture and set event
    if (re.indexIn(content) > -1)
        infoEvent->setText(re.cap(1));
    re.setPattern("RO\\[(\\d+)\\]");                // Capture and set round
    if (re.indexIn(content) > -1)
        infoRound->setText(re.cap(1));
    re.setPattern("DT\\[([\\w/\\-:\\.]+)\\]");      // Capture and set date
    if (re.indexIn(content) > -1)
        infoDate->setText(re.cap(1));

    re.setPattern("PB\\[([\\w ]+)\\]");             // Capture and set black player name
    if (re.indexIn(content) > -1)
        infoBlack->setText(re.cap(1));
    re.setPattern("BR\\[([\\w ]+)\\]");             // Capture and set black player rank
    if (re.indexIn(content) > -1)
        infoBlack->setText(infoBlack->text() + " (" + re.cap(1) + ")");
    re.setPattern("PW\\[([\\w ]+)\\]");             // Capture and set white player name
    if (re.indexIn(content) > -1)
        infoWhite->setText(re.cap(1));
    re.setPattern("WR\\[([\\w ]+)\\]");             // Capture and set white player rank
    if (re.indexIn(content) > -1)
        infoWhite->setText(infoWhite->text() + " (" + re.cap(1) + ")");

    /*re.setPattern("KM\\[(\\d+\\.?\\d*)\\]");        // Capture and set komi
    if (re.indexIn(content) > -1)
        infoKomi->setValue(re.cap(1).toFloat());*/
    re.setPattern("RE\\[([WB]\\+[\\w\\.]+)\\]");    // Capture and set score
    if (re.indexIn(content) > -1)
        infoScore->setText(re.cap(1));
}

} // End of namespace Kigo

#include "moc_infowidget.cpp"
