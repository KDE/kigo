/*
    Copyright 2008 Sascha Peilicke <sasch.pe@gmx.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "setupwidget.h"
#include "game/game.h"
#include "preferences.h"

#include <QFile>

namespace Kigo {

SetupWidget::SetupWidget(Game *game, QWidget *parent)
    : QWidget(parent), m_game(game)
    , m_lastFixedHandicap(Preferences::fixedHandicapValue())
{
    Q_ASSERT(m_game);
    setupUi(this);

    startButton->setIcon(KIcon("media-playback-start"));

    connect(startButton, SIGNAL(clicked()), this, SIGNAL(startClicked()));
}

SetupWidget::~SetupWidget()
{
    saveSettings();
}

void SetupWidget::newGame()
{
    loadSettings();
    gameSetupStack->setCurrentIndex(0);
    m_game->setBoardSize(Preferences::boardSize());
    if (Preferences::fixedHandicapEnabled()) {
        m_game->setFixedHandicap(Preferences::fixedHandicapValue());
    }
}

void SetupWidget::loadedGame(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    loadSettings();
    gameSetupStack->setCurrentIndex(1);
    m_game->init(fileName);
    m_lastFileName = fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QRegExp re;

    // Parse additional game information from SGF file
    re.setPattern("EV\\[([\\w ]+)\\]");             // Capture and set event
    if (re.indexIn(content) > -1) {
        infoEvent->setText(re.cap(1));
    }
    re.setPattern("RO\\[(\\d+)\\]");                // Capture and set round
    if (re.indexIn(content) > -1) {
        infoRound->setText(re.cap(1));
    }
    re.setPattern("DT\\[([\\w/\\-:\\.]+)\\]");      // Capture and set date
    if (re.indexIn(content) > -1) {
        infoDate->setText(re.cap(1));
    }

    re.setPattern("PB\\[([\\w ]+)\\]");             // Capture and set black player name
    if (re.indexIn(content) > -1) {
        blackPlayerName->setText(re.cap(1));
    }
    re.setPattern("BR\\[([\\w ]+)\\]");             // Capture and set black player rank
    if (re.indexIn(content) > -1) {
        blackPlayerName->setText(blackPlayerName->text() + " (" + re.cap(1) + ')');
    }
    re.setPattern("PW\\[([\\w ]+)\\]");             // Capture and set white player name
    if (re.indexIn(content) > -1) {
        whitePlayerName->setText(re.cap(1));
    }
    re.setPattern("WR\\[([\\w ]+)\\]");             // Capture and set white player rank
    if (re.indexIn(content) > -1) {
        whitePlayerName->setText(whitePlayerName->text() + " (" + re.cap(1) + ')');
    }

    re.setPattern("KM\\[(\\d+\\.?\\d*)\\]");        // Capture and set komi
    if (re.indexIn(content) > -1) {
        infoKomi->setText(re.cap(1));
    }
    re.setPattern("RE\\[([WB]\\+[\\w\\.]+)\\]");    // Capture and set score
    if (re.indexIn(content) > -1) {
        infoScore->setText(re.cap(1));
    }

    re.setPattern("[BW]\\[\\w\\w\\]");              // Parse move count
    int pos = 0, count = 0;
    while (pos >= 0) {                              // Count all occurrences of our pattern
        pos = re.indexIn(content, pos);
        if (pos >= 0) {
            pos += re.matchedLength();
            count++;
        }
    }
    startMoveSpinBox->setSuffix(i18n(" of %1", count));
    startMoveSpinBox->setMaximum(count);            // And set it as maximum and current
    startMoveSpinBox->setValue(count);              // move.
    startMoveSpinBox->setFocus(Qt::OtherFocusReason);
}

void SetupWidget::commit()
{
    saveSettings();

    if (Preferences::whitePlayerHuman()) {
        m_game->whitePlayer().setType(Player::Human);
    } else {
        m_game->whitePlayer().setType(Player::Computer);
    }
    m_game->whitePlayer().setStrength(Preferences::whitePlayerStrength());
    m_game->whitePlayer().setName(Preferences::whitePlayerName());

    if (Preferences::blackPlayerHuman()) {
        m_game->blackPlayer().setType(Player::Human);
    } else {
        m_game->blackPlayer().setType(Player::Computer);
    }
    m_game->blackPlayer().setStrength(Preferences::blackPlayerStrength());
    m_game->blackPlayer().setName(Preferences::blackPlayerName());

    // Set additional configuration based on game type
    if (gameSetupStack->currentIndex() == 0) {      // The user configured a new game
        m_game->setKomi(Preferences::komi());
    } else {                                        // The user configured a loaded game
        //Note: None currently
    }
}

void SetupWidget::on_startMoveSpinBox_valueChanged(int value)
{
    if (!m_lastFileName.isEmpty()) {
        m_game->init(m_lastFileName, value);
    }

    if (m_game->currentPlayer().isWhite()) {
        playerLabel->setText(i18n("for White"));
    } else if (m_game->currentPlayer().isBlack()) {
        playerLabel->setText(i18n("for Black"));
    } else {
        playerLabel->setText("");
    }
}

void SetupWidget::on_sizeGroupBox_changed(int /*id*/)
{
    if (sizeOther->isChecked()) {                   // Custom size enabled
        sizeOtherSpinBox->setEnabled(true);
        m_game->setBoardSize(sizeOtherSpinBox->value());
    } else {
        sizeOtherSpinBox->setEnabled(false);
        if (sizeSmall->isChecked()) {
            m_game->setBoardSize(9);
        } else if (sizeMedium->isChecked()) {
            m_game->setBoardSize(13);
        } else if (sizeBig->isChecked()) {
            m_game->setBoardSize(19);
        }
    }
    updateHandicapBox();                            // Handicap depends on board size
}

void SetupWidget::on_sizeOtherSpinBox_valueChanged(int value)
{
    m_game->setBoardSize(value);                  // Set free board size
    updateHandicapBox();
}

void SetupWidget::on_handicapGroupBox_toggled(bool isChecked)
{
    m_game->init();                               // Also removes handicap
    if (isChecked) {                                // Set handicap if checked
        m_game->setFixedHandicap(handicapSpinBox->value());
    }
}

void SetupWidget::on_handicapSpinBox_valueChanged(int value)
{
    m_lastFixedHandicap = handicapSpinBox->value(); //
    m_game->init();                               // Setting fixed handicap works only
    m_game->setFixedHandicap(value);              // on a blank game board
}

void SetupWidget::updateHandicapBox()
{
    int maxFixedHandicap = m_game->fixedHandicapUpperBound();

    if (maxFixedHandicap == 0) {
        handicapGroupBox->setEnabled(false);
    } else {
        handicapGroupBox->setEnabled(true);

        if (maxFixedHandicap >= handicapSpinBox->minimum()) {
            handicapSpinBox->setMaximum(maxFixedHandicap);

            if (m_lastFixedHandicap >= maxFixedHandicap) {
                handicapSpinBox->setValue(maxFixedHandicap);
            } else {
                handicapSpinBox->setValue(m_lastFixedHandicap);
            }

            if (handicapGroupBox->isChecked()) {
                m_game->setFixedHandicap(handicapSpinBox->value());
            }
        } else {
            handicapGroupBox->setChecked(false);
        }
    }
}

void SetupWidget::loadSettings()
{
    whitePlayerName->setText(Preferences::whitePlayerName());
    whiteStrengthSlider->setValue(Preferences::whitePlayerStrength());
    if (Preferences::whitePlayerHuman()) {
        whitePlayerCombo->setCurrentIndex(whitePlayerCombo->findText(i18n("Human")));
    } else {
        whitePlayerCombo->setCurrentIndex(whitePlayerCombo->findText(i18n("Computer")));
    }

    blackPlayerName->setText(Preferences::blackPlayerName());
    blackStrengthSlider->setValue(Preferences::blackPlayerStrength());
    if (Preferences::blackPlayerHuman()) {
        blackPlayerCombo->setCurrentIndex(blackPlayerCombo->findText(i18n("Human")));
    } else {
        blackPlayerCombo->setCurrentIndex(blackPlayerCombo->findText(i18n("Computer")));
    }

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
        //      the user are overridden by those found in the SGF file.
    }

    Preferences::setWhitePlayerStrength(whiteStrengthSlider->value());
    Preferences::setBlackPlayerStrength(blackStrengthSlider->value());
    Preferences::setWhitePlayerHuman(whitePlayerCombo->currentText() == i18n("Human"));
    Preferences::setBlackPlayerHuman(blackPlayerCombo->currentText() == i18n("Human"));

    Preferences::setKomi(komiSpinBox->value());
    Preferences::setFixedHandicapEnabled(handicapGroupBox->isChecked());
    Preferences::setFixedHandicapValue(handicapSpinBox->value());

    if (sizeSmall->isChecked()) {
        Preferences::setBoardSize(9);
    } else if (sizeMedium->isChecked()) {
        Preferences::setBoardSize(13);
    } else if (sizeBig->isChecked()) {
        Preferences::setBoardSize(19);
    } else if (sizeOther->isChecked()) {
        Preferences::setBoardSize(sizeOtherSpinBox->value());
    }
    Preferences::self()->writeConfig();
}

} // End of namespace Kigo

#include "moc_setupwidget.cpp"
