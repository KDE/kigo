/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_ERRORWIDGET_H
#define KIGO_ERRORWIDGET_H

#include "ui_errorwidget.h"

#include <QWidget>

namespace Kigo {

/**
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.5
 */
class ErrorWidget : public QWidget, private Ui::ErrorWidget
{
    Q_OBJECT

public:
    explicit ErrorWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void configureClicked();

private Q_SLOTS:
    void on_configureButton_clicked();
};

} // End of namespace Kigo

#endif
