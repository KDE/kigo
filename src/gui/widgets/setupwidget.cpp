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
#include "gui/graphicsview/themerenderer.h"

#include <QFile>
#include <QIcon>
namespace Kigo {

SetupWidget::SetupWidget(Game *game, QWidget *parent)
    : QWidget(parent)
    , m_game(game)
    , m_lastFileName()
    , m_lastFixedHandicap(Preferences::fixedHandicapValue())
{
    Q_ASSERT(m_game);
    setupUi(this);

    startButton->setIcon(QIcon::fromTheme( QStringLiteral( "media-playback-start" )));
    QPixmap whiteStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::WhiteStone, QSize(48, 48));
    whiteStoneImageLabel->setPixmap(whiteStone);
    QPixmap blackStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::BlackStone, QSize(48, 48));
    blackStoneImageLabel->setPixmap(blackStone);

    connect(startButton, &QPushButton::clicked, this, &SetupWidget::startClicked);
    connect(whiteIsComputerCheckBox, &QCheckBox::toggled, this, &SetupWidget::whiteIsComputer);
    connect(blackIsComputerCheckBox, &QCheckBox::toggled, this, &SetupWidget::blackIsComputer);
    connect(sizeSmall, &QRadioButton::clicked, this, &SetupWidget::on_boardSize_changed);
    connect(sizeMedium, &QRadioButton::clicked, this, &SetupWidget::on_boardSize_changed);
    connect(sizeBig, &QRadioButton::clicked, this, &SetupWidget::on_boardSize_changed);
    connect(sizeOther, &QRadioButton::clicked, this, &SetupWidget::on_boardSize_changed);
    connect(sizeOtherSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SetupWidget::on_boardSize_changed);
}

SetupWidget::~SetupWidget()
{
    saveSettings();
}

void SetupWidget::newGame()
{
    loadSettings();
    m_game->init();
    gameSetupStack->setCurrentIndex(0);
    m_game->setBoardSize(Preferences::boardSize());
    handicapSpinBox->setSuffix(ki18np(" Stone", " Stones"));
    m_game->setFixedHandicap(Preferences::fixedHandicapValue());
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
    re.setPattern(QStringLiteral("EV\\[([\\w ]+)\\]"));             // Capture and set event
    if (re.indexIn(content) > -1) {
        eventLabel->setText(re.cap(1));
        eventLabel->setVisible(true);
        eventStaticLabel->setVisible(true);
    } else {
        eventLabel->setVisible(false);
        eventStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("PC\\[([\\w ,]+)\\]"));             // location
    if (re.indexIn(content) > -1) {
        locationLabel->setText(re.cap(1));
        locationLabel->setVisible(true);
        locationStaticLabel->setVisible(true);
    } else {
        locationLabel->setVisible(false);
        locationStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("RO\\[(\\d+)\\]"));                // Capture and set round
    if (re.indexIn(content) > -1) {
        roundLabel->setText(re.cap(1));
        roundLabel->setVisible(true);
        roundStaticLabel->setVisible(true);
    } else {
        roundLabel->setVisible(false);
        roundStaticLabel->setVisible(false);
    }
    re.setPattern(QStringLiteral("DT\\[([\\w/\\-:\\.,]+)\\]"));      // Capture and set date
    if (re.indexIn(content) > -1) {
        dateLabel->setText(re.cap(1));
        dateLabel->setVisible(true);
        dateStaticLabel->setVisible(true);
    } else {
        dateLabel->setVisible(false);
        dateStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("PB\\[([\\w ]+)\\]"));             // Capture and set black player name
    if (re.indexIn(content) > -1) {
        blackPlayerName->setText(re.cap(1));
    }
    re.setPattern(QStringLiteral("BR\\[([\\w ]+)\\]"));             // Capture and set black player rank
    if (re.indexIn(content) > -1) {
        blackPlayerName->setText(blackPlayerName->text() + " (" + re.cap(1) + ')');
    }
    re.setPattern(QStringLiteral("BT\\[([\\w ]+)\\]"));             // black team
    if (re.indexIn(content) > -1) {
        blackPlayerName->setText(blackPlayerName->text() + " [" + re.cap(1) + ']');
    }

    re.setPattern(QStringLiteral("PW\\[([\\w ]+)\\]"));             // Capture and set white player name
    if (re.indexIn(content) > -1) {
        whitePlayerName->setText(re.cap(1));
    }
    re.setPattern(QStringLiteral("WR\\[([\\w ]+)\\]"));             // Capture and set white player rank
    if (re.indexIn(content) > -1) {
        whitePlayerName->setText(whitePlayerName->text() + " (" + re.cap(1) + ')');
    }
    re.setPattern(QStringLiteral("WT\\[([\\w ]+)\\]"));             // white team
    if (re.indexIn(content) > -1) {
        whitePlayerName->setText(whitePlayerName->text() + " [" + re.cap(1) + ']');
    }

    re.setPattern(QStringLiteral("KM\\[(\\d+\\.?\\d*)\\]"));        // Capture and set komi
    if (re.indexIn(content) > -1) {
        komiLabel->setText(re.cap(1));
        komiLabel->setVisible(true);
        komiStaticLabel->setVisible(true);
    } else {
        komiLabel->setVisible(false);
        komiStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("TM\\[(\\d+)\\]"));        // time limit in seconds
    if (re.indexIn(content) > -1) {
        int seconds = re.cap(1).toInt();
        int hours = seconds/3600;
        int minutes = (seconds/60)%60;
        QString minuteString = i18ncp("Time limit of a game in minutes", "%1 minute", "%1 minutes", minutes);
        if (hours) {
            timeLabel->setText(i18ncp("Time limit of a game, hours, minutes", "%1 hour, %2", "%1 hours, %2", hours, minuteString));
        } else {
            timeLabel->setText(minuteString);
        }
        timeLabel->setVisible(true);
        timeStaticLabel->setVisible(true);
    } else {
       timeLabel->setVisible(false);
       timeStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("RE\\[([WB]\\+[\\w\\.]+)\\]"));    // Capture and set score
    if (re.indexIn(content) > -1) {
        scoreLabel->setText(re.cap(1));
        scoreLabel->setVisible(true);
        scoreStaticLabel->setVisible(true);
    } else {
        scoreLabel->setVisible(false);
        scoreStaticLabel->setVisible(false);
    }

    int count = m_game->moveCount();
    startMoveSpinBox->setSuffix(i18n(" of %1", count));
    startMoveSpinBox->setMaximum(count);            // And set it as maximum and current
    startMoveSpinBox->setValue(count);              // move.
    startMoveSpinBox->setFocus(Qt::OtherFocusReason);
    connect(startMoveSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SetupWidget::on_startMoveSpinBox_valueChanged);
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
    m_game->gameSetup();
}

void SetupWidget::on_startMoveSpinBox_valueChanged(int value)
{
    if (!m_lastFileName.isEmpty()) {
        m_game->init(m_lastFileName, value);
    }

    if (m_game->currentPlayer().isWhite()) {
        playerLabel->setText(i18n("White to play"));
    } else if (m_game->currentPlayer().isBlack()) {
        playerLabel->setText(i18n("Black to play"));
    } else {
        playerLabel->setText(QLatin1String(""));
    }
}

void SetupWidget::on_boardSize_changed()
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

void SetupWidget::on_handicapSpinBox_valueChanged(int value)
{
    m_lastFixedHandicap = value; // Treat 1 handicap as 0
    m_game->init();                                   // Setting fixed handicap works only
    m_game->setFixedHandicap(value == 1 ? 0 : value); // on a blank game board
}

void SetupWidget::updateHandicapBox()
{
    int maxFixedHandicap = m_game->fixedHandicapUpperBound();
    handicapSpinBox->setEnabled(maxFixedHandicap > 0);
    handicapSpinBox->setMaximum(maxFixedHandicap);
    handicapSpinBox->setValue(m_lastFixedHandicap);
    on_handicapSpinBox_valueChanged(handicapSpinBox->value());
}

void SetupWidget::loadSettings()
{
    whitePlayerName->setText(Preferences::whitePlayerName());
    whiteStrengthSlider->setValue(Preferences::whitePlayerStrength());
    whiteIsComputerCheckBox->setChecked(!Preferences::whitePlayerHuman());

    blackPlayerName->setText(Preferences::blackPlayerName());
    blackStrengthSlider->setValue(Preferences::blackPlayerStrength());
    blackIsComputerCheckBox->setChecked(!Preferences::blackPlayerHuman());

    komiSpinBox->setValue(Preferences::komi());
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
    Preferences::setWhitePlayerHuman(!whiteIsComputerCheckBox->isChecked());
    Preferences::setBlackPlayerHuman(!blackIsComputerCheckBox->isChecked());

    Preferences::setKomi(komiSpinBox->value());
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
    Preferences::self()->save();
}

void SetupWidget::blackIsComputer(bool computer)
{
    blackPlayerStack->setCurrentIndex(computer ? 1 : 0);
}

void SetupWidget::whiteIsComputer(bool computer)
{
    whitePlayerStack->setCurrentIndex(computer ? 1 : 0);
}

} // End of namespace Kigo


