# VaNiX
### Snake Game - Varunee (202512067) , Nishka (202512035)

# 🐍 Snake Game - Cross-Platform

A modern, colorful, and interactive **terminal-based Snake Game** built using **C++**.  
Designed to run seamlessly on both **Windows** and **Linux**, it demonstrates cross-platform programming, terminal manipulation, and object-oriented design principles.

---

## 📑 Table of Contents

1. [Overview](#-overview)  
2. [Features](#-features)  
3. [System Architecture](#-system-architecture)  
4. [Technical Implementation](#-technical-implementation)  
5. [Installation & Compilation](#-installation--compilation)  
6. [Usage](#-usage)  
7. [Design Patterns & Best Practices](#-design-patterns--best-practices)  
8. [Performance Considerations](#-performance-considerations)  
9. [Future Enhancements](#-future-enhancements)  
10. [Team Contributors](#-team-contributors)  

---

## 🧩 Overview

The **Snake Game** is a classic arcade-style project recreated in **C++** using only **standard libraries** and **terminal control techniques**.  
It showcases real-time keyboard input, smooth movement, dynamic rendering, and persistent high-score storage.

This project was developed as part of an **Introduction to Programming (ITP)** course to demonstrate proficiency in:
- Structured and Object-Oriented Programming (OOP)
- File handling
- Platform abstraction
- Interactive console applications

---

## ✨ Features

| Feature | Description |
|----------|--------------|
| 🎮 **Cross-Platform** | Compatible with both **Windows** and **Linux** environments. |
| 🌈 **Colorful UI** | Enhanced visuals using **ANSI escape codes** and emojis. |
| 💾 **High Score System** | Saves and loads scores automatically via file I/O. |
| ⚡ **Real-Time Input** | Non-blocking input handling ensures smooth gameplay. |
| 🧱 **Dynamic Walls** | Detects and ends the game on wall collisions. |
| 🧠 **Self-Collision Detection** | Ends the game if the snake runs into itself. |
| 🔄 **Restart & Exit Menu** | Easy navigation after game over. |
| 🧍 **User-Friendly Interface** | Simple control scheme using both arrow keys and WASD. |

---

## 🏗️ System Architecture

The system is divided into modular components for clarity and maintainability:

```
+------------------------------------------------------+
|                    Game (Main)                      |
|  - Controls flow and game loop                      |
|  - Manages rendering, updates, and user input       |
+------------------------------------------------------+
             |                 |                  |
             v                 v                  v
     +-------------+    +--------------+   +----------------+
     |   Snake     |    |     Food     |   | HighScoreManager |
     +-------------+    +--------------+   +----------------+
     | Body logic  |    | Food spawn   |   | File I/O logic |
     | Movement    |    | Position gen |   | Save/load data |
     +-------------+    +--------------+   +----------------+
```

### Class Overview:
- **`Snake`** → Handles movement, direction, growth, and self-collision.  
- **`Food`** → Randomly spawns food at positions not occupied by the snake.  
- **`HighScoreManager`** → Manages reading/writing the high score file.  
- **`Game`** → Coordinates gameplay, input, rendering, and end conditions.

---

## ⚙️ Technical Implementation

- **Language:** C++ (C++11 and above)  
- **Libraries Used:**
  - `<deque>` – for snake body storage
  - `<fstream>` – for file I/O (high score system)
  - `<termios.h>` and `<ioctl.h>` – for non-blocking input on Linux
  - `<conio.h>` and `<windows.h>` – for Windows compatibility
  - ANSI escape codes for color and cursor control

- **Game Loop Structure:**
  ```cpp
  while (!game.over()) {
      game.input();
      game.update();
      game.draw();
      Sleep(game.speed());
  }
  ```

- **Cross-Platform Input Handling:**  
  Separate implementations for `_WIN32` and POSIX systems using preprocessor directives.

---

## 🧰 Installation & Compilation

### 🪟 On Windows:
1. Open **Command Prompt** or **PowerShell**.  
2. Compile using:
   ```bash
   g++ main.cpp -o snake
   ```
3. Run the game:
   ```bash
   snake.exe
   ```

### 🐧 On Linux / macOS:
1. Open **Terminal**.  
2. Compile using:
   ```bash
   g++ main.cpp -o snake
   ```
3. Run the game:
   ```bash
   ./snake
   ```

> 💡 Note: The program automatically handles raw terminal mode and restores it safely upon exit.

---

## 🎮 Usage

### 🎯 Objective:
Eat as many apples 🍎 as possible without hitting the wall or your own body.  
Each apple increases your score and makes the snake grow longer.

### 🕹️ Controls:

| Key | Action |
|:---:|:--------|
| **W / ↑** | Move Up |
| **S / ↓** | Move Down |
| **A / ←** | Move Left |
| **D / →** | Move Right |
| **ESC** | Exit Game |
| **R** | Restart after Game Over |
| **Q** | Quit Game |

---

## 🧠 Design Patterns & Best Practices

- **Object-Oriented Design (OOP):**  
  Clear separation between data (state) and behavior (methods) across Snake, Food, and Game classes.

- **Encapsulation:**  
  Class members are managed through well-defined interfaces.

- **Single Responsibility Principle:**  
  Each class performs one distinct task (e.g., `Snake` handles movement only).

- **Platform Abstraction:**  
  `#ifdef _WIN32` blocks separate Windows-specific and Linux-specific code, ensuring maintainability.

- **File Handling Best Practices:**  
  Safe opening, reading, and writing to maintain data persistence for high scores.

---

## 🚀 Performance Considerations

- **Non-blocking I/O:**  
  Real-time responsiveness through minimal CPU waiting using `kbhit()` and `read()` timeouts.  

- **Optimized Rendering:**  
  Uses ANSI cursor repositioning (`\033[H`) to redraw efficiently instead of clearing the screen entirely.

- **Memory Efficiency:**  
  `std::deque` allows fast push/pop operations for snake body movement.

---

## 🔮 Future Enhancements

Planned improvements for future versions:

- 🏁 Multiple difficulty levels (speed adjustments)
- 🧱 Obstacles and barrier mechanics  
- ⏸️ Pause and resume functionality  
- 🧾 Scoreboard with player names  
- 🌍 Border wrapping or maze modes  

---

## 👨‍💻 Team Contributors

- **Varunee (202512067)**
- **Nishka (202512035)**

> 🎓 *Project developed as part of the IT603 – Introduction to Programming (ITP) course, 2025.*

## 👨‍🏫 Under the Guidance of

**Faculty:** Prof. Ankush Chander  
**Teaching Assistant (TA):** Monson Verghese, Devansh Shah 
*MSc IT Department, Dhirubhai Ambani University (DAU)*

---



