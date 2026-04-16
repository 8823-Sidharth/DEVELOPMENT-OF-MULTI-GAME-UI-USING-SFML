# 🎮 Retro Arcade Gaming Platform (SFML - C++)

A retro-style multi-game arcade platform built using **C++ and SFML**, featuring classic games like **Snake**, **Tetris**, and **Breakout (Pickle Game)** with a modern UI and smooth gameplay.

---

## 🚀 Features

* 🎯 Multiple games in a single application
* 🧠 Scene-based architecture (Menu, Snake, Tetris, Breakout)
* 🎮 Smooth gameplay using **delta time (frame-independent movement)**
* 🧩 Dynamic difficulty system
* ⚡ Power-ups system (Breakout)
* 🎨 Retro UI with animations and hover effects
* 🛡️ Memory-safe implementation using `std::unique_ptr`

---

## 🕹️ Games Included

### 🐍 Snake

* Dynamic obstacle patterns (shapes, alphabets, numbers)
* Difficulty levels (Easy, Medium, Hard)
* Safe spawning system
* Score & level progression

---

### 🧱 Tetris

* Random piece generation
* Rotation using pivot transformation
* Line clearing with correct shifting logic
* Increasing difficulty with levels

---

### 🧩 Breakout (Pickle Game)

* Paddle and ball physics
* Brick destruction system
* Power-ups:

  * Expand paddle
  * Double score
  * Speed boost
  * Extra ball

---

## 🏗️ Project Structure

```
ProjectC/
│
├── src/
│   ├── App.cpp
│   ├── MenuScene.cpp
│   ├── SnakeScene.cpp
│   ├── TetrisScene.cpp
│   ├── PickleScene.cpp
│
├── assets/
│   ├── fonts/
│   ├── images/
│   └── sounds/
│
├── build/
│
└── README.md
```

---

## ⚙️ Technologies Used

* **C++**
* **SFML (Simple and Fast Multimedia Library)**
* Object-Oriented Programming (OOP)
* Game Development Concepts

---

## 🔁 Game Flow

1. Application starts → MenuScene loads
2. User selects a game
3. Scene switches dynamically
4. Game loop runs:

   * Handle Input (`handleEvent`)
   * Update Logic (`update`)
   * Render Frame (`render`)

---

## 🧠 Key Concepts Implemented

* Scene Management System
* Game Loop Architecture
* Collision Detection
* Delta Time (frame-independent movement)
* Smart Pointers (`std::unique_ptr`)
* Event Handling (keyboard & mouse)

---

## ▶️ How to Run

### 🔧 Requirements

* C++ Compiler (MinGW / GCC)
* SFML Library installed

---

### 🛠️ Compile & Run

```bash
g++ src/*.cpp -IC:/SFML/include -LC:/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system -o build/game.exe
./build/game.exe
```

---

## 🎯 Controls

### General

* `ESC` → Exit
* `B` → Back to Menu

### Snake

* `WASD / Arrow Keys` → Move
* `P` → Pause

### Tetris

* `← →` → Move
* `↑` → Rotate
* `↓` → Soft Drop
* `Space` → Hard Drop

### Breakout

* `← →` → Move Paddle
* `Space` → Launch Ball

---

## 📌 Highlights

* Clean modular design using scenes
* Real-time animations and UI effects
* Optimized performance using frame limiting and delta time
* Beginner-friendly yet scalable architecture

---


