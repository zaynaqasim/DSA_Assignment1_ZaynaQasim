#include "uno.h"
#include <algorithm>
#include <deque>
#include <map>        
#include <random>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;


//A single card in UNO (color + value).

struct CardImpl {
    string color;  //"Red", "Green", "Blue", "Yellow"
    string value;  //"0"-"9", "Skip", "Reverse", "Draw Two"
};

//Hidden state for each game instance
struct UNOImpl {
    int numPlayers;                  //total players
    int currentPlayer;               //whose turn
    bool clockwise;                  //turn direction
    deque<CardImpl> deck;            //draw pile
    vector<CardImpl> discard;        //discard pile
    vector<vector<CardImpl>> hands;  //each player's hand
};

//Global storage: each UNOGame* with its implementation
static map<const UNOGame*, UNOImpl*> gameStorage;

//Retrieve internal storage safely
static UNOImpl* getImpl(const UNOGame* g) {
    if (gameStorage.find(g) == gameStorage.end()) {
        throw runtime_error("UNOGame not initialized properly.");
    }
    return gameStorage[g];
}


UNOGame::UNOGame(int numPlayers) {
    if (numPlayers < 2 || numPlayers > 4)
        throw invalid_argument("Players must be between 2 and 4");

    UNOImpl* impl = new UNOImpl();
    impl->numPlayers = numPlayers;
    impl->currentPlayer = 0;
    impl->clockwise = true;
    impl->hands.resize(numPlayers);

    //Attach impl to this game instance
    gameStorage[this] = impl;
}

void UNOGame::initialize() {
    UNOImpl* impl = getImpl(this);

    //Reset hands and discard for re-initialization safety
    for (auto& h : impl->hands) h.clear();
    impl->discard.clear();

    //Build deck (76 cards: 0 once per color, 1–9 + action cards twice each)
    vector<string> colors = { "Red", "Green", "Blue", "Yellow" };
    vector<string> values = { "0","1","2","3","4","5","6","7","8","9",
                              "Skip","Reverse","Draw Two" };

    impl->deck.clear();
    for (auto& c : colors) {
        impl->deck.push_back({ c, "0" }); //one zero per color
        for (int rep = 0; rep < 2; rep++) {
            for (size_t i = 1; i < values.size(); i++) {
                impl->deck.push_back({ c, values[i] });
            }
        }
    }

    //Shuffle deck with fixed seed (reproducible results)
    mt19937 rng(1234);
    shuffle(impl->deck.begin(), impl->deck.end(), rng);

    //Deal 7 cards per player
    for (int i = 0; i < 7; i++) {
        for (int p = 0; p < impl->numPlayers; p++) {
            if (impl->deck.empty()) break; //deck safety
            impl->hands[p].push_back(impl->deck.front());
            impl->deck.pop_front();
        }
    }

    //Ensure there is a valid starting discard card
    while (!impl->deck.empty()) {
        CardImpl starter = impl->deck.front();
        impl->deck.pop_front();

        //Avoid action cards as first discard to prevent weird starts
        if (starter.value != "Skip" && starter.value != "Reverse" && starter.value != "Draw Two") {
            impl->discard.push_back(starter);
            break;
        }
        else {
            //put action card back at bottom
            impl->deck.push_back(starter);
        }
    }

    if (impl->discard.empty())
        throw runtime_error("Failed to initialize discard pile.");
}

bool UNOGame::isGameOver() const {
    UNOImpl* impl = getImpl(this);

    //Game ends if any player has no cards left
    for (auto& h : impl->hands) {
        if (h.empty()) return true;
    }

    //Stalemate: deck is empty and no one can play
    if (impl->deck.empty()) {
        //Check if at least one player has a legal move
        CardImpl top = impl->discard.back();
        bool playableExists = false;
        for (auto& h : impl->hands) {
            for (auto& c : h) {
                if (c.color == top.color || c.value == top.value) {
                    playableExists = true;
                    break;
                }
            }
            if (playableExists) break;
        }
        return !playableExists;
    }
    return false;
}

int UNOGame::getWinner() const {
    UNOImpl* impl = getImpl(this);
    for (int i = 0; i < impl->numPlayers; i++) {
        if (impl->hands[i].empty()) return i;
    }
    return -1; //no winner yet
}



//Card can be played if color or value matches top
static bool canPlay(const CardImpl& c, const CardImpl& top) {
    return (c.color == top.color || c.value == top.value);
}

//Compute next player index with direction + skips
static int nextPlayerIndex(const UNOImpl* impl, int start, int skipCount = 0) {
    int step = impl->clockwise ? 1 : -1;
    int idx = start;
    for (int i = 0; i <= skipCount; i++) {
        idx = (idx + step + impl->numPlayers) % impl->numPlayers;
    }
    return idx;
}


void UNOGame::playTurn() {
    UNOImpl* impl = getImpl(this);
    if (isGameOver()) return; //game already finished

    CardImpl top = impl->discard.back();
    auto& hand = impl->hands[impl->currentPlayer];

    //1. Try to play a card
    int chosen = -1;

    //Priority 1: Color match
    for (size_t i = 0; i < hand.size(); i++) {
        if (hand[i].color == top.color) { chosen = (int)i; break; }
    }
    //Priority 2: Value match
    if (chosen == -1) {
        for (size_t i = 0; i < hand.size(); i++) {
            if (hand[i].value == top.value) { chosen = (int)i; break; }
        }
    }
    //Priority 3: Action cards (Skip > Reverse > Draw Two)
    if (chosen == -1) {
        for (size_t i = 0; i < hand.size(); i++) {
            if ((hand[i].value == "Skip" || hand[i].value == "Reverse" || hand[i].value == "Draw Two")
                && canPlay(hand[i], top)) {
                chosen = (int)i; break;
            }
        }
    }

    if (chosen != -1) {
        //Play the chosen card
        CardImpl played = hand[chosen];
        impl->discard.push_back(played);
        hand.erase(hand.begin() + chosen);

        //Apply action card effect
        if (played.value == "Skip") {
            impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
        }
        else if (played.value == "Reverse") {
            impl->clockwise = !impl->clockwise;
            //Special rule: in 2-player game, Reverse = Skip
            if (impl->numPlayers == 2) {
                impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
            }
            else {
                impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
            }
        }
        else if (played.value == "Draw Two") {
            int victim = nextPlayerIndex(impl, impl->currentPlayer, 0);
            for (int i = 0; i < 2 && !impl->deck.empty(); i++) {
                impl->hands[victim].push_back(impl->deck.front());
                impl->deck.pop_front();
            }
            impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
        }
        else {
            impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
        }
    }
    else {
        
        //2. No card → Draw one
        if (!impl->deck.empty()) {
            CardImpl drawn = impl->deck.front();
            impl->deck.pop_front();

            if (canPlay(drawn, top)) {
                //Immediately play drawn card
                impl->discard.push_back(drawn);

                if (drawn.value == "Skip") {
                    impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
                }
                else if (drawn.value == "Reverse") {
                    impl->clockwise = !impl->clockwise;
                    if (impl->numPlayers == 2) {
                        impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
                    }
                    else {
                        impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
                    }
                }
                else if (drawn.value == "Draw Two") {
                    int victim = nextPlayerIndex(impl, impl->currentPlayer, 0);
                    for (int i = 0; i < 2 && !impl->deck.empty(); i++) {
                        impl->hands[victim].push_back(impl->deck.front());
                        impl->deck.pop_front();
                    }
                    impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
                }
                else {
                    impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
                }
            }
            else {
                // Keep the drawn card
                hand.push_back(drawn);
                impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
            }
        }
        else {
            //Deck empty then just skip turn
            impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
        }
    }
}

string UNOGame::getState() const {
    UNOImpl* impl = getImpl(this);
    ostringstream out;
    string dir = impl->clockwise ? "Clockwise" : "Counter-clockwise";
    const CardImpl& top = impl->discard.back();

    out << "Player " << impl->currentPlayer
        << "'s turn, Direction: " << dir
        << ", Top: " << top.color << " " << top.value
        << ", Players cards: ";

    for (int p = 0; p < impl->numPlayers; p++) {
        out << "P" << p << ":" << impl->hands[p].size();
        if (p < impl->numPlayers - 1) out << ", ";
    }
    return out.str();
}
