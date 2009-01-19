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

#ifndef KIGO_ENGINE_H
#define KIGO_ENGINE_H

#include "move.h"
#include "player.h"
#include "stone.h"

#include <QList>
#include <QProcess>
#include <QString>
#include <QUndoStack>

namespace Kigo {

class Score;

/**
 * The Engine class implements the Go game and acts as a wrapper around a
 * remote Go Game engine implementing the Go Text Protocol (GTP). It uses
 * GTP protocol version 2 and interfaces the engine executable in an
 * synchronous manor. The best supported engine should (naturally)
 * be GnuGo.
 *
 * @code
 * Engine *engine = new Engine;
 *
 * // Run a session with a Go engine in GTP mode
 * engine->start("gnugo --mode gtp");
 *
 * // Get some information about the Go engine
 * engine->name();
 * engine->version();
 *
 * engine->stop();
 * @endcode
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class Engine : public QObject
{
    Q_OBJECT

public:
    /**
     * Enumeration of all possible final states of a field when a game is over
     */
    enum FinalState {
        FinalAlive = 1,         ///< The stone on the field is alive
        FinalDead,              ///< The stone on the field is dead
        FinalSeki,              ///<
        FinalWhiteTerritory,    ///< The field belongs to the white player
        FinalBlackTerritory,    ///< The field belongs to the black player
        FinalDame,              ///< The field belongs to no player
        FinalStateInvalid       ///< The state is invalid, shows error
    };

    explicit Engine(QObject *parent = 0);
    ~Engine();

    /**
     * Connect to the given Go game engine in GTP mode. The most common
     * used case is 'gnugo --mode gtp' but this depends on your platform
     * and installed Go engine(s).
     *
     * @param command The executable command to start in GTP mode.
     */
    bool start(const QString &command = "gnugo --mode gtp");

    /**
     * Gracefully stop and exit the Go game engine.
     */
    void stop();

    /**
     * Check whether the Engine object is connected to a Go engine, running
     * and waiting for commands to be fed with.
     */
    bool isRunning() const { return m_process.state() == QProcess::Running; }
    QString name() const { return m_engineName; }
    QString version() const { return m_engineVersion; }
    QString command() const { return m_engineCommand; }
    QUndoStack *undoStack() { return &m_undoStack; }

    /**
     * Initialize a new game.
     */
    bool init();

    /**
     * Initialize from a SGF file and start from a specific move number
     * if desired.
     *
     * @param fileName The SGF file name
     * @param moveNumber The move number
     */
    bool init(const QString &fileName, int moveNumber = 0);

    /**
     * Save the current game as a SGF file.
     *
     * @param fileName The SGF file name
     */
    bool save(const QString &fileName);

    /**
     * Set the board size to NxN.
     *
     * @param size The board size (standard are 9x9, 13x13 and 19x19)
     */
    bool setBoardSize(int size);
    int boardSize() const { return m_boardSize; }

    /**
     * Set the komi.
     *
     * @param komi The komi to be set (usually 4.5 or 4.5 on 19x19 boards)
     */
    bool setKomi(float komi);
    float komi() const { return m_komi; }

    /**
     * Set up fixed placement handicap stones.
     *
     * @param handicap The number of handicap stones.
     */
    bool setFixedHandicap(int handicap);
    int fixedHandicap() const { return m_fixedHandicap; }

    /**
     * Returns the maximum amount fixed handicap stones placeable at the
     * current Go board size.
     *
     * @return The maximum allowed fixed handicap
     */
    int fixedHandicapUpperBound();

    Player &currentPlayer() { return m_currentPlayer; }
    const Player &currentPlayer() const { return m_currentPlayer; }
    Player &whitePlayer() { return m_whitePlayer; }
    const Player &whitePlayer() const { return m_whitePlayer; }
    Player &blackPlayer() { return m_blackPlayer; }
    const Player &blackPlayer() const { return m_blackPlayer; }

    bool playMove(const Move &move, bool undoable = true);
    bool playMove(const Player &player, const Stone &stone = Stone(), bool undoable = true);
    bool generateMove(const Player &player, bool undoable = true);
    bool undoMove();
    bool redoMove();

    int currentMoveNumber() const { return m_currentMove; }

    /**
     * Returns the last move made by either player. Don't call this method
     * when no moves where made at all!
     */
    Move lastMove() const;

    /**
     * Returns a list of all stones of that player on the board.
     */
    QList<Stone> stones(const Player &player);

    /**
     * Returns a list of all moves by that player.
     */
    QList<Move> moves(const Player &player);
    QList<Move> moves() { return m_movesList; }

    /**
     * Returns the positions of the liberties for the stone at 'field'.
     *
     * @param field The field to return liberties for
     */
    QList<Stone> liberties(const Stone &field);

    /**
     * Generate a list of the best moves for a player with weights.
     */
    QList<Stone> bestMoves(const Player &player);

    /**
     * List all legal moves for either player.
     */
    QList<Stone> legalMoves(const Player &player);

    /**
     * List the number of captures taken by either player.
     */
    int captures(const Player &player);

    /**
     * Report the final status of a field in a finished game.
     *
     * @param field Report status for that feed
     * @see FinalState
     */
    FinalState finalState(const Stone &field);

    /**
     * Report fields with a specified final status in a finished game.
     *
     * @param state Report only fields with that state
     * @see FinalState
     */
    QList<Stone> finalStates(FinalState state);

    /**
     * Compute the score of a finished game.
     *
     * @return Score in SGF format (RE property)
     */
    Score finalScore();

    /**
     * Returns an estimate of the final score based on the current game
     * situation.
     */
    Score estimateScore();

signals:
    /**
     * This signal is emitted when the board situation changed and
     * can be used to trigger an update to a visual representation.
     */
    void changed();

    /** This signal is emitted when the board size was changed. */
    void sizeChanged(int);

    /** This signal is emitted when a player resigns. */
    void resigned(const Player &);

    /**
     * This signal is emitted when both players played a pass move
     * after another. It is also send when pass moves are gone
     * (which is indicated by a value of 0).*/
    void consecutivePassMovesPlayed(int);

    /** This signal is emitted when the current player changes. */
    void currentPlayerChanged(const Player &);

    /**
     * This signal is emitted when the engine starts or ends a
     * non-blocking wait. This is useful to reflect the wait state
     * in the UI.
     */
    void waiting(bool);

    /** This signal is emitted when availability of redo moves changes */
    void canRedoChanged(bool);

    /** This signal is emitted when availability of undo moves changes */
    void canUndoChanged(bool);

private slots:
    /**
     * Wait gracefully for a response from the Go engine. The returned string
     * from the Go engine is stored in 'm_response'.
     *
     * @param nonBlocking This should be set for commands that take a long
     *                    time to complete to avoid ugly UI blocking.
     */
    bool waitResponse(bool nonBlocking = false);

    /**
     * Slot to handle QProcess's readyRead signal.
     */
    void readyRead();

private:
    void setCurrentPlayer(const Player &player);

    QProcess m_process;

    QString m_engineName;
    QString m_engineVersion;
    QString m_engineCommand;
    QString m_response;

    QList<Move> m_movesList;
    int m_currentMove;
    QUndoStack m_undoStack;

    Player &m_currentPlayer;
    Player m_blackPlayer;
    Player m_whitePlayer;

    float m_komi;
    int m_boardSize;
    int m_fixedHandicap;
    int m_consecutivePassMoveNumber;
    bool m_waitAndProcessEvents;
};

} // End of namespace Kigo

#endif
