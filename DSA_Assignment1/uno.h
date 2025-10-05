#ifndef UNO_H
#define UNO_H

#include <string>
#include <vector>

struct Card {

};

class UNOGame {
public:
    // Initialize game with given number of players
    UNOGame(int numPlayers);

    // Shuffle and deal cards to players
    virtual void initialize();

    // Play one turn of the game
    virtual void playTurn();

    // Return true if game has a winner
    virtual bool isGameOver() const;

    // Return index of winner
    virtual int getWinner() const;

    // Return current state as string
    virtual std::string getState() const;
};

#endif#pragma once
