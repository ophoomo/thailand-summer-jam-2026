# Thailand Summer Jam 2026 - Custom Game Engine

A custom-built 2D game engine developed from scratch for **Thailand Summer Jam 2026**.

This project is created as a learning exercise to explore the fundamentals of building a game engine, including core systems such as:

- Rendering system
- Input handling
- Game loop architecture
- Basic engine structure

---

## 🛠️ Tech Stack

- Language: **C++**
- Build System: **CMake**
- Version Control: **Git (with submodules)**

---

## 📦 Getting Started

### 1. Clone the Repository (with submodules)

This project uses **git submodules**, so make sure to clone it properly:

```bash
git clone --recurse-submodules https://github.com/ophoomo/thailand-summer-jam-2026.git
```
```bash
cd thailand-summer-jam-2026
```
```bash
git submodule update --init --recursive
```

### 2. Build the Project

Make sure you have:
- CMake (>= 4.3.1)
- C++ compiler (GCC / Clang / MSVC)

Build steps
```bash
cmake -S . -B build
cmake --build build
```

### 2. RUN
After building, you can run the executable:

GCC / Clang (Linux / macOS)
```bash
./build/src/game/tsj2026
```

MSVC (Window)
```bash
.\build\src\game\Debug\tsj2026.exe
```

## 🎯 Goals of the Project
- Understand low-level game engine architecture
- Practice designing reusable systems
- Learn how rendering and input systems work internally
- Build everything from scratch (no game engine frameworks)

## 🙌 Acknowledgements
- [Thailand Summer Jam 2026](https://itch.io/jam/thailand-summer-jam-2026)
- [ThePillowy](https://thepillowy.itch.io/)
- [@xthebasicx](https://github.com/xthebasicx)
- lapis5537
- Open-source libraries used via submodules
