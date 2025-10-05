# DSA Assignment 1 – Zayna Qasim

## Introduction
This repository contains solutions to three DSA assignment problems implemented in C++ using object-oriented programming.  
Each problem is modular (`.cpp` and `.h` files) and hides internal implementation details.

---

## Approach

### Polynomial ADT
- Implemented using **linked-list–style logic** with helper functions in `.cpp`.  
- Supports: **insert**, **print**, **addition**, **multiplication**, and **differentiation**.  
- Combines like terms and automatically removes zero coefficients.  
- Handles negative values and integer overflow safely.

### Text Editor Simulation
- Cursor-based editor using **two stacks** (left/right of cursor).  
- Supports character **insertion**, **deletion**, and **cursor movement**.  
- Rebuilds text with cursor for display efficiently.  
- Prevents invalid operations (non-printable characters, moving beyond text boundaries).

### UNO Card Game Simulation
- Supports **2–4 players** with a **76-card deck** (standard UNO minus wild cards).  
- Uses **linked lists** for deck/discard pile and **stacks** for player hands.  
- Implements action cards (**Skip**, **Reverse**, **Draw Two**) with correct priority and turn logic.  
- Shuffles deck with **fixed seed**, deals 7 cards per player, continues until a player wins or the deck is empty.

---

## Challenges Faced

### 1. Encapsulation & Header Constraints
- All `.h` files were **unchangeable**, preventing adding member variables.  
- Solved using **helper functions** and internal storage structures (maps, linked lists, stacks) in `.cpp`.  
- Ensured users could not access internal data directly, preserving abstraction and clean interfaces.

### 2. Polynomial ADT
- **Duplicate exponents:** careful merging during insertion, addition, and multiplication.  
- **Zero coefficients:** terms removed after operations to maintain valid polynomials.  
- **Negative coefficients and exponents:** handled in all operations including differentiation.  
- **Integer overflow:** safe checks in multiplication and differentiation to prevent runtime errors.

### 3. Text Editor Simulation
- **Cursor management:** insertion, deletion, and movement tracked consistently in left/right stacks.  
- **Boundary conditions:** prevented cursor from moving past start or end of text.  
- **Invalid operations:** safe handling of deletion at start and non-printable ASCII insertion.  
- **Display:** efficiently reconstructing text with cursor while maintaining correct order.

### 4. UNO Card Game Simulation
- **Action card logic:** Skip, Reverse, Draw Two handled with correct priority.  
- **2-player Reverse behavior:** special case reversing turn order correctly.  
- **Deck exhaustion & stalemate:** game ends gracefully if no cards left or no playable moves.  
- **Multiple playable cards:** rules determine which card to play first.  
- **Efficient card management:** linked lists for deck/discard pile and stacks for hands ensured fast operations.

### 5. Git/GitHub and Environment
- **Cleaning repository:** removed unnecessary build files, cache, and Visual Studio folders.  
- **.gitignore configuration:** prevented tracking of temporary/compiled files.

---

## GitHub Repository
[DSA_Assignment1_ZaynaQasim](https://github.com/zaynaqasim/DSA_Assignment1_ZaynaQasim.git)
