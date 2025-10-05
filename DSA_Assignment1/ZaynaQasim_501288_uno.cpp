#include "uno.h"
#include <stack>
#include <vector>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;

//Internal card representation
struct CardImpl {
    string color;  //"Red", "Green", "Blue", "Yellow"
    string value;  //"0"-"9", "Skip", "Reverse", "Draw Two"
};

//Node for linked list
struct CardNode {
    CardImpl card;
    CardNode* next;
    CardNode(const string& c, const string& v) : card({ c, v }), next(nullptr) {}
};

//Linked list for deck and player hands
class CardList {
public:
    CardNode* head;
    CardList() : head(nullptr) {}

    bool empty() const { return head == nullptr; }

    void push_front(CardNode* node) {
        node->next = head;
        head = node;
    }

    void push_back(CardNode* node) {
        if (!head) { head = node; node->next = nullptr; return; }
        CardNode* curr = head;
        while (curr->next) curr = curr->next;
        curr->next = node;
        node->next = nullptr;
    }

    CardNode* pop_front() {
        if (!head) return nullptr;
        CardNode* node = head;
        head = head->next;
        node->next = nullptr;
        return node;
    }

    int size() const {
        int count = 0;
        CardNode* curr = head;
        while (curr) { count++; curr = curr->next; }
        return count;
    }
};

//Hidden game state
struct UNOImpl {
    int numPlayers;
    int currentPlayer;
    bool clockwise;
    CardList deck;                  //draw pile
    stack<CardNode*> discard;       //discard pile
    vector<CardList> hands;         //each player's hand
};

//Global storage for UNOGame* with UNOImpl*
static map<const UNOGame*, UNOImpl*> gameStorage;

//Retrieve internal storage safely
static UNOImpl* getImpl(const UNOGame* g) {
    if (gameStorage.find(g) == gameStorage.end())
        throw runtime_error("UNOGame not initialized properly.");
    return gameStorage[g];
}

//Constructor
UNOGame::UNOGame(int numPlayers) {
    if (numPlayers < 2 || numPlayers > 4)
        throw invalid_argument("Players must be between 2 and 4");

    UNOImpl* impl = new UNOImpl();
    impl->numPlayers = numPlayers;
    impl->currentPlayer = 0;
    impl->clockwise = true;
    impl->hands.resize(numPlayers);

    gameStorage[this] = impl;
}

//Initialize game: build, shuffle, and deal deck
void UNOGame::initialize() {
    UNOImpl* impl = getImpl(this);

    //Clear previous state
    for (auto& h : impl->hands) h.head = nullptr;
    while (!impl->discard.empty()) impl->discard.pop();

    //Build deck
    vector<string> colors = { "Red", "Green", "Blue", "Yellow" };
    vector<string> values = { "0","1","2","3","4","5","6","7","8","9","Skip","Reverse","Draw Two" };

    vector<CardNode*> tempDeck;

    for (auto& c : colors) {
        tempDeck.push_back(new CardNode(c, "0")); //one zero
        for (int rep = 0; rep < 2; rep++) {
            for (size_t i = 1; i < values.size(); i++) {
                tempDeck.push_back(new CardNode(c, values[i]));
            }
        }
    }

    //Shuffle deck with fixed seed
    mt19937 rng(1234);
    shuffle(tempDeck.begin(), tempDeck.end(), rng);

    //Add to linked list deck
    for (auto node : tempDeck) impl->deck.push_back(node);

    //Deal 7 cards to each player
    for (int i = 0; i < 7; i++) {
        for (int p = 0; p < impl->numPlayers; p++) {
            CardNode* drawn = impl->deck.pop_front();
            if (drawn) impl->hands[p].push_back(drawn);
        }
    }

    //Place first non-action card on discard
    while (!impl->deck.empty()) {
        CardNode* starter = impl->deck.pop_front();
        if (starter->card.value != "Skip" &&
            starter->card.value != "Reverse" &&
            starter->card.value != "Draw Two") {
            impl->discard.push(starter);
            break;
        }
        else {
            impl->deck.push_back(starter); //put back action card
        }
    }
}

//Check if any player has no cards
bool UNOGame::isGameOver() const {
    UNOImpl* impl = getImpl(this);
    for (auto& h : impl->hands) if (!h.head) return true;

    //Stalemate: deck empty and no playable cards
    if (impl->deck.empty()) {
        CardImpl top = impl->discard.top()->card;
        for (auto& h : impl->hands) {
            CardNode* curr = h.head;
            while (curr) {
                if (curr->card.color == top.color || curr->card.value == top.value)
                    return false; //playable card exists
                curr = curr->next;
            }
        }
        return true;
    }
    return false;
}

//Return index of winner
int UNOGame::getWinner() const {
    UNOImpl* impl = getImpl(this);
    for (int i = 0; i < impl->numPlayers; i++)
        if (!impl->hands[i].head) return i;
    return -1;
}

//Helper: can card be played
static bool canPlay(const CardImpl& c, const CardImpl& top) {
    return (c.color == top.color || c.value == top.value);
}

//Helper: next player index
static int nextPlayerIndex(const UNOImpl* impl, int start, int skipCount = 0) {
    int step = impl->clockwise ? 1 : -1;
    int idx = start;
    for (int i = 0; i <= skipCount; i++)
        idx = (idx + step + impl->numPlayers) % impl->numPlayers;
    return idx;
}

//Play a turn
void UNOGame::playTurn() {
    UNOImpl* impl = getImpl(this);
    if (isGameOver()) return;

    if (impl->discard.empty()) return;
    CardImpl top = impl->discard.top()->card;
    CardList& hand = impl->hands[impl->currentPlayer];

    CardNode* prev = nullptr;
    CardNode* curr = hand.head;
    CardNode* chosen = nullptr;

    //Priority 1: color match
    while (curr) {
        if (curr->card.color == top.color) { chosen = curr; break; }
        prev = curr; curr = curr->next;
    }

    //Priority 2: value match
    if (!chosen) {
        prev = nullptr; curr = hand.head;
        while (curr) {
            if (curr->card.value == top.value) { chosen = curr; break; }
            prev = curr; curr = curr->next;
        }
    }

    //Priority 3: action cards
    if (!chosen) {
        prev = nullptr; curr = hand.head;
        while (curr) {
            if ((curr->card.value == "Skip" || curr->card.value == "Reverse" || curr->card.value == "Draw Two")
                && canPlay(curr->card, top)) {
                chosen = curr; break;
            }
            prev = curr; curr = curr->next;
        }
    }

    if (chosen) {
        //Remove chosen from hand
        if (prev) prev->next = chosen->next;
        else hand.head = chosen->next;
        chosen->next = nullptr;
        impl->discard.push(chosen);

        //UNO alert
        if (hand.size() == 1)
            cout << "Player " << impl->currentPlayer << " says UNO!\n";

        //Apply action card effects
        if (chosen->card.value == "Skip") impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
        else if (chosen->card.value == "Reverse") {
            impl->clockwise = !impl->clockwise;
            impl->currentPlayer = (impl->numPlayers == 2) ? nextPlayerIndex(impl, impl->currentPlayer, 1) : nextPlayerIndex(impl, impl->currentPlayer, 0);
        }
        else if (chosen->card.value == "Draw Two") {
            int victim = nextPlayerIndex(impl, impl->currentPlayer, 0);
            for (int i = 0; i < 2; i++) {
                CardNode* drawn = impl->deck.pop_front();
                if (drawn) impl->hands[victim].push_back(drawn);
            }
            impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
        }
        else impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
    }
    else {
        //No playable card → draw one
        CardNode* drawn = impl->deck.pop_front();
        if (drawn) {
            if (canPlay(drawn->card, top)) {
                impl->discard.push(drawn);
                //Apply effects like above
                if (drawn->card.value == "Skip") impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
                else if (drawn->card.value == "Reverse") {
                    impl->clockwise = !impl->clockwise;
                    impl->currentPlayer = (impl->numPlayers == 2) ? nextPlayerIndex(impl, impl->currentPlayer, 1) : nextPlayerIndex(impl, impl->currentPlayer, 0);
                }
                else if (drawn->card.value == "Draw Two") {
                    int victim = nextPlayerIndex(impl, impl->currentPlayer, 0);
                    for (int i = 0; i < 2; i++) {
                        CardNode* add = impl->deck.pop_front();
                        if (add) impl->hands[victim].push_back(add);
                    }
                    impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 1);
                }
                else impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
            }
            else {
                hand.push_back(drawn);
                impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
            }
        }
        else impl->currentPlayer = nextPlayerIndex(impl, impl->currentPlayer, 0);
    }
}

//Get game state
string UNOGame::getState() const {
    UNOImpl* impl = getImpl(this);
    ostringstream out;
    string dir = impl->clockwise ? "Clockwise" : "Counter-clockwise";
    CardImpl top = impl->discard.top()->card;

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
