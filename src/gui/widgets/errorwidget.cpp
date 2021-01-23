/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "errorwidget.h"
#include "preferences.h"

#include <QIcon>


namespace Kigo {

ErrorWidget::ErrorWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    configureButton->setIcon(QIcon::fromTheme( QStringLiteral( "configure" )));
}

void ErrorWidget::on_configureButton_clicked()
{
    Q_EMIT configureClicked();
}

} // End of namespace Kigo
