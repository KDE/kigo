/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gamewidget.h"
#include "game/game.h"
#include "game/stone.h"
#include "preferences.h"

#include "gui/graphicsview/themerenderer.h"

#include <QIcon>
namespace Kigo {

GameWidget::GameWidget(Game *game, QWidget *parent)
    : QWidget(parent), m_game(game)
{
    Q_ASSERT(m_game);
    setupUi(this);

    finishButton->setIcon(QIcon::fromTheme( QStringLiteral( "media-playback-stop" )));

    connect(m_game, &Game::boardInitialized, this, &GameWidget::init);
    connect(m_game, &Game::boardChanged, this, &GameWidget::update);
    connect(m_game, &Game::consecutivePassMovesPlayed, this, &GameWidget::enableFinishButton);
    connect(finishButton, &QPushButton::clicked, this, &GameWidget::finishButtonClicked);
}

void GameWidget::init()
{
    if (!m_game->isRunning()) {
        //qCDebug(KIGO_LOG) << "Game is not running, no information update";
        return;
    }

    if (Preferences::whitePlayerHuman()) {
        whiteNameLabel->setText(Preferences::whitePlayerName());
    } else {
        whiteNameLabel->setText(i18n("Computer (level %1)", Preferences::whitePlayerStrength()));
    }
    if (Preferences::blackPlayerHuman()) {
        blackNameLabel->setText(Preferences::blackPlayerName());
    } else {
        blackNameLabel->setText(i18n("Computer (level %1)", Preferences::blackPlayerStrength()));
    }
    komiLabel->setText(QString::number(m_game->komi()));
    handicapLabel->setText(i18np("%1 Stone", "%1 Stones", m_game->fixedHandicap()));

    const QPixmap whiteStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::WhiteStone, QSize(48, 48));
    whiteStoneImageLabel->setPixmap(whiteStone);
    const QPixmap blackStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::BlackStone, QSize(48, 48));
    blackStoneImageLabel->setPixmap(blackStone);

    update();
}

void GameWidget::update()
{
    moveLabel->setText(QString::number(m_game->currentMoveNumber()));

    if (!m_game->moves().isEmpty()) {
        const Move last = m_game->lastMove();
        if (last.player()->isWhite()) {
            lastMoveLabel->setText(i18nc("Indication who played the last move", "%1 (white)", last.stone().toString()));
        } else if (last.player()->isBlack()) {
            lastMoveLabel->setText(i18nc("Indication who played the last move", "%1 (black)", last.stone().toString()));
        } else {
            lastMoveLabel->clear();
        }
    }

    if (m_game->currentPlayer().isWhite()) {
        const QPixmap whiteStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::WhiteStone, QSize(64, 64));
        currentPlayerImageLabel->setPixmap(whiteStone);
        currentPlayerLabel->setText(i18n("White's turn"));
    } else if (m_game->currentPlayer().isBlack()) {
        const QPixmap blackStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::BlackStone, QSize(64, 64));
        currentPlayerImageLabel->setPixmap(blackStone);
        currentPlayerLabel->setText(i18n("Black's turn"));
    } else {
        currentPlayerLabel->clear();
    }

    whiteCapturesLabel->setText(i18np("%1 capture", "%1 captures", m_game->captures(m_game->whitePlayer())));
    blackCapturesLabel->setText(i18np("%1 capture", "%1 captures", m_game->captures(m_game->blackPlayer())));
}

void GameWidget::finishButtonClicked()
{
    finishButton->setEnabled(false);
    Q_EMIT finishClicked();
}

void GameWidget::enableFinishButton()
{
    finishButton->setEnabled(true);
}

} // End of namespace Kigo

#include "moc_gamewidget.cpp"
