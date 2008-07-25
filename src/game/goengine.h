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
 * @file This file is part of KGO and defines the classes GoEngine, GoEngine::Stone
 *       and GoEngine::Score, which together implement a Go Text Protocol (GTP)
 *       interface to communicate with Go engines supporting GTP protocol
 *       version 2.
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 */
#ifndef KGO_GOENGINE_H
#define KGO_GOENGINE_H

#include <QProcess>
#include <QList>
#include <QPair>
#include <QString>

namespace KGo {

/**
 * The GoEngine class implements the Go game and acts as a wrapper around a
 * remote Go Game engine implementing the Go Text Protocol (GTP). It uses
 * GTP protocol version 2 and interfaces the engine executable in an
 * asynchronous manor. The best supported engine should (naturally)
 * be GnuGo.
 *
 * @code
 * GoEngine *engine = new GoEngine;
 *
 * // Run a session with a Go engine in GTP mode
 * engine->run("gnugo --mode gtp");
 *
 * // Get some informations about the Go engine
 * engine->name();
 * engine->version();
 * engine->help();
 *
 * engine->quit();
 * @endcode
 *
 * @author Sascha Peilicke <sasch.pe@gmx.de>
 * @since 0.1
 */
class GoEngine : public QObject
{
    Q_OBJECT

public:
    /**
     * Enumeration of available player colors.
     */
    enum PlayerColor {
        WhitePlayer = 1,        ///< The white player
        BlackPlayer,            ///< The black player
        InvalidPlayer           ///< Is only used as return value
    };

    /**
     * Enumeration of what color the stone has on a certain board field.
     */
    enum FieldStatus {
        EmptyField = 1,         ///< The field is empty
        WhiteField,             ///< The field has a white stone on it
        BlackField,             ///< The field has a black stone on it
        InvalidField            ///< An invalid field was queried, shows error
    };

    /**
     * Enumeration of all possible states of a dragon
     */
    enum DragonStatus {
        AliveDragon = 1,        ///< The dragon lives
        CriticalDragon,         ///< The dragon is critical
        DeadDragon,             ///< The dragon is dead
        UnknownDragon,          ///< The state of the dragon is unknown
    };

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

    /**
     * This class represents a stone on a field of the game board.
     */
    class Stone
    {
    public:
        /**
         * Standard constructor. Can optionally set from the given coordinates.
         *
         * @param x The x coordinate of the stone
         * @param y The y coordinate of the stone
         */
        Stone(char x = -1, int y = -1) : m_x(x), m_y(y) {}

        /**
         * Constructor to set from a stone given as a string.
         *
         * @param stone The stone as string
         */
        Stone(const QString &stone);

        /**
         * Checks wether the stone is valid or not
         */
        bool isValid() const;

        /**
         * Returns the stone as a string
         */
        QString toString() const;

        /**
         * Returns the stone as a byte array
         */
        QByteArray toLatin1() const;

        /**
         * Retrieves the x coordinate of the stone
         */
        char x() const { return m_x; }

        /**
         * Retrieves the y coordinate of the stone
         */
        int y() const { return m_y; }

    private:
        char m_x;       ///< The horizontal field coordinate (from 'A' to 'T')
        int m_y;        ///< The vertical field coordinate (from 1 to 19)
    };

    /**
     * This class is used as return value by the 'newScore', 'estimateScore'
     * and 'finalScore' GoEngine methods.
     */
    class Score
    {
    public:
        /**
         * Standard constructor. Used to create a Score object from a
         * score string returned by the Go engine.
         *
         * @param scoreString The score string from the engine
         */
        Score(const QString &scoreString = QString());

        /**
         * Checks wether the score is valid or not.
         */
        bool isValid() const;

        /**
         * Converts the score into a string.
         */
        QString toString() const;

        /**
         * Returns the player which currently leads by points.
         */
        PlayerColor player() const { return m_player; }

        /**
         * Returns the amount of points (aka score) which the leading
         * player has more than the other.
         */
        int score() const { return m_score; }

        /**
         * Returns the lower bound for a score estimate.
         */
        int lowerBound() const { return m_lowerBound; }

        /**
         * Returns the upper bound for a score estimate.
         */
        int upperBound() const { return m_upperBound; }

    private:
        PlayerColor m_player;   ///< The player which has more points
        int m_score;            ///< The amount of points the player leads with
        int m_lowerBound;       ///< Estimate lower bound
        int m_upperBound;       ///< Estimate upper bound
    };

    ////////////////////////////////////////////////////////////////////

    GoEngine();
    ~GoEngine();

    ////////////////////////////////////////////////////////////////////
    // GTP: Administrative commands
    ////////////////////////////////////////////////////////////////////

    /**
     * Connect to the given Go game engine in GTP mode. The most common
     * used case is 'gnugo --mode gtp' but this depends on your platform
     * and installed Go engine(s).
     *
     * @param command The executable command to start in GTP mode.
     */
    bool run(const QString &command = "gnugo --mode gtp");

    /**
     * Check wether the GoEngine object is connected to a Go engine, running
     * and waiting for commands to be fed with.
     */
    bool isRunning() const { return m_process.state() == QProcess::Running; }

    /**
     * Retrieve the last response the Go game engine made  occuredafter it
     * received a command.
     */
    QString response() const { return m_response; }

    /**
     * Gracefully stop and exit the Go game engine.
     */
    void quit();

    /**
     * Load a SGF file, possibly up to a move number or the first
     * occurence of a move.
     *
     * @param fileName The SGF file name
     * @param moveNumber The move number
     */
    bool loadSgf(const QString &fileName, int moveNumber = 0);

    /**
     * Save the current game as a SGF filer.
     *
     * @param fileName The SGF file name
     */
    bool saveSgf(const QString &fileName);

    ////////////////////////////////////////////////////////////////////
    // GTP: Program identity
    ////////////////////////////////////////////////////////////////////

    /**
     * Retrieves the name of the Go game engine application.
     */
    QString name();

    /**
     * Retrieves the GTP protocol version implemented by the Go game engine.
     */
    int protocolVersion();

    /**
     * Retrieves the version of the Go game engine application.
     */
    QString version();

    ////////////////////////////////////////////////////////////////////
    // GTP: Setting the board size, clearing
    ////////////////////////////////////////////////////////////////////

    /**
     * Set the board size to NxN.
     *
     * @param size The board size (standard are 9x9, 13x13 and 19x19)
     */
    bool setBoardSize(int size);

    /**
     * Retrieves the current board size.
     */
    int boardSize();

    /**
     * Clears the board.
     */
    bool clearBoard();

    ////////////////////////////////////////////////////////////////////
    // GTP: Setting komi, level, handicap
    ////////////////////////////////////////////////////////////////////

    /**
     * Set the komi.
     *
     * @param komi The komi to be set (usually 4.5 or 4.5 on 19x19 boards)
     */
    bool setKomi(float komi);

    /**
     * Set the Go engine strength level.
     *
     * @param level The strength level (from 1 to 10)
     */
    bool setLevel(int level);

    /**
     * Set up fixed placement handicap stones.
     *
     * @param handicap The number of handicap stones.
     */
    bool setFixedHandicap(int handicap);

    ////////////////////////////////////////////////////////////////////
    // GTP: Playing and generating moves
    ////////////////////////////////////////////////////////////////////

    /**
     * Play a stone for the player with 'color' at 'field'
     *
     * @param color The player's color for whom to play the stone
     * @param field The board field on which to play the stone
     */
    bool playMove(PlayerColor color, const Stone &field);

    /**
     * Let the player with 'color' pass.
     *
     * @param color The player's color who want's to pass
     */
    bool passMove(PlayerColor color);

    /**
     * Generate and play the supposedly best move for 'color'.
     *
     * @param color Play the move for player with that color
     */
    bool generateMove(PlayerColor color);

    /**
     * Undo last move.
     *
     * @param i
     */
    bool undoMove(int i = 1);

    /////////////////////////////////////////////////////////////////////
    // GTP: Retractable moves
    /////////////////////////////////////////////////////////////////////

    /**
     * Play a stone of the given color at the given field.
     *
     * @param color Play a stone for that color
     * @param field Play a stone at that field
     */
    bool tryMove(PlayerColor color, const Stone &field);

    /**
     * Undo a trymove.
     */
    bool popGo();

    ////////////////////////////////////////////////////////////////////
    // GTP: Board status
    ////////////////////////////////////////////////////////////////////

    /**
     * Returns a pair of the last move and the player's color who made it.
     *
     * @return A Pair with the player color and the last move.
     */
     QPair<PlayerColor, Stone> lastMove();

    /**
     * Return the color (state) of a specific board field.
     *
     * @param field The field to report the state
     * @see FieldStatus
     */
    FieldStatus whatColor(const Stone &field);

    /**
     * Returns a list of all stones of 'color' on the board.
     *
     * @param color The player color list the stones for
     * @return
     */
    QList<Stone> listStones(PlayerColor color);

    /**
     * Count the number of liberties for the stone at 'field'.
     *
     * @param field The field to count liberties for
     */
    int countLiberties(const Stone &field);

    /**
     * Returns the positions of the liberties for the stone at 'field'.
     *
     * @param field The field to return liberties for
     */
    QList<Stone> findLiberties(const Stone &field);

    /**
     * Tell wether a move at 'field' for player 'color' is legal.
     *
     * @param color The player to test the move for
     * @param field The field to test the move for
     */
    bool isLegal(PlayerColor color, const Stone &field);

    /**
     * Generate a list of the best moves for a player with 'color' with
     * weights.
     *
     * @param color Player to generate the list for
     */
    QString topMoves(PlayerColor color);

    /**
     * List all legal moves for either color.
     *
     * @param color List legal moves for this color
     */
    QList<Stone> legalMoves(PlayerColor color);

    /**
     * List the number of captures taken by either color.
     */
    int captures(PlayerColor color);

    /////////////////////////////////////////////////////////////////////
    // GTP: Tactical reading
    /////////////////////////////////////////////////////////////////////

    /**
     * Try to attack a field.
     *
     * @param field The field to attack
     */
    QString attack(const Stone &field);

    /**
     * Try to defend a field.
     *
     * @param field The field to defend>
     */
    QString defend(const Stone &field);

    /**
     * Increase depth values by one.
     */
    bool increaseDepths();

    /**
     * Decrease depth values by one.
     */
    bool decreaseDepths();

    /////////////////////////////////////////////////////////////////////
    // GTP: OWL reading and eyes
    /////////////////////////////////////////////////////////////////////

    /**
     * Try to attack a dragon.
     *
     * @param field The dragon field to attack
     */
    QString owlAttack(const Stone &field);

    /**
     * Try to defend a dragon.
     *
     * @param field The dragon field to defend
     */
    QString owlDefense(const Stone &field);

    /**
     * Evaluate an eye space and returns a minumum and maximum number of
     * eyes. If these differ an attack and a defense point are additionally
     * returned. If the field is not an eye space or not of unique color,
     * a single -1 is returned.
     *
     * @param field The eye field to evaluate
     * @return Minimum and maximum nuber of eyes.
     * @see EyeInfo
     */
    QString evalEye(const Stone &field);

    /////////////////////////////////////////////////////////////////////
    // GTP: Dragon status
    /////////////////////////////////////////////////////////////////////

    /**
     * Determine status of a dragon. Returns the dragon status, attack
     * points, defense points. Points of attack and defense are only given
     * if the status is critical and the owl code is enabled.
     *
     * @param field The dragon stone
     * @see DragonStatus
     */
    DragonStatus dragonStatus(const Stone &field);

    /**
     * Determine wether two stones belong to the same dragon.
     *
     * @param field1 The first stone
     * @param field2 The second stone
     */
    bool sameDragon(const Stone &field1, const Stone &field2);

    /**
     * Returns the information in the dragon data structure. The data
     * is formatted corresponding to gtp_worm__. If a field is given
     * only worm data for the field is returned.
     *
     * @param field The field for which information is wanted
     */
    QString dragonData(const Stone &field = Stone());

    /////////////////////////////////////////////////////////////////////
    // GTP: Score
    /////////////////////////////////////////////////////////////////////

    /**
     * Report the final status of a field in a finished game.
     *
     * @param field Report status for that feed
     * @see FinalState
     */
    FinalState finalStatus(const Stone &field);

    /**
     * Report fields with a specified final status in a finished game.
     *
     * @param state Report only fields with that state
     * @see FinalState
     */
    QList<Stone> finalStatusList(FinalState state);

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

    /////////////////////////////////////////////////////////////////////
    // GTP: Statistics
    /////////////////////////////////////////////////////////////////////

    /**
     * Retrieve the count of life nodes.
     *
     * @return The number of life nodes
     */
    int getLifeNodeCounter();

    /**
     * Reset the count of life nodes.
     */
    bool resetLifeNodeCounter();

    /**
     * Retrieve the count of owl nodes.
     *
     * @return The number of owl nodes.
     */
    int getOwlNodeCounter();

    /**
     * Reset the count of owl nodes.
     */
    bool resetOwlNodeCounter();

    /**
     * Retrieve the count of reading nodes.
     *
     * @return The number of reading nodes.
     */
    int getReadingNodeCounter();

    /**
     * Reset the count of reading nodes.
     */
    bool resetReadingNodeCounter();

    /**
     * Retrieve the count of trymoves/trykos.
     */
    int getTryMoveCounter();

    /**
     * Reset the count of trymoves/trykos.
     */
    bool resetTryMoveCounter();

    /////////////////////////////////////////////////////////////////////
    // GTP: debug
    /////////////////////////////////////////////////////////////////////

    /**
     * Write the complete board to stdout.
     */
    bool showBoard();

    /**
     * Dump stack to stdout.
     */
    bool dumpStack();

    /**
     * Return the cutstone field in the worm data structure.
     *
     * @param field The field to inspect
     * @return The cutstone value of worm data for that field
     */
     int wormCutStone(const Stone &field);

    /**
     * Return the information in the worm data structure. If no
     * valid field is given, worm data is returned for all fields.
     *
     * @param field The field to inspect
     * @return Worm data formatted
     */
    QString wormData(const Stone &field = Stone());

    /**
     * List of lists of linked together stones around a given field
     * from the worm data structure.
     *
     * @param field The location
     */
    QList<Stone> wormStones(const Stone &field);

    /**
     * Tune the parameters for the move ordering in the tactical reading.
     *
     * @param parameters The move ordering parameters
     */
    bool tuneMoveOrdering(int parameters);

    /**
     * List all known commands.
     *
     * @return The list of known commands
     */
    QList<QString> help();

    /**
     * Turn uncertainty reports from owl_attack and owl_defend on or off.
     *
     * @param enabled Wether to turn uncertainty reports on or off
     */
    bool reportUncertainty(bool enabled);

    /**
     * Sends a custom command to the Go engine and returns the result.
     *
     * @param command The custom command
     * @return Result of the command
     */
    QString shell(const QString &command);

    /**
     * Evaluate wether a command is known.
     *
     * @param command The command to evaluate
     */
    bool knownCommand(const QString &command);

    /**
     * A simple echo command to test the engine.
     *
     * @param command The command to echo
     * @return
     */
    QString echo(const QString &command);

signals:
    /**
     * This signal is emmited when the process encounters an error.
     */
    void error(QProcess::ProcessError);

    /**
     * This signal is emitted when all heavy computation is over
     * and the Go engine is ready to receive the next command.
     */
    void ready();

    /**
     * This signal is emitted when the board situation changed and
     * can be used to trigger an update to a visual representation.
     */
    void boardChanged();

    void nextTurn(PlayerColor);

private slots:
    /**
     * This slot reads everything from the process's stdout
     */
    void readStandardOutput();

    /**
     * This slot reads everything from the process's stderr. This is
     * mainly for debugging purposes, because this should only happen
     * if a bug occured.
     */
    void readStandardError();

    /**
     * Wait gracefully for a response from the Go engine. The returned string
     * from the Go engine is stored in 'm_response'.
     */
    bool waitResponse();

private:
    QString m_response;     ///< Stores the last answer from the engine
    QProcess m_process;     ///< To interact with the engine executable
};

} // End of namespace KGo

#endif
