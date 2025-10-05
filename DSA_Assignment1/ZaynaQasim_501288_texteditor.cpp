#include "texteditor.h"
#include <stack>
#include <map>
#include <string>
#include <stdexcept>
#include <algorithm>

using namespace std;

//Node-based storage using stacks for left/right of cursor
struct EditorImpl {
    stack<char> left;  //characters before cursor
    stack<char> right; //characters after cursor
};

//Global map to link each TextEditor object to its storage
static map<const TextEditor*, EditorImpl*> editorStorage;

//Get storage for an editor
static EditorImpl* getStorage(const TextEditor* ed) {
    if (!editorStorage[ed]) editorStorage[ed] = new EditorImpl();
    return editorStorage[ed];
}

//Insert printable ASCII character at cursor
void TextEditor::insertChar(char c) {
    if (c < 32 || c > 126) throw invalid_argument("Only printable ASCII allowed");
    getStorage(this)->left.push(c);
}

//Delete character before cursor
void TextEditor::deleteChar() {
    stack<char>& l = getStorage(this)->left;
    if (!l.empty()) l.pop();
}

//Move cursor left
void TextEditor::moveLeft() {
    EditorImpl* impl = getStorage(this);
    if (!impl->left.empty()) {
        impl->right.push(impl->left.top());
        impl->left.pop();
    }
}

//Move cursor right
void TextEditor::moveRight() {
    EditorImpl* impl = getStorage(this);
    if (!impl->right.empty()) {
        impl->left.push(impl->right.top());
        impl->right.pop();
    }
}

//Get text with cursor position '|'
string TextEditor::getTextWithCursor() const {
    EditorImpl* impl = getStorage(this);

    //Build left string in correct order
    string leftStr;
    stack<char> temp = impl->left;
    while (!temp.empty()) { leftStr.push_back(temp.top()); temp.pop(); }
    reverse(leftStr.begin(), leftStr.end());

    //Build right string (top is first character after cursor)
    string rightStr;
    temp = impl->right;
    while (!temp.empty()) { rightStr.push_back(temp.top()); temp.pop(); }

    return leftStr + "|" + rightStr;
}
