#include "texteditor.h"
#include <stack>
#include <string>
#include <map>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

/*
   - The header file doesn't allow us to add members to TextEditor.
   - To work around this, each TextEditor object is associated
     with an EditorImpl instance stored in a global map.

   EditorImpl:
     left  : stack of characters to the LEFT of the cursor
     right : stack of characters to the RIGHT of the cursor
*/
struct EditorImpl {
    stack<char> left;   //chars before cursor
    stack<char> right;  //chars after cursor
};

//Global storage: each TextEditor* to its private EditorImpl
static map<const TextEditor*, EditorImpl*> editorStorage;

/*
   Utility: get storage for an editor
   - If this editor has no storage yet, create a new one.
   - Always returns a valid pointer.
*/
static EditorImpl* getStorage(const TextEditor* ed) {
    if (editorStorage.find(ed) == editorStorage.end()) {
        editorStorage[ed] = new EditorImpl();
    }
    return editorStorage[ed];
}

   /*
      Insert a character at the cursor.
      - Only printable ASCII [32..126] is allowed.
      - Character goes onto the "left" stack (before cursor).
      - Example: text="ab|c", insert('x') → "abx|c"
   */
void TextEditor::insertChar(char c) {
    if (c < 32 || c > 126) {
        throw invalid_argument("Only printable ASCII characters allowed");
    }

    EditorImpl* impl = getStorage(this);
    impl->left.push(c);
}

/*
   Delete character immediately BEFORE the cursor (like backspace).
   - If cursor at position 0 (left empty), nothing happens.
   - Example: text="ab|c", delete → "a|c"
   - Example: text="|abc", delete → no change
*/
void TextEditor::deleteChar() {
    EditorImpl* impl = getStorage(this);
    if (!impl->left.empty()) {
        impl->left.pop();
    }
    //else: cursor at start, nothing to delete
}

/*
   Move cursor one position LEFT.
   - If already at leftmost (left stack empty), nothing happens.
   - Otherwise: pop from left → push onto right
   - Example: text="ab|c", moveLeft → "a|bc"
*/
void TextEditor::moveLeft() {
    EditorImpl* impl = getStorage(this);
    if (!impl->left.empty()) {
        impl->right.push(impl->left.top());
        impl->left.pop();
    }
    //else: cursor already at start
}

/*
   Move cursor one position RIGHT.
   - If already at rightmost (right stack empty), nothing happens.
   - Otherwise: pop from right → push onto left
   - Example: text="a|bc", moveRight → "ab|c"
*/
void TextEditor::moveRight() {
    EditorImpl* impl = getStorage(this);
    if (!impl->right.empty()) {
        impl->left.push(impl->right.top());
        impl->right.pop();
    }
    //else: cursor already at end
}

/*
   Display full text with cursor position marked by '|'.
   - left stack: characters before cursor
   - right stack: characters after cursor
   - We rebuild strings from the stacks carefully.

   Example:
     left = ['a','b']   (top='b')
     right = ['c','d']  (top='c')
     → "ab|cd"
*/
string TextEditor::getTextWithCursor() const {
    EditorImpl* impl = getStorage(this);

    //Build string from left stack (reverse to get correct order)
    string leftStr;
    stack<char> temp = impl->left;
    while (!temp.empty()) {
        leftStr.push_back(temp.top());
        temp.pop();
    }
    reverse(leftStr.begin(), leftStr.end());

    //Build string from right stack (top is first char after cursor,
    //so no reverse needed here)
    string rightStr;
    temp = impl->right;
    while (!temp.empty()) {
        rightStr.push_back(temp.top());
        temp.pop();
    }

    return leftStr + "|" + rightStr;
}
