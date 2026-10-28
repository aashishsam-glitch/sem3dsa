# Undo and Redo Using Stack Implementation
> **Data Structures and Algorithms (DSA) Micro-Project**  
> **Core Engine:** C Language &bull; **Frontend:** HTML5, CSS3, Vanilla JavaScript &bull; **Protocol:** Lightweight HTTP / JSON

---

## 📋 Table of Contents
1. [Project Overview](#-project-overview)
2. [Why Stacks for Undo and Redo? (LIFO Principle)](#-why-stacks-for-undo-and-redo-lifo-principle)
3. [File Structure & Explanations](#-file-structure--explanations)
4. [How to Run the Project Locally](#-how-to-run-the-project-locally)
5. [Demonstration & Testing Guide (Requirement 6)](#-demonstration--testing-guide-requirement-6)
6. [Core C Implementation Details](#-core-c-implementation-details)
7. [Viva / Presentation Questions & Answers](#-viva--presentation-questions--answers)

---

## 🌟 Project Overview
This project is an interactive, educational web application designed to demonstrate the real-world application of the **Stack** data structure in implementing **Undo** and **Redo** operations (as used in text editors like Microsoft Word, VS Code, and Notepad).

Unlike simulated demos, this project uses an **actual native C backend**. The C program (`server.exe`) manages the two stacks (`undoStack` and `redoStack`) in memory and performs all `push`, `pop`, `edit`, `undo`, and `redo` operations. The web frontend communicates with the C backend via lightweight HTTP JSON requests.

---

## 🥞 Why Stacks for Undo and Redo? (LIFO Principle)

A **Stack** is a linear data structure that follows the **LIFO (Last In, First Out)** principle:
- The **last** element pushed onto the stack is the **first** one to be popped out.
- In text editing, the most recently made change is always the first change you want to revert when you press "Undo".

### The Dual-Stack Architecture
To support both backward travel (Undo) and forward travel (Redo), two stacks are required:

| Operation | Action on `undoStack` | Action on `redoStack` | New `currentText` |
| :--- | :--- | :--- | :--- |
| **Edit(newText)** | `push(currentText)` | `clearStack()` | `newText` |
| **Undo()** | `pop()` $\rightarrow$ restored | `push(currentText)` | Popped value from `undoStack` |
| **Redo()** | `push(currentText)` | `pop()` $\rightarrow$ restored | Popped value from `redoStack` |
| **Reset()** | `clearStack()` | `clearStack()` | `""` (Empty string) |
| **Display()** | *No change* | *No change* | Current text in memory |

> **Why is Redo cleared on a new Edit?**  
> When you undo a few steps and type something new, you create a *new timeline branch*. The previous "future" (Redo history) is no longer valid, so the Redo stack must be cleared.

---

## 📁 File Structure & Explanations

The project contains only essential files:

```
ia3dsa/
├── server.c      # Core C DSA stack logic & lightweight HTTP server (Winsock2)
├── index.html    # Interactive user interface with side-by-side stack visualizers
├── style.css     # Modern styling, responsive layout & push/pop animations
├── app.js        # Pure vanilla JavaScript API bridge and DOM updater
├── run.bat       # 1-click build & run script for Windows
└── README.md     # Project documentation, setup guide & viva reference
```

### File Details:
1. **`server.c`**:
   - Contains `struct Stack` with fixed-size array `items[MAX][MAX_TEXT_LEN]` (capacity: 30) and `top` pointer.
   - Implements `initialize()`, `isEmpty()`, `isFull()`, `push()`, `pop()`, and `clearStack()`.
   - Supports sliding window bounded history so real-time typing never overflows.
   - Embeds a minimal Winsock HTTP server on port `8080` that parses requests and returns JSON state.
2. **`index.html`**:
   - Text editor area and control buttons: **Apply Edit**, **Undo**, **Redo**, **Reset**, and **Display Current Text**.
   - Top-First visual dual-stack cylinders (newest elements appear right at the TOP with glowing `TOP ➔` badge).
   - Mode switcher for **Word-by-Word**, **Character-by-Character**, and **Manual Apply**.
   - Hands-free **Auto-Play Simulation Demo** buttons (Auto-Type Words, Auto-Type Chars, Auto-Undo, Auto-Redo).
3. **`style.css`**:
   - Clean, modern dark theme with distinct color palettes (Cyan for Undo, Purple for Redo).
   - Top-down LIFO layout with push-from-top and pop-to-top animations.
   - Fully responsive for mobile and desktop screens.
4. **`app.js`**:
   - Pure Vanilla JavaScript controller with live word/char auto-recording, Top-First rendering, and animated auto-playback.
5. **`run.bat`**:
   - Automatically searches for GCC on Windows.
   - Compiles `server.c` with `-lws2_32`.
   - Automatically launches `http://localhost:8080` in the browser and starts `server.exe`.

---

## 🚀 How to Run the Project Locally

### Option 1: One-Click Execution (Windows)
Double-click `run.bat` or run in terminal:
```cmd
.\run.bat
```
This script will:
1. Locate your GCC compiler (MinGW).
2. Compile `server.c` into `server.exe`.
3. Open `http://localhost:8080` in your default browser.
4. Start the server.

### Option 2: Manual Terminal Commands
If you prefer running commands manually:

1. **Compile with GCC:**
   ```bash
   gcc server.c -o server.exe -lws2_32
   ```
   *(Note for Linux/macOS users: `gcc server.c -o server`)*

2. **Run the executable:**
   ```bash
   .\server.exe
   ```

3. **Open in your browser:**
   Visit: **`http://localhost:8080`**

---

## 🧪 Demonstration & Testing Guide (Requirement 6)

You can run these steps manually or use the built-in **"College Presentation Demo Steps"** buttons on the website:

| Step | Action | Input / Button | Expected Result | Stack State |
| :---: | :--- | :--- | :--- | :--- |
| **1** | Enter initial text | Type `"Hello"` $\rightarrow$ Click **Edit / Apply** | Current text becomes `"Hello"`. Undo stack records previous blank state `""`. | `undoStack: [""]`<br>`redoStack: []` |
| **2** | Add more text | Type `"Hello World"` $\rightarrow$ Click **Edit / Apply** | Current text becomes `"Hello World"`. `"Hello"` is pushed onto `undoStack`. | `undoStack: ["", "Hello"]`<br>`redoStack: []` |
| **3** | Revert change | Click **Undo** | Restores `"Hello"`. `"Hello World"` is pushed onto `redoStack`. | `undoStack: [""]`<br>`redoStack: ["Hello World"]` |
| **4** | Re-apply change | Click **Redo** | Restores `"Hello World"`. `"Hello"` is pushed back onto `undoStack`. | `undoStack: ["", "Hello"]`<br>`redoStack: []` |
| **5** | Undo & branch | Click **Undo**, then enter `"Hi"` $\rightarrow$ Click **Edit** | Current text becomes `"Hi"`. **Redo stack is cleared!** | `undoStack: ["", "Hello"]`<br>`redoStack: []` |
| **6** | Verify Redo empty | Click **Redo** | Alert shown: *"Redo stack is empty! Nothing to redo."* | `redoStack` remains empty |
| **7** | Reset | Click **Reset All** | Current text is emptied and both stacks are cleared. | `undoStack: []`<br>`redoStack: []` |

---

## 💻 Core C Implementation Details

### 1. Data Structure Definition
```c
#define MAX 10
#define MAX_TEXT_LEN 512

typedef struct {
    char items[MAX][MAX_TEXT_LEN];
    int top;
} Stack;

Stack undoStack;
Stack redoStack;
char currentText[MAX_TEXT_LEN] = "";
```

### 2. Stack Operations
- **`initialize(Stack *s)`**: Sets `s->top = -1`.
- **`isEmpty(Stack *s)`**: Returns `1` if `s->top == -1`, otherwise `0`.
- **`isFull(Stack *s)`**: Returns `1` if `s->top == MAX - 1`, otherwise `0`.
- **`push(Stack *s, const char *text)`**: Increments `top` and copies the text string.
- **`pop(Stack *s, char *destination)`**: Copies the top string to `destination` and decrements `top`.
- **`clearStack(Stack *s)`**: Resets `s->top = -1`.

### 3. Complexity
- **Time Complexity:**
  - `push()`: $O(1)$ constant time.
  - `pop()`: $O(1)$ constant time.
  - `isEmpty()` / `isFull()`: $O(1)$ constant time.
- **Space Complexity:** $O(N \times L)$ where $N = 10$ items and $L = 512$ bytes per string.

---

## 🎓 Viva / Presentation Questions & Answers

**Q1: What is a Stack and what principle does it follow?**  
> *Answer:* A Stack is a linear data structure that follows the Last In, First Out (LIFO) principle, where insertions and deletions happen only at one end called the `top`.

**Q2: Why do we need two stacks instead of one for Undo/Redo?**  
> *Answer:* One stack (`undoStack`) can only store history backwards. When an undo occurs, that state is popped off. To allow the user to redo that action, the popped state must be saved into a second stack (`redoStack`).

**Q3: What happens when a user types new text after undoing?**  
> *Answer:* A new edit creates a new branch of document history. The previous future actions in `redoStack` become invalid, so `redoStack` is immediately cleared (`clearStack(&redoStack)`).

**Q4: How do you handle Stack Overflow and Underflow?**  
> *Answer:* 
> - Before `push()`, we check `isFull()`. If `top == MAX - 1`, we reject the operation to prevent buffer overflow.
> - Before `pop()`, we check `isEmpty()`. If `top == -1`, we return a message indicating the stack is empty to prevent accessing invalid array indices.

**Q5: How does the web browser communicate with the C backend?**  
> *Answer:* The C backend uses Windows Sockets (`winsock2.h`) to listen for HTTP requests on port 8080. When a user clicks a button, JavaScript sends a `fetch()` request (`POST /api/edit`, `/api/undo`, etc.). The C backend executes the stack operation and replies with a JSON response containing the updated stack state.
