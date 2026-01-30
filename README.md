# ntris

NES-style Tetris clone for the Linux terminal. Single binary, C + ncurses.

## Overview

A faithful recreation of classic NES Tetris gameplay in a terminal environment. The game runs as a single compiled binary with no external runtime dependencies beyond ncurses. Designed for Linux terminals with standard 80x24 or larger dimensions.

## Requirements

### Platform
- **OS**: Linux (POSIX-compliant)
- **Interface**: Terminal UI (TUI) via ncurses
- **Output**: Single statically-usable binary (`ntris`)
- **Terminal**: Minimum 80x24, supports color if available

### Technology
- **Language**: C99
- **UI Library**: ncurses
- **Build System**: Makefile (GNU Make)
- **Compiler**: GCC with `-Wall -Wextra -Werror -std=c99`
- **External Dependencies**: ncurses only (libncurses-dev)

### Infrastructure
- **Hosting**: Local-only (no network, no server)
- **Database**: None
- **External Services**: None
- **Protocols**: stdin/stdout (keyboard input, terminal output)

## Features

### P0 — Core Gameplay (MVP)

These features are required for the game to be playable:

1. **Game Board**: 10-wide × 20-tall grid. Cells are either empty or filled with a color.
2. **Tetromino Pieces**: All 7 standard pieces (I, O, T, S, Z, J, L) with correct shapes and distinct colors.
3. **Piece Spawning**: Random piece selection. New piece spawns at top-center. Game over if spawn position is blocked.
4. **Movement**: Left, right, soft drop (down arrow). Hard drop (spacebar) instantly places piece.
5. **Rotation**: Clockwise rotation (up arrow). Wall kicks when rotation would collide.
6. **Gravity**: Piece falls automatically. Speed increases with level.
7. **Line Clearing**: Full rows are detected and removed. Rows above shift down.
8. **Scoring**: Points awarded for line clears (1/2/3/4 lines = 100/300/500/800 × level). Soft drop and hard drop award points.
9. **Levels**: Start at level 1. Level increases every 10 lines cleared. Each level increases gravity speed.
10. **Game Over**: Detected when a new piece cannot be placed. Display final score.
11. **Render**: Draw board, current piece, next piece preview, score, level, and lines cleared. Use ncurses colors.
12. **Input Handling**: Non-blocking keyboard input. Arrow keys for movement/rotation, spacebar for hard drop, 'q' to quit, 'p' to pause.
13. **Timing**: Frame-based game loop with consistent timing. Gravity tick rate tied to level.
14. **Sound**: Terminal bell (beep) on line clears. No audio library dependency.

### P1 — Polish Features (Post-MVP)

Nice-to-have features after core gameplay works:

1. **Ghost Piece**: Semi-transparent preview showing where the current piece will land.
2. **Start Screen**: Title screen with level selection (1-10) before gameplay begins.
3. **Session High Score**: Track highest score during the current session (not persisted to disk).

### Out of Scope

- Network play / multiplayer
- Persistent high score storage (file I/O)
- Audio beyond terminal bell
- Mouse input
- Configuration files
- Package distribution (deb, rpm, etc.)

## Architecture Guidance

### Module Structure

The game should be organized into focused modules with clear ownership boundaries:

- **Piece module**: Tetromino shapes, rotation tables, color mappings
- **Board module**: Grid storage, collision detection, line clearing
- **Game module**: Game state machine, scoring, level progression
- **Input module**: Non-blocking keyboard handling, key mapping
- **Timing module**: Frame loop, gravity tick, delta time
- **Render module**: ncurses drawing, UI layout, color management
- **Sound module**: Terminal bell wrapper
- **Main**: Entry point, initialization, game loop orchestration

### Design Principles

- Each module exposes a header (`.h`) with its public API and an implementation (`.c`)
- Modules communicate through well-defined function calls, not global state
- The game loop runs in main, calling into modules for input, update, and render
- No dynamic memory allocation for gameplay (static arrays for board, piece data)
- All piece rotation data is compile-time constant (lookup tables, not computed)

### Build

```bash
make        # Build the binary
make clean  # Remove build artifacts
./ntris     # Run the game
```

## Controls

| Key | Action |
|-----|--------|
| ← → | Move piece left/right |
| ↓ | Soft drop (move down faster) |
| ↑ | Rotate clockwise |
| Space | Hard drop (instant placement) |
| p | Pause/unpause |
| q | Quit |
