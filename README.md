# Ruins of the Unown

A two-player cooperative puzzle-adventure built with **Qt 6.10 Widgets** and **C++17** (qmake).
Two explorers share one keyboard, survive hazards, solve puzzles, and collect **sigils** (clues)
that unlock the final treasure. Course project for FC2.

> **Status: Milestone 1 in progress** (Level 1 + shared interfaces). Game flow, HUD, clue
> storage and level restart are done; movement, world, hazards and puzzles are being built by
> teammates against the shared interfaces.

## Team

| Member | Area |
|--------|------|
| Adham  | Game flow, HUD, completion/restart, integration, UML (A1–A5) |
| Ahmed  | Input, movement, animation, health, character art (B1–B5) |
| Malek  | World, collisions, hazards, Level 1 layout, level reset (C1–C5) |
| Ali    | Puzzles, co-op mechanic, clues, Level 1 puzzles, hints (D1–D6) |

## Build & run

Requirements: **Qt 6.10** (Widgets) and a C++17 compiler.

**Qt Creator:** open `RuinsOfTheUnown.pro`, pick a Qt 6 kit, press Run.

**Command line:**
```bash
qmake6 RuinsOfTheUnown.pro      # or: qmake
make                            # or: mingw32-make / nmake on Windows
./RuinsOfTheUnown
```

All asset paths must be relative or come from `resources.qrc` (no absolute paths), so a fresh
clone builds on any machine.

## How to play (current build)

1. Click anywhere on the title screen to start Level 1.
2. A level ends only when **both explorers stand on the exit and the level's clue is collected**.
3. If either explorer reaches 0 HP the level restarts. Clues from earlier levels are kept;
   clues from the current level are lost.

Real movement keys are not merged yet. Until then, **temporary debug hotkeys** drive the stubs:

| Key | Effect |
|-----|--------|
| `1` / `2` | Damage Explorer A / B by 50 HP |
| `3` | Collect Sigil 1 |
| `4` / `5` | Toggle Explorer A / B on the exit |

These are removed in task A5.

## Architecture

![Current UML](docs/uml_m1.png)

*Current code. Vector version: [`docs/uml_m1.pdf`](docs/uml_m1.pdf), source: [`docs/uml_m1.dot`](docs/uml_m1.dot).*

| Class | Responsibility |
|-------|----------------|
| `MainWindow` | Owns the scene/view, HUD, banner, timers and the `GameState` machine. Runs the 16 ms game loop and enforces the rules: death check **before** exit check, exit requires clue **and** both explorers, two-tier clue storage. |
| `HUDWidget` | Mouse-transparent overlay: two health bars, level label, five sigil slots. Displays state only; it owns no game logic. |
| `GameState` | `Title → Playing → LevelComplete → NextLevelStub` |
| `Config` | Shared constants (window size, tick rate, max HP, banner time). |

**Game flow**

```
Title --click--> Playing --(clue && both at exit)--> LevelComplete --2.5 s--> NextLevelStub
                    |
                    +--(either explorer dead)--> restartCurrentLevel()
```

`MainWindow.cpp` is divided into numbered sections (setup, overlay, state machine, input, game
loop, level lifecycle, clue storage). Each carries `TODO` notes saying which teammate or
milestone plugs in there.

### Planned shared interfaces (task A5)

![Planned interfaces](docs/uml_m1_planned.png)

*Dashed = not yet written. Vector version: [`docs/uml_m1_planned.pdf`](docs/uml_m1_planned.pdf).
The main UML is regenerated once these headers are merged.*

## Repository layout

```
RuinsOfTheUnown.pro      qmake project
resources.qrc            Qt resource file
main.cpp                 entry point
mainwindow.{h,cpp,ui}    game window, state machine, game loop
HUDWidget.{h,cpp}        HUD overlay
assets/                  sprites/, tiles/, ui/  (original or properly licensed only)
docs/                    UML diagrams (.dot source, .pdf, .png)
```

## Roadmap

**Milestone 1 (due 21 Nov 2026)**
- [x] A1 Repo and Qt setup
- [x] A2 Game flow (click-to-start, completion banner)
- [x] A3 HUD (health bars, level label, sigil slots)
- [x] A4 Exit validation, death restart, clue persistence
- [ ] A5 Publish `Explorer.h`, `Hazard.h`, `Puzzle.h`, `Level.h`; replace debug hotkeys; final UML
- [ ] Teammate tasks B1–B5, C1–C5, D1–D6

**Milestone 2 (due 10 Dec 2026)**
- [ ] 5 levels, moving hazards/guardians from Level 2, at least 4 puzzle types
- [ ] Title menu (Play, Settings, Credits, Exit) and 5-level select
- [ ] Progress and replay via `save.txt` (`unlocked_level=3`, `clues=1,2`)
- [ ] Bonus: data-driven level loading from external level files

## Contributing

- Branch per task, named as in the progress tracker (`feature/hud`, `feature/movement`, ...).
- Small, focused commits with prefixes like `feat(...)`, `fix(...)`, `refactor(...)`, `docs(...)`.
- Open a PR into `main`; do not push directly.
- Build locally before opening the PR.
- Design puzzles and code structure as a team. Per the course AI policy, no AI-generated full
  classes or complete solutions, and keep the AI usage log up to date.

## Credits

Qt 6 (LGPL). Asset credits will be listed here as art is added.
