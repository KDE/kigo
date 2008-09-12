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
 * @file This file is part of KGO and implements the class SetupScreen.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#include "setupscreen.h"
#include "game/goengine.h"
#include "gui/graphicsview/gamescene.h"
#include "gui/graphicsview/gameview.h"
#include "preferences.h"

#include <KDebug>

#include <QFile>

namespace KGo {

SetupScreen::SetupScreen(GameScene *scene, QWidget *parent)
    : QWidget(parent)
    , m_gameEngine(scene->engine())
    , m_lastFixedHandicap(Preferences::fixedHandicapValue())
{
    if (!m_gameEngine->isRunning())
        kFatal() << "No Go engine is running!";         // Engine should be running here

    setupUi(this);
    GameView *gameView = new GameView(scene, this);
    gameView->setInteractive(false);                    // This is just a preview scene
    previewFrame->setLayout(new QHBoxLayout());
    previewFrame->layout()->addWidget(gameView);
    setupNewGame();                                     // Default configure new game
}

SetupScreen::~SetupScreen()
{
    saveSettings();
}

void SetupScreen::setupNewGame()
{
    loadSettings();
    gameSetupStack->setCurrentIndex(0);
    m_gameEngine->setBoardSize(Preferences::boardSize());
    if (Preferences::fixedHandicapEnabled())
        m_gameEngine->setFixedHandicap(Preferences::fixedHandicapValue());
}

void SetupScreen::setupLoadedGame(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    loadSettings();
    gameSetupStack->setCurrentIndex(1);
    m_gameEngine->loadSgf(fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    m_lastFileName = fileName;
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

    re.setPattern("KM\\[(\\d+\\.?\\d*)\\]");        // Capture and set komi
    if (re.indexIn(content) > -1)
        infoKomi->setValue(re.cap(1).toFloat());
    re.setPattern("RE\\[([WB]\\+[\\w\\.]+)\\]");    // Capture and set score
    if (re.indexIn(content) > -1)
        infoScore->setText(re.cap(1));

    // Parse move count
    re.setPattern("[BW]\\[\\w\\w\\]");
    int pos = 0;
    int count = 0;
    while (pos >= 0) {                              // Count all occurences of our pattern
        pos = re.indexIn(content, pos);
        if (pos >= 0) {
            pos += re.matchedLength();
            ++count;
        }
    }
    startMoveSpinBox->setSuffix(i18n(" of %1", count));
    startMoveSpinBox->setMaximum(count);            // And set it as maximum and current
    startMoveSpinBox->setValue(count);              // move.
}

void SetupScreen::on_startMoveSpinBox_valueChanged(int value)
{
    if (!m_lastFileName.isEmpty())
        m_gameEngine->loadSgf(m_lastFileName, value);

    switch (m_gameEngine->currentPlayer()) {
        case GoEngine::WhitePlayer: playerLabel->setText(i18n("White's move")); break;
        case GoEngine::BlackPlayer: playerLabel->setText(i18n("Black's move")); break;
        case GoEngine::InvalidPlayer: playerLabel->setText(""); break;
    }
}

void SetupScreen::on_sizeGroupBox_changed(int /*id*/)
{
    if (sizeOther->isChecked()) {                   // Custom size enabled
        sizeOtherSpinBox->setEnabled(true);
        m_gameEngine->setBoardSize(sizeOtherSpinBox->value());
    } else {
        sizeOtherSpinBox->setEnabled(false);
        if (sizeSmall->isChecked())
            m_gameEngine->setBoardSize(9);
        else if (sizeMedium->isChecked())
            m_gameEngine->setBoardSize(13);
        else if (sizeBig->isChecked())
            m_gameEngine->setBoardSize(19);
    }
    updateHandicapBox();                            // Handicap depends on board size
}

void SetupScreen::on_sizeOtherSpinBox_valueChanged(int value)
{
    m_gameEngine->setBoardSize(value);              // Set free board size
    updateHandicapBox();
}

void SetupScreen::on_handicapGroupBox_toggled(bool isChecked)
{
    m_gameEngine->clearBoard();                     // Also removes handicap
    if (isChecked)                                  // Set handicap if checked
        m_gameEngine->setFixedHandicap(handicapSpinBox->value());
}

void SetupScreen::on_handicapSpinBox_valueChanged(int value)
{
    m_lastFixedHandicap = handicapSpinBox->value(); //
    m_gameEngine->clearBoard();                     // Setting fixed handicap works only
    m_gameEngine->setFixedHandicap(value);          // on a blank game board
}

void SetupScreen::on_startButton_clicked()
{
    saveSettings();

    m_gameEngine->setPlayerStrength(GoEngine::WhitePlayer, Preferences::whitePlayerStrength());
    if (Preferences::whitePlayerHuman())
        m_gameEngine->setPlayerType(GoEngine::WhitePlayer, GoEngine::HumanPlayer);
    else
        m_gameEngine->setPlayerType(GoEngine::WhitePlayer, GoEngine::ComputerPlayer);

    m_gameEngine->setPlayerStrength(GoEngine::BlackPlayer, Preferences::blackPlayerStrength());
    if (Preferences::blackPlayerHuman())
        m_gameEngine->setPlayerType(GoEngine::BlackPlayer, GoEngine::HumanPlayer);
    else
        m_gameEngine->setPlayerType(GoEngine::BlackPlayer, GoEngine::ComputerPlayer);

    // Set additional configuration based on game type
    if (gameSetupStack->currentIndex() == 0) {      // The user configured a new game
        m_gameEngine->setKomi(Preferences::komi());
    } else {                                        // The user configured a loaded game
    }
    emit startClicked();
}

void SetupScreen::updateHandicapBox()
{
    int maxFixedHandicap = m_gameEngine->fixedHandicapMax();

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
                m_gameEngine->setFixedHandicap(handicapSpinBox->value());
        } else
            handicapGroupBox->setChecked(false);
    }
}

void SetupScreen::loadSettings()
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

void SetupScreen::saveSettings()
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

} // End of namespace KGo

#include "moc_setupscreen.cpp"
