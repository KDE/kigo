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

namespace KGo {

SetupScreen::SetupScreen(GameScene *scene, QWidget *parent)
    : QWidget(parent)
    , m_gameEngine(scene->engine())
{
    if (!m_gameEngine->isRunning())
        kFatal() << "No Go engine is running!";             // Engine should really be running here!

    setupUi(this);
    GameView *gameView = new GameView(scene, this);
    gameView->setInteractive(false);                        // This is just a preview scene
    previewFrame->setLayout(new QHBoxLayout());
    previewFrame->layout()->addWidget(gameView);
    setupNewGame();                                         // Configure new game per default
}

SetupScreen::~SetupScreen()
{
    saveSettings();
}

void SetupScreen::setupNewGame()
{
    gameSetupStack->setCurrentIndex(0);
    loadSettings();

    m_gameEngine->setLevel(Preferences::difficulty());
    m_gameEngine->setKomi(Preferences::komi());
    m_gameEngine->setBoardSize(Preferences::boardSize());
    if (Preferences::fixedHandicapEnabled())
        m_gameEngine->setFixedHandicap(Preferences::fixedHandicapValue());
}

void SetupScreen::setupLoadedGame(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    gameSetupStack->setCurrentIndex(1);
    loadSettings();

    m_gameEngine->loadSgf(fileName);
    //TODO: Set max value of startMoveSpinBox
    //TODO: Display all related game information in the info box
}

void SetupScreen::on_whitePlayerCombo_currentIndexChanged(const QString &text)
{
    if (text == tr("Human")) {
        whitePlayerName->setEnabled(true);
        if (blackPlayerCombo->currentText() == tr("Human")) {
            difficultyBox->setEnabled(false);
            difficultySlider->setEnabled(false);
        } else {
            difficultyBox->setEnabled(true);
            difficultySlider->setEnabled(true);
        }
    } else if (text == tr("Computer")) {
        whitePlayerName->setEnabled(false);
        if (blackPlayerCombo->currentText() == tr("Computer")) {
            difficultyBox->setEnabled(false);
            difficultySlider->setEnabled(false);
        } else {
            difficultyBox->setEnabled(true);
            difficultySlider->setEnabled(true);
        }
    }
}

void SetupScreen::on_blackPlayerCombo_currentIndexChanged(const QString &text)
{
    if (text == tr("Human")) {
        blackPlayerName->setEnabled(true);
        if (whitePlayerCombo->currentText() == tr("Human")) {
            difficultyBox->setEnabled(false);
            difficultySlider->setEnabled(false);
        } else {
            difficultyBox->setEnabled(true);
            difficultySlider->setEnabled(true);
        }
    } else if (text == tr("Computer")) {
        blackPlayerName->setEnabled(false);
        if (whitePlayerCombo->currentText() == tr("Computer")) {
            difficultyBox->setEnabled(false);
            difficultySlider->setEnabled(false);
        } else {
            difficultyBox->setEnabled(true);
            difficultySlider->setEnabled(true);
        }
    }
}

void SetupScreen::on_startMoveSpinBox_valueChanged(int value)
{
    Q_UNUSED(value);
    //TODO: Show the corresponding board
}

void SetupScreen::on_difficultySlider_valueChanged(int value)
{
    m_gameEngine->setLevel(value);                  // Go engine difficulty level
}

void SetupScreen::on_sizeGroupBox_changed(int /*id*/)
{
    if (sizeOther->isChecked()) {
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
    updateHandicapBox();
}

void SetupScreen::on_sizeOtherSpinBox_valueChanged(int value)
{
    m_gameEngine->setBoardSize(value);              // Set free board size
    updateHandicapBox();
}

void SetupScreen::on_handicapGroupBox_toggled(bool isChecked)
{
    m_gameEngine->clearBoard();
    if (isChecked)
        m_gameEngine->setFixedHandicap(handicapSpinBox->value());
}

void SetupScreen::on_handicapSpinBox_valueChanged(int value)
{
    //TODO: Check and adjust max amount fixed handicap for smaller boards
    m_gameEngine->clearBoard();                     // Setting fixed handicap works only
    m_gameEngine->setFixedHandicap(value);          // on a blank game board
}

void SetupScreen::on_startButton_clicked()
{
    saveSettings();                                 // Save current game configuration
    emit startClicked();
}

void SetupScreen::updateHandicapBox()
{
    // Size changed, set fixed handicap reasonable display
    int fixedHandicap = m_gameEngine->fixedHandicap();
    if (fixedHandicap == 0)
        handicapGroupBox->setChecked(false);
    //TODO: Set min value
    handicapSpinBox->setValue(fixedHandicap);
}

void SetupScreen::loadSettings()
{
    kDebug() << "Load settings";
    if (Preferences::whitePlayerHuman()) {
        whitePlayerCombo->setCurrentIndex(whitePlayerCombo->findText(tr("Human")));
        whitePlayerName->setEnabled(true);
    } else {
        whitePlayerCombo->setCurrentIndex(whitePlayerCombo->findText(tr("Computer")));
        whitePlayerName->setEnabled(false);
    }
    whitePlayerName->setText(Preferences::whitePlayerName());

    if (Preferences::blackPlayerHuman()) {
        blackPlayerCombo->setCurrentIndex(blackPlayerCombo->findText(tr("Human")));
        blackPlayerName->setEnabled(true);
    } else {
        blackPlayerCombo->setCurrentIndex(blackPlayerCombo->findText(tr("Computer")));
        blackPlayerName->setEnabled(false);
    }
    blackPlayerName->setText(Preferences::blackPlayerName());

    if ((Preferences::blackPlayerHuman() && !Preferences::whitePlayerHuman())
        || (!Preferences::blackPlayerHuman() && Preferences::whitePlayerHuman())) {
        difficultyBox->setEnabled(true);
        difficultySlider->setEnabled(true);
    }

    difficultySlider->setValue(Preferences::difficulty());
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
    kDebug () << "Save settings";
    Preferences::setWhitePlayerHuman(whitePlayerCombo->currentText() == tr("Human"));
    Preferences::setWhitePlayerName(whitePlayerName->text());
    Preferences::setBlackPlayerHuman(blackPlayerCombo->currentText() == tr("Human"));
    Preferences::setBlackPlayerName(blackPlayerName->text());

    Preferences::setDifficulty(difficultySlider->value());
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
