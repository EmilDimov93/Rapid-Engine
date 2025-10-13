# <img width="44" height="32" alt="icon" src="https://github.com/user-attachments/assets/6cd5b753-9a23-4d7a-ba2c-b50ac599b534" /> Rapid Engine


![RapidEngine stars](https://img.shields.io/github/stars/EmilDimov93/Rapid-Engine?style=plastic&label=⭐&color=FFD700)
![GitHub commits](https://img.shields.io/github/commit-activity/m/EmilDimov93/Rapid-Engine?style=plastic)
![Total Commits](https://img.shields.io/github/commit-activity/t/EmilDimov93/Rapid-Engine?style=plastic)
![GitHub last commit](https://img.shields.io/github/last-commit/EmilDimov93/Rapid-Engine?style=plastic)
![48 Nodes](https://img.shields.io/badge/Nodes-48-purple?style=plastic)
![C](https://img.shields.io/badge/language-C-555555?style=plastic)
![Raylib](https://img.shields.io/badge/Library-Raylib-ff69b4?style=plastic)

**Rapid Engine** is a game engine written in C, using the Raylib library, that includes a visual scripting language called **CoreGraph**. Designed for speed and full control with node-based logic.

<p align="center">
  <img width="610" height="368" alt="my-image" src="https://github.com/user-attachments/assets/a0e63453-6cea-45d2-8531-56069eba1c72" />
</p>


## ⚡ Core Features

- 💡 **CoreGraph**: Node-based scripting language
- 🖼️ Custom UI with the Raylib library
- 🎯 Real-time interaction and graph editing
- ⚙️ Basic language constructs such as variables, arithmetic, logic, conditionals, and loops
- 🎮 Spawning and moving sprites
- ✂️ Hitbox editor: Visual polygon hitbox creation
- 💾 Save, build and run systems

https://github.com/user-attachments/assets/b6dc9735-5eb8-498a-9802-c1eac80a9a4a


## 🔍 In-Depth Capabilities

- Interpreter for the CoreGraph language
- Viewport that shows either the CoreGraph Editor or the Game Screen
- File management system in bottom menu
- Real-time log for error and debug messages
- Variables menu for viewing information about the current variables and their values
- Different pin types: flow, dropdown, text box and more
- Custom window management top menu with settings


## 🧩 All node types:

| Category   | Type                    |
|------------|-------------------------|
| Variable   | Create number           |
| Variable   | Create string           |
| Variable   | Create bool             |
| Variable   | Create color            |
| Event      | Event Start             |
| Event      | Event Tick              |
| Event      | Event On Button         |
| Event      | Create Custom Event     |
| Event      | Call Custom Event       |
| Get        | Get variable            |
| Get        | Get Screen Width        |
| Get        | Get Screen Height       |
| Get        | Get Mouse Position      |
| Get        | Get Random Number       |
| Set        | Set variable            |
| Set        | Set Background          |
| Set        | Set FPS                 |
| Flow       | Branch                  |
| Flow       | Loop                    |
| Flow       | Delay                   |
| Flow       | Flip Flop               |
| Flow       | Break                   |
| Flow       | Return                  |
| Sprite     | Create sprite           |
| Sprite     | Set Sprite Position     |
| Sprite     | Set Sprite Rotation     |
| Sprite     | Set Sprite Texture      |
| Sprite     | Set Sprite Size         |
| Sprite     | Spawn sprite            |
| Sprite     | Destroy sprite          |
| Sprite     | Move To                 |
| Sprite     | Force                   |
| Prop       | Draw Prop Texture       |
| Prop       | Draw Prop Rectangle     |
| Prop       | Draw Prop Circle        |
| Logical    | Comparison              |
| Logical    | Gate                    |
| Logical    | Arithmetic              |
| Debug      | Print To Log            |
| Debug      | Draw Debug Line         |
| Literal    | Literal number          |
| Literal    | Literal string          |
| Literal    | Literal bool            |
| Literal    | Literal color           |
| Camera     | Move Camera             |
| Camera     | Zoom Camera             |
| Camera     | Get Camera Center       |
| Sound      | Play Sound              |


## 🧪 In Development

- Sprite collision events
- Text Editor
- Sprite sheet editor
- Exporting game
- Cross platform support
- New CoreGraph nodes


## ⚠️ Note: Rapid Engine is not packaged for public release yet, but you can build and run it manually:

```bash
gcc Engine/unity.c Engine/raylib/lib/libraylib.a -o ./RapidEngine -Iraylib/include -lopengl32 -lgdi32 -lwinmm -mwindows
```

```bash
./RapidEngine
```

## 📧 Support

For assistance, contact [support@rapidengine.eu](mailto:support@rapidengine.eu)
