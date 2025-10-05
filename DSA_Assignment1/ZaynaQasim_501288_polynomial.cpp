#include "polynomial.h"
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <limits>

using namespace std;

/*
   The header file does not allow us to store members directly.
   To work around this, we use a hidden implementation (PolyImpl)
   managed through a static map that links each Polynomial object
   to its internal storage.

   - Storage: map<int, int> terms   (exponent to coefficient)
   - Advantages: easy to merge like terms, sorted order guaranteed
   - Hidden: users of Polynomial never see this structure
*/
struct PolyImpl {
    map<int, int> terms; //each key=exponent, value=coefficient
};

//Global map: each Polynomial* to its private PolyImpl
static map<const Polynomial*, PolyImpl*> polyStorage;

//Utility: safely get storage for a polynomial
static PolyImpl* getStorage(const Polynomial* p) {
    if (polyStorage.find(p) == polyStorage.end()) {
        polyStorage[p] = new PolyImpl();
    }
    return polyStorage[p];
}

/*
   Insert a term into the polynomial.
   - Exponent must be non-negative
   - Coefficient 0 is ignored
   - If exponent already exists, coefficients are added (like terms combined)
   - If result becomes 0, term is removed
*/
void Polynomial::insertTerm(int coefficient, int exponent) {
    if (exponent < 0)
        throw invalid_argument("Exponent must be non-negative");

    if (coefficient == 0)
        return; //skip useless terms

    PolyImpl* impl = getStorage(this);

    //Check for integer overflow (safe-ish guard)
    long long newVal = (long long)impl->terms[exponent] + coefficient;
    if (newVal > numeric_limits<int>::max() || newVal < numeric_limits<int>::min()) {
        throw overflow_error("Coefficient overflow detected");
    }

    impl->terms[exponent] = (int)newVal;

    //Remove if coefficient becomes 0
    if (impl->terms[exponent] == 0) {
        impl->terms.erase(exponent);
    }
}

/*
   Convert polynomial to a human-readable string.
   - Example: 3x^4 + 2x^2 - x + 5
   - If empty, returns "0"
*/
string Polynomial::toString() const {
    PolyImpl* impl = getStorage(this);
    if (impl->terms.empty()) return "0";

    ostringstream oss;
    bool first = true;

    //Iterate from largest exponent downwards
    for (auto it = impl->terms.rbegin(); it != impl->terms.rend(); ++it) {
        int exp = it->first;
        int coeff = it->second;

        if (coeff == 0) continue; //just in case

        //Add sign
        if (!first) {
            oss << (coeff > 0 ? " + " : " - ");
        }
        else {
            if (coeff < 0) oss << "-";
        }

        int absCoeff = abs(coeff);

        //Print coefficient
        if (exp == 0) {
            oss << absCoeff; //constant term
        }
        else {
            if (absCoeff != 1) oss << absCoeff;// hide '1x'
            oss << "x";
            if (exp != 1) oss << "^" << exp;
        }

        first = false;
    }

    return oss.str();
}

/*
   Add two polynomials and return the result.
   - Does not modify originals
   - Like terms combined
*/
Polynomial Polynomial::add(const Polynomial& other) const {
    Polynomial result;
    PolyImpl* rimpl = getStorage(&result);
    PolyImpl* a = getStorage(this);
    PolyImpl* b = getStorage(&other);

    for (auto& kv : a->terms) {
        rimpl->terms[kv.first] += kv.second;
    }
    for (auto& kv : b->terms) {
        rimpl->terms[kv.first] += kv.second;
    }

    //Remove any zero terms
    for (auto it = rimpl->terms.begin(); it != rimpl->terms.end();) {
        if (it->second == 0) it = rimpl->terms.erase(it);
        else ++it;
    }

    return result;
}

/*
   Multiply two polynomials.
   - (a_n x^n) * (b_m x^m) = (a_n * b_m) x^(n+m)
   - All pairs combined
   - Like terms merged
*/
Polynomial Polynomial::multiply(const Polynomial& other) const {
    Polynomial result;
    PolyImpl* rimpl = getStorage(&result);
    PolyImpl* a = getStorage(this);
    PolyImpl* b = getStorage(&other);

    for (auto& kv1 : a->terms) {
        for (auto& kv2 : b->terms) {
            int exp = kv1.first + kv2.first;
            long long coeffVal = (long long)kv1.second * kv2.second + rimpl->terms[exp];

            if (coeffVal > numeric_limits<int>::max() || coeffVal < numeric_limits<int>::min()) {
                throw overflow_error("Coefficient overflow in multiplication");
            }
            rimpl->terms[exp] = (int)coeffVal;
        }
    }

    //Clean up zero terms
    for (auto it = rimpl->terms.begin(); it != rimpl->terms.end();) {
        if (it->second == 0) it = rimpl->terms.erase(it);
        else ++it;
    }

    return result;
}

/*
   Differentiate polynomial (d/dx).
   - Rule: d/dx (a*x^n) = (a*n)*x^(n-1)
   - Constants vanish
*/
Polynomial Polynomial::derivative() const {
    Polynomial result;
    PolyImpl* rimpl = getStorage(&result);
    PolyImpl* impl = getStorage(this);

    for (auto& kv : impl->terms) {
        int exp = kv.first;
        int coeff = kv.second;

        if (exp > 0) {
            long long newCoeff = (long long)coeff * exp;
            if (newCoeff > numeric_limits<int>::max() || newCoeff < numeric_limits<int>::min()) {
                throw overflow_error("Coefficient overflow in derivative");
            }
            rimpl->terms[exp - 1] += (int)newCoeff;
        }
    }

    return result;
}
