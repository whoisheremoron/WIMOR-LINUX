<p align="center">
  <img src="/res/logo.png" alt="wimor-linux">
  <a href="/LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="MIT License"></a>
  <a><img src="https://img.shields.io/badge/Platform-Linux-blue.svg" alt="Linux"></a>
  <a><img src="https://img.shields.io/badge/Game-CS:GO-blue.svg" alt="CS:GO"></a>
  <a><img src="https://img.shields.io/badge/State-Maintained-blue.svg" alt="Maintained"></a>
</p>

# wimor

An open-source, highly optimized HvH & legit utility for Counter-Strike: Global Offensive on Linux (forked from gamesneeze).

## 🚀 Key Improvements & Fork Features

This fork focuses on bringing actual **HvH capabilities** and user experience enhancements:

### ⚡ Ragebot AutoWall (Wall Penetration)
* Real-time iterative bullet simulator (up to 4 wall penetrations).
* Custom game-accurate surface penetration modifiers (Concrete, Wood, Metal, Glass, Tile, Plaster, etc.).
* CS:GO matching damage drop-off formulas and material damage multipliers.
* Exact hitbox-level scale damage calculations incorporating helmet and armor values.
* Uses standard `MASK_SHOT` (`0x4600400B`) physics trace masks.

### 🎯 Aim & Firing Enhancements
* **AutoShoot Toggle/Hold Mode**: Switch between traditional key hold or click-to-toggle auto-firing.
* **AutoShoot HUD Indicator**: Clear visual status (`TOGGLED`, `HOLDING`, or `OFF`) rendered right on your overlay.
* **Multi-Hitbox Scanning**: Scan and select hitboxes (Head, Neck, Chest, Stomach, Pelvis) using a custom selection bitmask.
* **Min Damage Slider**: Ensure bullets only fire if the predicted wallbang damage exceeds your threshold.
* **Highest Damage Target Selection**: Intelligently locks onto the player/hitbox receiving the most damage.
* **Ammo & Gun Checks**: Automatically ignores utilities, knives, or empty guns to keep gameplay clean and reload cycles smooth.

### 👁️ Scoped Third Person
* **Third Person While Scoped**: Remain in thirdperson mode when scoping with sniper rifles (SSG08, AWP, SCAR-20, G3SG1).
* **Automatic Scope Overlay Removal**: Temporarily blocks the black HUD scope lines while scoped in thirdperson, ensuring a clean field of view while retaining scoped accuracy.

### 🎨 Visual & Rebranding Update
* Fully rebranded UI and paths under **`wimor`** (configuration is stored in `~/.wimor/wimor.cfg`).
* Refreshed premium dark blue UI color scheme based on **`#190191`**.
* Default clantag initialized to `"LINUX | WIMOR "` with marquee scroll support.

---

## Current Features

- **Ragebot**: AutoWall, Multi-Hitbox Scan, Min Damage, Target Selection (Crosshair, Distance, Highest Damage), Force Baim, AutoShoot (Toggle/Hold), AutoPeek, Resolver.
- **AntiAim**: Static, Jitter, Fake Jitter, Real Jitter, Spin, Slow Walk, FakeLag (up to 14 ticks).
- **Legitbot**: Smooth, Custom FOV, Override configs for all weapon types, Recoil Compensation, Aim while blind.
- **Visuals**: Box ESP, Skeleton ESP, Health bars, Armor, Chams (Flat, Metallic, Glass), Backtrack Chams, Item/Grenade/C4 ESP, World Color Modulation, Skybox Changer, Nightmode, Fog Override, Third Person (including Scoped).
- **Misc**: Clantag Changer, Chat Filter Bypass, Auto Defuse, Auto Accept, Rank Revealer, Hitmarkers & Damage Markers, JumpBug, EdgeBug, Edge Jump, Auto Strafe, Flappy Bird clone.

---

## How to build & run

### Debian / Ubuntu / Pop OS / Linux Mint required packages:
```sudo apt install -y libsdl2-dev cmake git gcc-10 g++-10 gdb clang```

### Arch / Manjaro required packages:
```sudo pacman -S --needed base-devel git cmake gdb sdl2 clang```

### Fedora required packages:
```sudo dnf install gcc-g++ gdb SDL2-devel cmake git clang```

### Run Build & Inject:
```sh
git clone https://github.com/whoisheremoron/WIMOR-LINUX.git
cd WIMOR-LINUX

# 1. Сборка чита
mkdir -p build && cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
make -j$(nproc --all)
cd ..

# 2. Инжект чита (убедись, что игра запущена!)
chmod +x run.sh
./run.sh
```

## Использование `run.sh`
Скрипт `./run.sh` автоматически копирует скомпилированную библиотеку в директорию игры (`csgo legacy/bin`) и осуществляет инжект в процесс `csgo_linux64` через GDB.

*При повторном изменении кода просто перекомпилируй его с помощью `make` в папке `build` и запусти `./run.sh`.*

