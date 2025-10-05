#include "polynomial.h"
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <limits>
#include <cmath>

using namespace std;

//Node class for linked list
class Node {
public:
    int coef;
    int exp;
    Node* next;
    Node(int c, int e) : coef(c), exp(e), next(nullptr) {}
};

//Global map to store each polynomial's head pointer
static map<const Polynomial*, Node*> polynomialData;

//Get head pointer for a polynomial
static Node*& getHead(const Polynomial* p) {
    return polynomialData[p];
}

//Deep copy linked list
static Node* copyList(Node* head) {
    if (!head) return nullptr;
    Node* newHead = new Node(head->coef, head->exp);
    Node* currNew = newHead;
    Node* currOld = head->next;
    while (currOld) {
        currNew->next = new Node(currOld->coef, currOld->exp);
        currNew = currNew->next;
        currOld = currOld->next;
    }
    return newHead;
}

//Delete linked list
static void deleteList(Node*& head) {
    while (head) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
}

//Insert node in descending order and combine like terms
static void insertNode(Node*& head, int coef, int exp) {
    if (coef == 0) return;

    //Check overflow
    if (abs((long long)coef) > numeric_limits<int>::max()) {
        throw overflow_error("Coefficient overflow");
    }

    Node* prev = nullptr;
    Node* curr = head;

    while (curr && curr->exp > exp) {
        prev = curr;
        curr = curr->next;
    }

    if (curr && curr->exp == exp) {
        long long newCoef = (long long)curr->coef + coef;
        if (newCoef > numeric_limits<int>::max() || newCoef < numeric_limits<int>::min()) {
            throw overflow_error("Coefficient overflow");
        }
        curr->coef = (int)newCoef;
        if (curr->coef == 0) { //remove node
            if (prev) prev->next = curr->next;
            else head = curr->next;
            delete curr;
        }
    }
    else {
        Node* newNode = new Node(coef, exp);
        if (prev) {
            newNode->next = prev->next;
            prev->next = newNode;
        }
        else {
            newNode->next = head;
            head = newNode;
        }
    }
}

//Insert term into polynomial
void Polynomial::insertTerm(int coefficient, int exponent) {
    if (exponent < 0) throw invalid_argument("Exponent must be non-negative");
    if (coefficient == 0) return;
    insertNode(getHead(this), coefficient, exponent);
}

//Convert polynomial to string
string Polynomial::toString() const {
    Node* head = getHead(this);
    if (!head) return "0";

    ostringstream oss;
    Node* curr = head;
    bool first = true;

    while (curr) {
        if (!first) {
            oss << (curr->coef > 0 ? " + " : " - ");
        }
        else if (curr->coef < 0) {
            oss << "-";
        }

        int absCoef = abs(curr->coef);
        if (absCoef != 1 || curr->exp == 0) oss << absCoef;
        if (curr->exp > 0) oss << "x";
        if (curr->exp > 1) oss << "^" << curr->exp;

        first = false;
        curr = curr->next;
    }
    return oss.str();
}

//Add two polynomials
Polynomial Polynomial::add(const Polynomial& other) const {
    Polynomial result;
    Node* head1 = getHead(this);
    Node* head2 = getHead(&other);

    Node* curr = head1;
    while (curr) {
        insertNode(getHead(&result), curr->coef, curr->exp);
        curr = curr->next;
    }

    curr = head2;
    while (curr) {
        insertNode(getHead(&result), curr->coef, curr->exp);
        curr = curr->next;
    }

    return result;
}

//Multiply two polynomials
Polynomial Polynomial::multiply(const Polynomial& other) const {
    Polynomial result;
    Node* head1 = getHead(this);
    Node* head2 = getHead(&other);

    for (Node* n1 = head1; n1; n1 = n1->next) {
        for (Node* n2 = head2; n2; n2 = n2->next) {
            long long prod = (long long)n1->coef * n2->coef;
            if (prod > numeric_limits<int>::max() || prod < numeric_limits<int>::min()) {
                throw overflow_error("Coefficient overflow in multiplication");
            }
            insertNode(getHead(&result), (int)prod, n1->exp + n2->exp);
        }
    }

    return result;
}

//Derivative of polynomial
Polynomial Polynomial::derivative() const {
    Polynomial result;
    Node* head = getHead(this);
    for (Node* curr = head; curr; curr = curr->next) {
        if (curr->exp != 0) {
            long long prod = (long long)curr->coef * curr->exp;
            if (prod > numeric_limits<int>::max() || prod < numeric_limits<int>::min()) {
                throw overflow_error("Coefficient overflow in derivative");
            }
            insertNode(getHead(&result), (int)prod, curr->exp - 1);
        }
    }
    return result;
}

//Cleanup global map
void cleanupPolynomials() {
    for (auto& kv : polynomialData) {
        deleteList(kv.second);
    }
    polynomialData.clear();
}
