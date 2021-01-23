/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    explicit GeneralConfig(QWidget *parent = nullptr);

private Q_SLOTS:
    void updateEngineCommand();
};

} // End of namespace Kigo

#endif
