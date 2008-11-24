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

#include <QFile>

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
}

void SetupWidget::loadedGame(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    loadSettings();
    gameSetupStack->setCurrentIndex(1);
    m_engine->loadGameFromSGF(fileName);
    m_lastFileName = fileName;

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
        blackPlayerName->setText(re.cap(1));
    re.setPattern("BR\\[([\\w ]+)\\]");             // Capture and set black player rank
    if (re.indexIn(content) > -1)
        blackPlayerName->setText(blackPlayerName->text() + " (" + re.cap(1) + ")");
    re.setPattern("PW\\[([\\w ]+)\\]");             // Capture and set white player name
    if (re.indexIn(content) > -1)
        whitePlayerName->setText(re.cap(1));
    re.setPattern("WR\\[([\\w ]+)\\]");             // Capture and set white player rank
    if (re.indexIn(content) > -1)
        whitePlayerName->setText(whitePlayerName->text() + " (" + re.cap(1) + ")");

    /*re.setPattern("KM\\[(\\d+\\.?\\d*)\\]");        // Capture and set komi
    if (re.indexIn(content) > -1)
        infoKomi->setValue(re.cap(1).toFloat());*/
    re.setPattern("RE\\[([WB]\\+[\\w\\.]+)\\]");    // Capture and set score
    if (re.indexIn(content) > -1)
        infoScore->setText(re.cap(1));

    re.setPattern("[BW]\\[\\w\\w\\]");              // Parse move count
    int pos = 0, count = 0;
    while (pos >= 0) {                              // Count all occurences of our pattern
        pos = re.indexIn(content, pos);
        if (pos >= 0) {
            pos += re.matchedLength();
            count++;
        }
    }
    startMoveSpinBox->setSuffix(i18n(" of %1", count));
    startMoveSpinBox->setMaximum(count);            // And set it as maximum and current
    startMoveSpinBox->setValue(count);              // move.
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
        //Note: None currently
    }
}

void SetupWidget::on_startMoveSpinBox_valueChanged(int value)
{
    if (!m_lastFileName.isEmpty())
        m_engine->loadGameFromSGF(m_lastFileName, value);

    switch (m_engine->currentPlayer()) {
        case GoEngine::WhitePlayer: playerLabel->setText(i18n("for White")); break;
        case GoEngine::BlackPlayer: playerLabel->setText(i18n("for Black")); break;
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
    if (gameSetupStack->currentIndex() == 0) {      // The user configured a new game
        Preferences::setWhitePlayerName(whitePlayerName->text());
        Preferences::setBlackPlayerName(blackPlayerName->text());
    } else {
        //Note: Don't save player names for a loaded game because the names set by
        //      the user are overriden by those found in the SGF file.
    }

    Preferences::setWhitePlayerStrength(whiteStrengthSlider->value());
    Preferences::setBlackPlayerStrength(blackStrengthSlider->value());
    Preferences::setWhitePlayerHuman(whitePlayerCombo->currentText() == i18n("Human"));
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
