# <img width="44" height="32" alt="icon" src="https://github.com/user-attachments/assets/6cd5b753-9a23-4d7a-ba2c-b50ac599b534" /> Rapid Engine


![RapidEngine stars](https://img.shields.io/github/stars/EmilDimov93/Rapid-Engine?style=plastic&label=⭐&color=FFD700)
![Total Commits](https://img.shields.io/github/commit-activity/t/EmilDimov93/Rapid-Engine?style=plastic)
![54 Nodes](https://img.shields.io/badge/Nodes-54-purple?style=plastic)
![C](https://img.shields.io/badge/language-C-555555?style=plastic)
![Raylib](https://img.shields.io/badge/Library-Raylib-ff69b4?style=plastic)

**Rapid Engine** is a game engine written in C, using the Raylib library, that includes a visual scripting language called **CoreGraph**. Designed for speed and full control with node-based logic.

<p align="center">
  <img width="610" height="368" alt="my-image" src="https://github.com/user-attachments/assets/a0e63453-6cea-45d2-8531-56069eba1c72" />
</p>

## ⚡ Core Features

#### 💡 **CoreGraph**: Node-based scripting language

- Real-time interaction and graph editing
- ***54*** node types covering variables, arithmetic, logic, conditionals, loops, and sprite manipulation
- ***22*** pin types: flow, dropdown menu, text box and more
- Select, delete, copy, and paste nodes

#### 💻 Interpreter

- Build and Run CoreGraph projects instantly
- Optimized for smooth performance at high FPS

#### 🖼️ Custom UI with the Raylib library

- File management in bottom menu
- Real-time log for errors and debug messages
- Variables panel showing live values
- Top bar for window managment
- Settings menu with an option to save preferences

#### ✂️ Hitbox editor

- Visual polygon hitbox creation for sprites
- Add vertices easily with a click

#### 🧷 Text editor

- Edit any text or code file
- Cut, Copy & Paste functions
- Save or open files in your default editor

#### 📚 Project Manager

- Create & load projects

---

## 🧩 All node types:

| Category   | Node Type                 |
|------------|--------------------------|
| Variable   | Create number, string, bool, color |
| Variable   | Cast to number, string, bool, color |
| Event      | Event Start, Tick, On Button |
| Get        | Variable, Screen Width, Screen Height, Mouse Position, Random Number, Sprite Position |
| Set        | Variable, Background, FPS |
| Flow       | Branch, Loop, Flip Flop, Break, Sequence |
| Sprite     | Create, Set Position, Set Rotation, Set Texture, Set Size, Spawn, Destroy, Force |
| Prop       | Draw Rectangle, Draw Circle |
| Logical    | Comparison, Gate, Arithmetic, Clamp, Lerp, Sin, Cos |
| Debug      | Print to Log, Draw Line, Comment |
| Literal    | Number, String, Bool, Color |
| Camera     | Move, Zoom, Get Center, Shake |
| Sound      | Play |


## ⚠️ Build Instructions

```
git clone --recurse-submodules https://github.com/EmilDimov93/Rapid-Engine.git
cd Rapid-Engine
mkdir build; cd build
```

**Windows**
```
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc ..
cmake --build .
```

### Linux / macOS
```
cmake -DCMAKE_C_COMPILER=gcc ..
cmake --build .
```

## 📧 Support

For assistance, contact [support@rapidengine.eu](mailto:support@rapidengine.eu)
