# 2048 Game (WinAPI)

A lightweight, native Windows implementation of the classic 2048 puzzle game, written in **C++** using the **WinAPI** library. This project demonstrates how to build a functional, interactive desktop application with custom UI elements, animations, and game logic without relying on high-level game engines.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-0078d7.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B-00599c.svg)

## Features

- **Native Windows Performance**: Built directly on the Windows API for a small footprint and high responsiveness.
- **Classic Gameplay**: Experience the original 2048 mechanics where matching tiles merge into their sum.
- **Customizable Goals**: Feeling like a shorter game or a real challenge? Change your target goal (8, 16, 64, or 2048) directly through the **Goal** menu.
- **Visual Polish**: Includes smooth animations for tile creation and merging, plus a custom icon and theme.
- **Game State Management**: Automatically saves and loads your progress using a local `.ini` file, so you can pick up right where you left off.
- **Score Tracking**: Keep track of your progress as you merge tiles and aim for the high score.

## How to Play

### Controls
- **W**: Move Up
- **A**: Move Left
- **S**: Move Down
- **D**: Move Right
- **Ctrl + N**: Start a New Game

### Objective
Slide the tiles on the 4x4 grid. When two tiles with the same number touch, they **merge into one**! Reach the target tile number (default is 2048) to win the game.

## Building from Source

### Prerequisites
- **Microsoft Visual Studio** (2019 or later recommended)
- **C++ Desktop Development** workload installed

### Instructions
1. Clone the repository:
   ```bash
   git clone https://github.com/your-repo/2048-game.git
   ```
2. Open `2048.sln` in Visual Studio.
3. Set the build configuration to **Release** and your desired architecture (e.g., **x64** or **x86**).
4. Build the solution (**Ctrl + Shift + B**).
5. Run the application from the output directory or directly from Visual Studio (**F5**).

## Project Structure

- `2048/main.cpp`: The entry point for the application.
- `2048/app_2048.cpp/h`: Manages the main window, UI events, and GDI-based rendering.
- `2048/board.cpp/h`: Contains the core game logic, including tile movement, merging, and board state.
- `2048/Resource.rc`: Defines UI resources such as icons, menus, and keyboard accelerators.

## 📜 License

This project is open-source and available under the [MIT License](LICENSE).
