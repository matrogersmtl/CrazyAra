/*
  CrazyAra, a deep learning chess variant engine
  Copyright (C) 2018       Johannes Czech, Moritz Willig, Alena Beyer
  Copyright (C) 2019-2020  Johannes Czech

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License fåor more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
 * @file: state.h
 * Created on 13.07.2020
 * @author: queensgambit
 *
 * State is an abstract class which is used in the MCTS as a generic interface for various environments.
 * It uses the curiously recurring template pattern (CRTP) idiom to reduce the number of virtual methods.
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

typedef uint64_t Key;
typedef int Action;
const int ACTION_NONE = 0;

enum TerminalType {
    TERMINAL_LOSS,
    TERMINAL_DRAW,
    TERMINAL_WIN,
    TERMINAL_CUSTOM,
    TERMINAL_NONE
};

enum Result {
    DRAWN = 0,
    WHITE_WIN,
    BLACK_WIN,
    NO_RESULT,
};

template <class T>
class State
{
public:

    /**
     * @brief leads_to_terminal Checks if a given action leads to a terminal state
     * @param a Given action
     * @return true if leads to terminal, else false
     */
    bool leads_to_terminal(Action a)
    {
        std::unique_ptr<State> posCheckTerminal = static_cast<T*>(this)->clone();
        bool givesCheck = posCheckTerminal->gives_check(a);
        posCheckTerminal->do_action(a);
        return posCheckTerminal->check_result(givesCheck) != NO_RESULT;
    }

    /**
     * @brief legal_actions Returns all legal actions as a vector list
     * @return vector of legal actions
     */
    std::vector<Action> legal_actions() {
        return static_cast<T*>(this)->legal_actions();
    }

    /**
     * @brief set Sets a new states and modifies the current state.
     * @param fenStr String description about the state
     * @param isChess960 If true 960 mode will be active
     * @param variant Variant which the position corresponds to.
     * @return An alias to the updated state
     */
    void set(const std::string& fenStr, bool isChess960, int variant) {
        static_cast<T*>(this)->set(fenStr, isChess960, variant);
    }

    /**
     * @brief get_state_planes Returns the state plane representation of the current state which can be used for NN inference.
     * @param normalize If true thw normalized represnetation should be returned, otherwise the raw representation
     * @param inputPlanes Pointer to the memory array where to set the state plane representation. It is assumed that the memory has already been allocated
     */
    void get_state_planes(bool normalize, float* inputPlanes) const {
        static_cast<const T*>(this)->get_state_planes(normalize, inputPlanes);
    }

    /**
     * @brief steps_from_null Number of steps form the initial position (e.g. starting position)
     * @return number of steps
     */
    unsigned int steps_from_null() const {
        return static_cast<const T*>(this)->steps_from_null();
    }

    /**
     * @brief is_chess960 Returns true if the position is a 960 random position, else false
     * @return bool
     */
    bool is_chess960() const {
        return static_cast<const T*>(this)->is_chess960();
    }

    /**
     * @brief fen Returns the fen or string description of the current state
     * @return string
     */
    std::string fen() const {
        return static_cast<const T*>(this)->fen();
    }

    /**
     * @brief do_action Applies a given action to the current state
     * @param action Type of action to apply. It is assumed that the action is discrete and integer format
     */
    void do_action(Action action) {
        static_cast<T*>(this)->do_action(action);
    }

    /**
     * @brief undo_action Undos a given action
     * @param action Type of action to apply. It is assumed that the action is discrete and integer format
     */
    void undo_action(Action action) {
        static_cast<T*>(this)->undo_action(action);
    }

    /**
     * @brief number_repetitions Returns the number of times this state has already occured in the current episode
     * @return int
     */
    unsigned int number_repetitions() const {
        return static_cast<const T*>(this)->number_repetitions();
    }

    /**
     * @brief side_to_move Returns the side to move (e.g. Color: WHITE or BLACK) in chess
     * @return int
     */
    int side_to_move() const {
        return static_cast<const T*>(this)->side_to_move();
    }

    /**
     * @brief hash_key Returns a uique identifier for the current position which can be used for accessing the hash table
     * @return
     */
    Key hash_key() const {
        return static_cast<const T*>(this)->hash_key();
    }

    /**
     * @brief flip Flips the state along the x-axis
     */
    void flip() {
        return static_cast<T*>(this)->flip();
    }

    /**
     * @brief uci_to_action Converts the given action in uci notation to an action object
     * @param uciStr uci specification for the action
     * @return Action
     */
    Action uci_to_action(std::string& uciStr) const {
        return static_cast<const T*>(this)->uci_to_action(uciStr);
    }

    /**
     * @brief action_to_san Converts a given action to SAN (pgn move notation) usign the current position and legal moves
     * @param action Given action
     * @param legalActions List of legal moves for the current position
     * @param leadsToWin Indicator which marks action as a terminating action (usually indicated with suffix #).
     * @param bookMove Indicator which marks action as book move
     * @return SAN string
     */
    std::string action_to_san(Action action, const std::vector<Action>& legalActions, bool leadsToWin=false, bool bookMove=false) const {
        return static_cast<const T*>(this)->action_to_san(action, legalActions, leadsToWin, bookMove);
    }

    /**
     * @brief is_terminal Returns the terminal type for the current state. If the state is a non terminal state,
     * then TERMINAL_NONE should be returned.
     * @param numberLegalMoves Number of legal moves in the current position
     * @param inCheck Boolean which defines if there is a check in the current position
     * @param customTerminalValue Value which will be assigned to the node value evaluation. You need to return TERMINAL_CUSTOM in this case;
     * otherwise the value will later be overwritten. In the default case, this parameter can be ignored.
     * @return TerminalType
     */
    TerminalType is_terminal(size_t numberLegalMoves, bool inCheck, float& customTerminalValue) const {
        return static_cast<const T*>(this)->is_terminal(numberLegalMoves, inCheck, customTerminalValue);
    }

    /**
     * @brief check_result Returns the current game result. In case a normal position is given NO_RESULT is returned.
     * @param inCheck Determines if a king in the current position is in check (needed to differ between checkmate and stalemate).
     * It can be computed by `gives_check(<last-move-before-current-position>)`.
     * @return value in [DRAWN, WHITE_WIN, BLACK_WIN, NO_RESULT]
     */
    Result check_result(bool inCheck) const {
        return static_cast<const T*>(this)->check_result(inCheck);
    }

    /**
     * @brief gives_check Checks if the current action is a checking move
     * @param action Action
     * @return bool
     */
    bool gives_check(Action action) const {
        return static_cast<const T*>(this)->gives_check(action);
    }

    /**
     * @brief print Print method used for the operator <<
     * @param os OS stream object
     */
    void print(std::ostream& os) const {
        static_cast<const T*>(this)->print(os);
    }

    /**
     * @brief operator << Operator overload for <<
     * @param os ostream object
     * @param state state object
     * @return ostream
     */
    friend std::ostream& operator<<(std::ostream& os, const State& state)
    {
        state.print(os);
        return os;
    }

    /**
     * @brief clone Clones the current state as a deep copy
     * @return deep copy
     */
    virtual std::unique_ptr<T> clone() const = 0;
};

#endif // GAMESTATE_H
