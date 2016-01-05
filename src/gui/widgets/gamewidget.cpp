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
        //qDebug() << "Game is not running, no information update";
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

    QPixmap whiteStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::WhiteStone, QSize(48, 48));
    whiteStoneImageLabel->setPixmap(whiteStone);
    QPixmap blackStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::BlackStone, QSize(48, 48));
    blackStoneImageLabel->setPixmap(blackStone);

    update();
}

void GameWidget::update()
{
    moveLabel->setText(QString::number(m_game->currentMoveNumber()));

    if (m_game->moves().size() > 0) {
        Move last = m_game->lastMove();
        if (last.player()->isWhite()) {
            lastMoveLabel->setText(i18nc("Indication who played the last move", "%1 (white)", last.stone().toString()));
        } else if (last.player()->isBlack()) {
            lastMoveLabel->setText(i18nc("Indication who played the last move", "%1 (black)", last.stone().toString()));
        } else {
            lastMoveLabel->clear();
        }
    }

    if (m_game->currentPlayer().isWhite()) {
        QPixmap whiteStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::WhiteStone, QSize(64, 64));
        currentPlayerImageLabel->setPixmap(whiteStone);
        currentPlayerLabel->setText(i18n("White's turn"));
    } else if (m_game->currentPlayer().isBlack()) {
        QPixmap blackStone = ThemeRenderer::self()->renderElement(Kigo::ThemeRenderer::Element::BlackStone, QSize(64, 64));
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
    emit finishClicked();
}

void GameWidget::enableFinishButton()
{
    finishButton->setEnabled(true);
}

} // End of namespace Kigo



