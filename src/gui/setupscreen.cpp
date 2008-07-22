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
#include "preferences.h"
#include "game/gtp.h"
#include "gamescene.h"
#include "gameview.h"

#include <KDebug>

namespace KGo {

SetupScreen::SetupScreen(Gtp *gtp, QWidget *parent)
    : QWidget(parent)
    , m_gtp(gtp)
    , m_gameScene(new GameScene(m_gtp))
    , m_gameView(new GameView(m_gameScene, this))
{
    Q_ASSERT(gtp);

    setupUi(this);
    m_gameView->setInteractive(false);                    // This is just a preview, not a real game
    previewFrame->setLayout(new QHBoxLayout());
    previewFrame->layout()->addWidget(m_gameView);
    setupNewGame();                                        // Configure new game per default
}

SetupScreen::~SetupScreen()
{
    saveSettings();
}

void SetupScreen::setupNewGame()
{
    newGameBox->show();
    loadedGameBox->hide();
    infoBox->hide();
    loadSettings();
    m_gtp->openSession(Preferences::engineCommand());    // (Re)Connect to the configured go engine
    m_gtp->clearBoard();
}

void SetupScreen::setupLoadedGame(const QString &fileName, bool showInfo)
{
    Q_ASSERT(!fileName.isEmpty());

    newGameBox->hide();
    loadedGameBox->show();
    infoBox->setVisible(showInfo);
    loadSettings();
    m_gtp->openSession(Preferences::engineCommand());    // (Re)Connect to the configured go engine
    m_gtp->loadSgf(fileName);
    //TODO: Set max value of startMoveSpinBox
    if (showInfo) {
        //TODO: Display all related game information in the info box
    }
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
    //TODO: Show the corresponding board
}

void SetupScreen::on_sizeGroupBox_changed(int /*id*/)
{
    if (sizeOther->isChecked())
        sizeOtherSpinBox->setEnabled(true);
    else
        sizeOtherSpinBox->setEnabled(false);
}

void SetupScreen::on_startButton_clicked()
{
    saveSettings();
    if (newGameBox->isVisible()) {                        // Means we configured a new game
        m_gtp->setBoardSize(Preferences::boardSize());
        m_gtp->setLevel(Preferences::difficulty());
        m_gtp->setKomi(Preferences::komi());
        m_gtp->setFixedHandicap(Preferences::fixedHandicap());
    } else {                                            // Means we configured a loaded game
        //NOTE: Nothing to do here, all settings where already loaded from the SGF file.
    }
    emit startClicked();
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

    if ((Preferences::blackPlayerHuman() && !Preferences::whitePlayerHuman()) || (!Preferences::blackPlayerHuman() && Preferences::whitePlayerHuman())) {
        difficultyBox->setEnabled(true);
        difficultySlider->setEnabled(true);
    }

    difficultySlider->setValue(Preferences::difficulty());
    komiSpinBox->setValue(Preferences::komi());
    handicapSpinBox->setValue(Preferences::fixedHandicap());

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
    Preferences::setFixedHandicap(handicapSpinBox->value());

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
