# BruhBruh

A multiplayer top-down shooter built in C++ using [raylib](https://www.raylib.com/), featuring a custom UDP networking layer, authoritative server architecture, and a modular client/server/shared codebase.

## Features

- **Multiplayer networking** — Custom UDP transport layer with a dedicated authoritative game server
- **Multiple characters** — Choose from a roster of characters (Tonts, Hodge, Raff, JJ), each with their own stats and bullet definitions
- **Combat system** — Projectile-based shooting with hitboxes, hurtboxes, health, death, and respawn logic
- **Dynamic walls** — Placeable destructible walls with health, damage, and destroy events synced across clients
- **Map system** — Text-based map format supporting static walls and player spawn points
- **HUD & UI** — In-game heads-up display and death screen
- **CI/CD** — GitHub Actions pipeline building on both Ubuntu and Windows on every push

## Architecture

The project is split into three modules:

- `src/server` — Authoritative game server: simulation tick loop, input handling, state broadcasting, and event publishing
- `src/client` — Game client: rendering, UI, camera, and player entity management via raylib
- `src/shared` — Code shared by both: player/bullet state, character definitions, map loading, collision components, and the network packet definitions
- `src/network` — Low-level UDP transport abstraction (`ITransport`, `ServerTransport`, `ClientTransport`)

## Building

**Prerequisites:**
- CMake 3.20+
- C++20-compatible compiler (GCC, Clang, or MSVC)
- On Linux, the following system libraries (installable via apt):

```bash
sudo apt-get install -y build-essential cmake libasound2-dev libx11-dev \
  libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev \
  libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

**Clone with submodules** (raylib is a submodule):

```bash
git clone --recurse-submodules https://github.com/your-username/BruhBruh.git
cd BruhBruh
```

**Configure and build:**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The compiled binary will be located at `build/BruhBruh` (Linux) or `build/Release/BruhBruh.exe` (Windows).

## Running

**Start the server:**

```bash
./BruhBruh --server
```

**Start a client** (connects to `127.0.0.1:54000` by default):

```bash
./BruhBruh --client
```

Running with no arguments defaults to client mode.

## Map Format

Maps are plain text files in `assets/maps/`. The supported directives are:

```
# Static wall:   WALL  x  y  width  height
# Spawn point:   SPAWN index  x  y

WALL  0    0    800  16     # top border
WALL  0    584  800  16     # bottom border
SPAWN 0    100  100
SPAWN 1    700  500
```

## Tech Stack

| Component | Technology |
|---|---|
| Language | C++20 |
| Graphics / Input | raylib |
| Build System | CMake |
| Networking | Custom UDP sockets (cross-platform) |
| CI | GitHub Actions (Ubuntu + Windows) |
