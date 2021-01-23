/*
    SPDX-FileCopyrightText: 2008 Sascha Peilicke <sasch.pe@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIGO_GAME_H
#define KIGO_GAME_H

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
 * The Game class implements the Go game and acts as a wrapper around a
 * remote Go Game game implementing the Go Text Protocol (GTP). It uses
 * GTP protocol version 2 and interfaces the game executable in an
 * synchronous manor. The best supported game should (naturally)
 * be GnuGo.
 *
 * @code
 * Game *game = new Game;
 *
 * // Run a session with a Go game in GTP mode
 * game->start("gnugo --mode gtp");
 *
 * // Get some information about the Go game
 * game->name();
 * game->version();
 *
 * game->stop();
 * @endcode
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class Game : public QObject
{
    Q_OBJECT

public:
    /**
     * Enumeration of all possible final states of a field when a game is over
     */
    enum class FinalState {
        FinalAlive = 1,         ///< The stone on the field is alive
        FinalDead,              ///< The stone on the field is dead
        FinalSeki,              ///<
        FinalWhiteTerritory,    ///< The field belongs to the white player
        FinalBlackTerritory,    ///< The field belongs to the black player
        FinalDame,              ///< The field belongs to no player
        FinalStateInvalid       ///< The state is invalid, shows error
    };

    explicit Game(QObject *parent = nullptr);
    ~Game() override;

    /**
     * Connect to the given Go game game in GTP mode. The most common
     * used case is 'gnugo --mode gtp' but this depends on your platform
     * and installed Go game(s).
     *
     * @param command The executable command to start in GTP mode.
     */
    bool start(const QString &command);

    /**
     * Gracefully stop and exit the Go game game.
     */
    void stop();

    /**
     * Check whether the Game object is connected to a Go game, running
     * and waiting for commands to be fed with.
     */
    bool isRunning() const { return m_process.state() == QProcess::Running; }
    QString engineName() const { return m_engineName; }
    QString engineVersion() const { return m_engineVersion; }
    QString engineCommand() const { return m_engineCommand; }
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

    Player &currentPlayer() { return *m_currentPlayer; }
    const Player &currentPlayer() const { return *m_currentPlayer; }
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
     * Returns the number of moves in the game so far.
     * Especially helpful when loading a game.
     */
    int moveCount();

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
     * Compute the score of a finished game. This method can take a
     * long time to complete. It is thus recommended to use estimatedScore().
     *
     * @return Score in SGF format (RE property)
     */
    Score finalScore();

    /**
     * Returns an estimate of the final score based on the current game
     * situation.
     */
    Score estimatedScore();

    bool canRedo() const { return m_undoStack.canRedo(); }
    bool canUndo() const { return m_undoStack.canUndo(); }

    bool isFinished() const { return m_gameFinished; }

Q_SIGNALS:
    /**
     * This signal is emitted when the board is first started and
     * can be used to trigger an update to a visual representation.
     */
    void boardInitialized();

    /**
     * This signal is emitted when the board situation changed and
     * can be used to trigger an update to a visual representation.
     */
    void boardChanged();

    /** This signal is emitted when the board size was changed. */
    void boardSizeChanged(int);

    /** This signal is emitted when a player resigns. */
    void resigned(const Kigo::Player &);

    void passMovePlayed(const Kigo::Player &);
    /**
     * This signal is emitted when both players played a pass move
     * after another. It is also send when pass moves are gone
     * (which is indicated by a value of 0).*/
    void consecutivePassMovesPlayed(int);

    /** This signal is emitted when the current player changes. */
    void currentPlayerChanged(const Kigo::Player &);

    /**
     * This signal is emitted when the game starts or ends a
     * non-blocking wait. This is useful to reflect the wait state
     * in the UI.
     */
    void waiting(bool);

    /** This signal is emitted when availability of redo moves changes */
    void canRedoChanged(bool);

    /** This signal is emitted when availability of undo moves changes */
    void canUndoChanged(bool);

public Q_SLOTS:
    void gameSetup();
    
private Q_SLOTS:
    /**
     * Wait gracefully for a response from the Go game. The returned string
     * from the Go game is stored in 'm_response'.
     *
     * @param nonBlocking This should be set for commands that take a long
     *                    time to complete to avoid ugly UI blocking.
     */
    bool waitResponse(bool nonBlocking = false);

    /**
     * Slot to handle QProcess's readyRead signal.
     */
    void readyRead();

    void undoIndexChanged(int index);

private:
    void setCurrentPlayer(Player &player);

    QProcess m_process;

    QString m_engineName;
    QString m_engineVersion;
    QString m_engineCommand;
    QString m_response;

    QList<Move> m_movesList;
    int m_currentMove;
    QUndoStack m_undoStack;
    int m_lastUndoIndex;

    Player *m_currentPlayer;
    Player m_blackPlayer;
    Player m_whitePlayer;

    float m_komi;
    int m_boardSize;
    int m_fixedHandicap;
    int m_consecutivePassMoveNumber;
    bool m_waitAndProcessEvents;
    bool m_gameFinished;
};

} // End of namespace Kigo

#endif
