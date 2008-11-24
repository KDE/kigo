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

#include "setupwidget.h"
#include "game/goengine.h"
#include "preferences.h"

#include <KDebug>

namespace Kigo {

SetupWidget::SetupWidget(GoEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
    , m_lastFixedHandicap(Preferences::fixedHandicapValue())
{
    setupUi(this);
}

SetupWidget::~SetupWidget()
{
    saveSettings();
}

void SetupWidget::newGame()
{
    loadSettings();
    gameSetupStack->setCurrentIndex(0);
    m_engine->setBoardSize(Preferences::boardSize());
    if (Preferences::fixedHandicapEnabled())
        m_engine->setFixedHandicap(Preferences::fixedHandicapValue());
    //TODO: Update info widget
}

void SetupWidget::loadedGame(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    loadSettings();
    gameSetupStack->setCurrentIndex(1);
    m_engine->loadGameFromSGF(fileName);
    // TODO: Update info widget
}

void SetupWidget::commit()
{
    saveSettings();

    m_engine->setPlayerStrength(GoEngine::WhitePlayer, Preferences::whitePlayerStrength());
    if (Preferences::whitePlayerHuman())
        m_engine->setPlayerType(GoEngine::WhitePlayer, GoEngine::HumanPlayer);
    else
        m_engine->setPlayerType(GoEngine::WhitePlayer, GoEngine::ComputerPlayer);

    m_engine->setPlayerStrength(GoEngine::BlackPlayer, Preferences::blackPlayerStrength());
    if (Preferences::blackPlayerHuman())
        m_engine->setPlayerType(GoEngine::BlackPlayer, GoEngine::HumanPlayer);
    else
        m_engine->setPlayerType(GoEngine::BlackPlayer, GoEngine::ComputerPlayer);

    // Set additional configuration based on game type
    if (gameSetupStack->currentIndex() == 0) {      // The user configured a new game
        m_engine->setKomi(Preferences::komi());
    } else {                                        // The user configured a loaded game
    }
}

void SetupWidget::on_startMoveSpinBox_valueChanged(int value)
{
    if (!m_lastFileName.isEmpty())
        m_engine->loadGameFromSGF(m_lastFileName, value);

    switch (m_engine->currentPlayer()) {
        case GoEngine::WhitePlayer: playerLabel->setText(i18n("White's move")); break;
        case GoEngine::BlackPlayer: playerLabel->setText(i18n("Black's move")); break;
        case GoEngine::InvalidPlayer: playerLabel->setText(""); break;
    }
}

void SetupWidget::on_sizeGroupBox_changed(int /*id*/)
{
    if (sizeOther->isChecked()) {                   // Custom size enabled
        sizeOtherSpinBox->setEnabled(true);
        m_engine->setBoardSize(sizeOtherSpinBox->value());
    } else {
        sizeOtherSpinBox->setEnabled(false);
        if (sizeSmall->isChecked())
            m_engine->setBoardSize(9);
        else if (sizeMedium->isChecked())
            m_engine->setBoardSize(13);
        else if (sizeBig->isChecked())
            m_engine->setBoardSize(19);
    }
    updateHandicapBox();                            // Handicap depends on board size
}

void SetupWidget::on_sizeOtherSpinBox_valueChanged(int value)
{
    m_engine->setBoardSize(value);              // Set free board size
    updateHandicapBox();
}

void SetupWidget::on_handicapGroupBox_toggled(bool isChecked)
{
    m_engine->clearBoard();                     // Also removes handicap
    if (isChecked)                                  // Set handicap if checked
        m_engine->setFixedHandicap(handicapSpinBox->value());
}

void SetupWidget::on_handicapSpinBox_valueChanged(int value)
{
    m_lastFixedHandicap = handicapSpinBox->value(); //
    m_engine->clearBoard();                     // Setting fixed handicap works only
    m_engine->setFixedHandicap(value);          // on a blank game board
}

void SetupWidget::updateHandicapBox()
{
    int maxFixedHandicap = m_engine->fixedHandicapMax();

    if (maxFixedHandicap == 0) {
        handicapGroupBox->setEnabled(false);
    } else {
        handicapGroupBox->setEnabled(true);

        if (maxFixedHandicap >= handicapSpinBox->minimum()) {
            handicapSpinBox->setMaximum(maxFixedHandicap);

            if (m_lastFixedHandicap >= maxFixedHandicap)
                handicapSpinBox->setValue(maxFixedHandicap);
            else
                handicapSpinBox->setValue(m_lastFixedHandicap);

            if (handicapGroupBox->isChecked())
                m_engine->setFixedHandicap(handicapSpinBox->value());
        } else
            handicapGroupBox->setChecked(false);
    }
}

void SetupWidget::loadSettings()
{
    whitePlayerName->setText(Preferences::whitePlayerName());
    whiteStrengthSlider->setValue(Preferences::whitePlayerStrength());
    if (Preferences::whitePlayerHuman())
        whitePlayerCombo->setCurrentIndex(whitePlayerCombo->findText(i18n("Human")));
    else
        whitePlayerCombo->setCurrentIndex(whitePlayerCombo->findText(i18n("Computer")));

    blackPlayerName->setText(Preferences::blackPlayerName());
    blackStrengthSlider->setValue(Preferences::blackPlayerStrength());
    if (Preferences::blackPlayerHuman())
        blackPlayerCombo->setCurrentIndex(blackPlayerCombo->findText(i18n("Human")));
    else
        blackPlayerCombo->setCurrentIndex(blackPlayerCombo->findText(i18n("Computer")));

    komiSpinBox->setValue(Preferences::komi());
    handicapGroupBox->setChecked(Preferences::fixedHandicapEnabled());
    handicapSpinBox->setValue(Preferences::fixedHandicapValue());

    switch (Preferences::boardSize()) {
        case 9:
            sizeSmall->setChecked(true);
            break;
        case 13:
            sizeMedium->setChecked(true);
            break;
        case 19:
            sizeBig->setChecked(true);
            break;
        default:
            sizeOther->setChecked(true);
            sizeOtherSpinBox->setEnabled(true);
            sizeOtherSpinBox->setValue(Preferences::boardSize());
            break;
    }
}

void SetupWidget::saveSettings()
{
    Preferences::setWhitePlayerName(whitePlayerName->text());
    Preferences::setWhitePlayerStrength(whiteStrengthSlider->value());
    Preferences::setWhitePlayerHuman(whitePlayerCombo->currentText() == i18n("Human"));

    Preferences::setBlackPlayerName(blackPlayerName->text());
    Preferences::setBlackPlayerStrength(blackStrengthSlider->value());
    Preferences::setBlackPlayerHuman(blackPlayerCombo->currentText() == i18n("Human"));

    Preferences::setKomi(komiSpinBox->value());
    Preferences::setFixedHandicapEnabled(handicapGroupBox->isChecked());
    Preferences::setFixedHandicapValue(handicapSpinBox->value());

    if (sizeSmall->isChecked())
        Preferences::setBoardSize(9);
    else if (sizeMedium->isChecked())
        Preferences::setBoardSize(13);
    else if (sizeBig->isChecked())
        Preferences::setBoardSize(19);
    else if (sizeOther->isChecked())
        Preferences::setBoardSize(sizeOtherSpinBox->value());

    Preferences::self()->writeConfig();
}

} // End of namespace Kigo

#include "moc_setupwidget.cpp"
