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

#include <KDebug>

namespace Kigo {

GameWidget::GameWidget(Game *game, QWidget *parent)
    : QWidget(parent), m_game(game)
{
    Q_ASSERT(m_game);
    setupUi(this);

    finishButton->setIcon(KIcon( QLatin1String( "media-playback-stop" )));

    connect(m_game, SIGNAL(boardInitialized()), this, SLOT(init()));
    connect(m_game, SIGNAL(boardChanged()), this, SLOT(update()));
    connect(m_game, SIGNAL(consecutivePassMovesPlayed(int)), this, SLOT(enableFinishButton()));
    connect(finishButton, SIGNAL(clicked()), this, SLOT(finishButtonClicked()));
}

void GameWidget::init()
{
    if (!m_game->isRunning()) {
        kDebug() << "Game is not running, no information update";
        return;
    }

    if (Preferences::whitePlayerHuman()) {
        whitePlayerName->setText(Preferences::whitePlayerName());
    } else {
        whitePlayerName->setText(i18n("Computer (Level %1)", Preferences::whitePlayerStrength()));
    }
    if (Preferences::blackPlayerHuman()) {
        blackPlayerName->setText(Preferences::blackPlayerName());
    } else {
        blackPlayerName->setText(i18n("Computer (Level %1)", Preferences::blackPlayerStrength()));
    }
    komiSpinBox->setValue(m_game->komi());
    handicapSpinBox->setValue(m_game->fixedHandicap());
    handicapSpinBox->setSuffix(ki18np(" Stone", " Stones"));

    update();
}

void GameWidget::update()
{
    moveSpinBox->setValue(m_game->currentMoveNumber());

    if (m_game->moves().size() > 0) {
        Move last = m_game->lastMove();
        if (last.player()->isWhite()) {
            moveSpinBox->setSuffix(i18n(" (White %1)", last.stone().toString()));
        } else if (last.player()->isBlack()) {
            moveSpinBox->setSuffix(i18n(" (Black %1)", last.stone().toString()));
        } else {
            moveSpinBox->setSuffix("");
        }
    }

    if (m_game->currentPlayer().isWhite()) {
        playerLineEdit->setText(i18n("White's move"));
    } else if (m_game->currentPlayer().isBlack()) {
        playerLineEdit->setText(i18n("Black's move"));
    } else {
        playerLineEdit->setText("");
    }

    whiteCapturesLineEdit->setText(QString::number(m_game->captures(m_game->whitePlayer())));
    blackCapturesLineEdit->setText(QString::number(m_game->captures(m_game->blackPlayer())));
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

#include "moc_gamewidget.cpp"

