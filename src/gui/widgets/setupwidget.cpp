/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "setupwidget.h"
#include "game/game.h"
#include "preferences.h"
#include "gui/graphicsview/themerenderer.h"

#include <QFile>
#include <QIcon>
#include <QRegularExpression>

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
    const QPixmap whiteStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::WhiteStone, QSize(48, 48));
    whiteStoneImageLabel->setPixmap(whiteStone);
    const QPixmap blackStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::BlackStone, QSize(48, 48));
    blackStoneImageLabel->setPixmap(blackStone);

    connect(startButton, &QPushButton::clicked, this, &SetupWidget::startClicked);
    connect(whiteIsComputerCheckBox, &QCheckBox::toggled, this, &SetupWidget::whiteIsComputer);
    connect(blackIsComputerCheckBox, &QCheckBox::toggled, this, &SetupWidget::blackIsComputer);
    connect(sizeSmall, &QRadioButton::clicked, this, &SetupWidget::onBoardSizeChanged);
    connect(sizeMedium, &QRadioButton::clicked, this, &SetupWidget::onBoardSizeChanged);
    connect(sizeBig, &QRadioButton::clicked, this, &SetupWidget::onBoardSizeChanged);
    connect(sizeOther, &QRadioButton::clicked, this, &SetupWidget::onBoardSizeChanged);
    connect(sizeOtherSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SetupWidget::onBoardSizeChanged);
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
    const QString content = in.readAll();
    file.close();

    QRegularExpression re;
    QRegularExpressionMatch match;

    // Parse additional game information from SGF file
    re.setPattern(QStringLiteral("EV\\[([\\w ]+)\\]"));             // Capture and set event

    if ((match = re.match(content)).hasMatch()) {
        eventLabel->setText(match.captured(1));
        eventLabel->setVisible(true);
        eventStaticLabel->setVisible(true);
    } else {
        eventLabel->setVisible(false);
        eventStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("PC\\[([\\w ,]+)\\]"));             // location
    if ((match = re.match(content)).hasMatch()) {
        locationLabel->setText(match.captured(1));
        locationLabel->setVisible(true);
        locationStaticLabel->setVisible(true);
    } else {
        locationLabel->setVisible(false);
        locationStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("RO\\[(\\d+)\\]"));                // Capture and set round
    if ((match = re.match(content)).hasMatch()) {
        roundLabel->setText(match.captured(1));
        roundLabel->setVisible(true);
        roundStaticLabel->setVisible(true);
    } else {
        roundLabel->setVisible(false);
        roundStaticLabel->setVisible(false);
    }
    re.setPattern(QStringLiteral("DT\\[([\\w/\\-:\\.,]+)\\]"));      // Capture and set date
    if ((match = re.match(content)).hasMatch()) {
        dateLabel->setText(match.captured(1));
        dateLabel->setVisible(true);
        dateStaticLabel->setVisible(true);
    } else {
        dateLabel->setVisible(false);
        dateStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("PB\\[([\\w ]+)\\]"));             // Capture and set black player name
    if ((match = re.match(content)).hasMatch()) {
        blackPlayerName->setText(match.captured(1));
    }
    re.setPattern(QStringLiteral("BR\\[([\\w ]+)\\]"));             // Capture and set black player rank
    if ((match = re.match(content)).hasMatch()) {
        blackPlayerName->setText(blackPlayerName->text() + " (" + match.captured(1) + ')');
    }
    re.setPattern(QStringLiteral("BT\\[([\\w ]+)\\]"));             // black team
    if ((match = re.match(content)).hasMatch()) {
        blackPlayerName->setText(blackPlayerName->text() + " [" + match.captured(1) + ']');
    }

    re.setPattern(QStringLiteral("PW\\[([\\w ]+)\\]"));             // Capture and set white player name
    if ((match = re.match(content)).hasMatch()) {
        whitePlayerName->setText(match.captured(1));
    }
    re.setPattern(QStringLiteral("WR\\[([\\w ]+)\\]"));             // Capture and set white player rank
    if ((match = re.match(content)).hasMatch()) {
        whitePlayerName->setText(whitePlayerName->text() + " (" + match.captured(1) + ')');
    }
    re.setPattern(QStringLiteral("WT\\[([\\w ]+)\\]"));             // white team
    if ((match = re.match(content)).hasMatch()) {
        whitePlayerName->setText(whitePlayerName->text() + " [" + match.captured(1) + ']');
    }

    re.setPattern(QStringLiteral("KM\\[(\\d+\\.?\\d*)\\]"));        // Capture and set komi
    if ((match = re.match(content)).hasMatch()) {
        komiLabel->setText(match.captured(1));
        komiLabel->setVisible(true);
        komiStaticLabel->setVisible(true);
    } else {
        komiLabel->setVisible(false);
        komiStaticLabel->setVisible(false);
    }

    re.setPattern(QStringLiteral("TM\\[(\\d+)\\]"));        // time limit in seconds
    if ((match = re.match(content)).hasMatch()) {
        const int seconds = match.captured(1).toInt();
        const int hours = seconds/3600;
        const int minutes = (seconds/60)%60;
        const QString minuteString = i18ncp("Time limit of a game in minutes", "%1 minute", "%1 minutes", minutes);
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
    if ((match = re.match(content)).hasMatch()) {
        scoreLabel->setText(match.captured(1));
        scoreLabel->setVisible(true);
        scoreStaticLabel->setVisible(true);
    } else {
        scoreLabel->setVisible(false);
        scoreStaticLabel->setVisible(false);
    }

    const int count = m_game->moveCount();
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
        m_game->whitePlayer().setType(Player::Type::Human);
    } else {
        m_game->whitePlayer().setType(Player::Type::Computer);
    }
    m_game->whitePlayer().setStrength(Preferences::whitePlayerStrength());
    m_game->whitePlayer().setName(Preferences::whitePlayerName());

    if (Preferences::blackPlayerHuman()) {
        m_game->blackPlayer().setType(Player::Type::Human);
    } else {
        m_game->blackPlayer().setType(Player::Type::Computer);
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

void SetupWidget::onBoardSizeChanged()
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
    const int maxFixedHandicap = m_game->fixedHandicapUpperBound();
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

#include "moc_setupwidget.cpp"
