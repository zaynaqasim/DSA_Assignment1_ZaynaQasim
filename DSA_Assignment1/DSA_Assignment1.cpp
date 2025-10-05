#include <iostream>
#include <string>
#include "polynomial.h"
#include "texteditor.h"
#include "uno.h"

using namespace std;

void testPolynomial() {
    cout << "\n=== Polynomial Test ===" << endl;

    Polynomial p1, p2;
    p1.insertTerm(3, 2);   // 3x^2
    p1.insertTerm(4, 1);   // +4x
    p2.insertTerm(5, 1);   // 5x

    cout << "P1: " << p1.toString() << endl;
    cout << "P2: " << p2.toString() << endl;

    Polynomial sum = p1.add(p2);
    cout << "Sum: " << sum.toString() << endl;

    Polynomial product = p1.multiply(p2);
    cout << "Product: " << product.toString() << endl;

    Polynomial derivative = p1.derivative();
    cout << "Derivative of P1: " << derivative.toString() << endl;
}

void testTextEditor() {
    cout << "\n=== Text Editor Test ===" << endl;

    TextEditor editor;

    cout << "Initial: " << editor.getTextWithCursor() << endl;

    editor.insertChar('H');
    editor.insertChar('e');
    editor.insertChar('l');
    editor.insertChar('l');
    editor.insertChar('o');
    cout << "After typing: " << editor.getTextWithCursor() << endl;

    editor.moveLeft();
    editor.moveLeft();
    editor.insertChar('X');
    cout << "After insert near cursor: " << editor.getTextWithCursor() << endl;

    editor.deleteChar();
    cout << "After delete: " << editor.getTextWithCursor() << endl;
}

void testUNOGame() {
    cout << "\n=== UNO Game Test ===" << endl;

    UNOGame game(2);   // 2 players
    game.initialize(); // fixed seed shuffle (1234)

    cout << game.getState() << endl;
    while (!game.isGameOver()) {
        game.playTurn();
        cout << game.getState() << endl;
    }

    cout << "Winner is Player " << game.getWinner() << "!" << endl;
}

int main() {
    testPolynomial();
    testTextEditor();
    testUNOGame();
    return 0;
}
