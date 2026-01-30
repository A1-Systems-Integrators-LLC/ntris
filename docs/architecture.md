# ntris Architecture Document

**Project**: ntris - NES-style Tetris clone for Linux terminal
**Version**: 1.0
**Date**: 2026-01-29
**Status**: Planning Phase

---

## Table of Contents

1. [Requirements Analysis](#requirements-analysis)
2. [Architectural Overview](#architectural-overview)
3. [Technology Stack](#technology-stack)
4. [Module Design](#module-design)
5. [Data Architecture](#data-architecture)
6. [API Design](#api-design)
7. [Game Loop Architecture](#game-loop-architecture)
8. [Build System](#build-system)
9. [Testing Strategy](#testing-strategy)
10. [Implementation Phases](#implementation-phases)
11. [Architectural Decisions](#architectural-decisions)

---

## Requirements Analysis

### Functional Requirements

**P0 (MVP - Core Gameplay)**:
1. 10×20 game board with cell states (empty/colored)
2. All 7 standard Tetromino pieces (I, O, T, S, Z, J, L)
3. Random piece spawning with game-over detection
4. Movement controls (left, right, soft drop, hard drop)
5. Clockwise rotation with wall kicks
6. Automatic gravity with level-based speed
7. Line clearing with row collapse
8. Scoring system (line clears, drops)
9. Level progression (every 10 lines)
10. Game over state detection
11. Full UI rendering (board, next piece, stats)
12. Non-blocking keyboard input with pause/quit
13. Frame-based timing system
14. Terminal bell for line clears

**P1 (Polish Features)**:
1. Ghost piece preview
2. Title/start screen with level selection
3. Session high score tracking

### Non-Functional Requirements

**Performance**:
- Consistent frame rate (60 FPS target)
- No perceivable input lag
- Smooth rendering without flicker

**Quality**:
- Zero compiler warnings with strict flags
- No memory leaks (static allocation only)
- Robust error handling for terminal edge cases

**Portability**:
- POSIX-compliant C99
- Works on any Linux distribution with ncurses
- Graceful degradation on small terminals

**Maintainability**:
- Clear module boundaries
- Self-documenting code with minimal comments
- Each module testable in isolation

---

## Architectural Overview

### System Context

```
┌─────────────────────────────────────────┐
│         Linux Terminal (ncurses)        │
│  ┌───────────────────────────────────┐  │
│  │         ntris Binary              │  │
│  │                                   │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │      Game Loop (Main)       │  │  │
│  │  └──┬────────┬─────────┬───────┘  │  │
│  │     │        │         │          │  │
│  │  ┌──▼──┐  ┌──▼──┐   ┌──▼──┐      │  │
│  │  │Input│  │Game │   │Render│     │  │
│  │  │     │  │Logic│   │      │     │  │
│  │  └─────┘  └──┬──┘   └─────┘      │  │
│  │              │                    │  │
│  │      ┌───────┼───────┐            │  │
│  │   ┌──▼──┐ ┌──▼───┐ ┌▼────┐       │  │
│  │   │Board│ │Piece │ │Timing│      │  │
│  │   └─────┘ └──────┘ └──────┘      │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
     ▲                           │
     │  Keyboard Input           │ Display Output
     └───────────────────────────┘
```

### Architectural Style

**Modular Procedural Architecture**:
- Each module is a compilation unit (`.h` + `.c`)
- Modules expose public APIs through headers
- Data flows through function calls, not global state
- Game loop coordinates all modules

**Rationale**: Procedural C architecture provides:
- Clear ownership boundaries
- Simple build process
- Predictable performance
- Easy debugging (stack traces, gdb)

### Module Hierarchy

```
main.c
  ├── timing.h       (frame loop, delta time)
  ├── input.h        (keyboard handling)
  ├── game.h         (game state machine)
  │   ├── board.h    (grid, collision, line clearing)
  │   └── piece.h    (tetromino data, rotation)
  ├── render.h       (ncurses UI)
  └── sound.h        (terminal bell)
```

**Dependency Rules**:
1. Lower layers never depend on higher layers
2. `main.c` depends on all top-level modules
3. `game.h` depends on `board.h` and `piece.h`
4. `board.h` and `piece.h` are foundation modules (no game-specific dependencies)
5. `render.h` reads game state but never modifies it

---

## Technology Stack

### Core Technologies

| Component | Technology | Rationale |
|-----------|-----------|-----------|
| **Language** | C99 | Performance, simplicity, no runtime dependencies |
| **UI Library** | ncurses 6.x | Standard terminal UI, color support, non-blocking input |
| **Build System** | GNU Make | Simple, ubiquitous, no external build tools |
| **Compiler** | GCC 7+ | Best C99 support, strict warning flags |

### Development Tools

| Tool | Purpose |
|------|---------|
| `gcc` | Compilation with `-Wall -Wextra -Werror -std=c99` |
| `make` | Build orchestration |
| `valgrind` | Memory leak detection (should report 0 leaks) |
| `gdb` | Debugging |
| `splint` | Static analysis (optional) |

### External Dependencies

**Runtime**: `libncurses.so` (ncurses library)
**Build-time**: `libncurses-dev` (ncurses headers)

---

## Module Design

### 1. Piece Module (`piece.h`, `piece.c`)

**Purpose**: Tetromino piece definitions, shapes, colors, rotation logic.

**Responsibilities**:
- Define all 7 piece types (I, O, T, S, Z, J, L)
- Store rotation states (0°, 90°, 180°, 270°)
- Provide piece color mappings
- Support rotation queries (get next rotation state)
- No game state - just data and transformations

**Key Data Structures**:
```c
typedef enum {
    PIECE_I, PIECE_O, PIECE_T, PIECE_S,
    PIECE_Z, PIECE_J, PIECE_L, PIECE_COUNT
} PieceType;

typedef enum {
    ROT_0 = 0, ROT_90 = 1, ROT_180 = 2, ROT_270 = 3
} RotationState;

typedef struct {
    int cells[4][2];  // [block_index][x/y]
} PieceShape;
```

**Public API**:
```c
// Get piece shape for given type and rotation
const PieceShape* piece_get_shape(PieceType type, RotationState rotation);

// Get piece color
int piece_get_color(PieceType type);

// Get next rotation state (with wrap-around)
RotationState piece_rotate_cw(RotationState current);

// Get piece name for display
const char* piece_get_name(PieceType type);
```

**Implementation Notes**:
- All shapes stored in compile-time constant arrays
- Rotation tables use 4×4 coordinate grids (standard SRS)
- Colors use ncurses color pairs (1-7)

---

### 2. Board Module (`board.h`, `board.c`)

**Purpose**: Game board grid storage, collision detection, line clearing.

**Responsibilities**:
- Manage 10×20 grid of cells
- Collision detection (piece vs board, piece vs walls)
- Lock piece into board when placed
- Detect complete lines
- Clear lines and shift rows down
- Provide read-only access to grid state

**Key Data Structures**:
```c
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

typedef struct {
    int cells[BOARD_HEIGHT][BOARD_WIDTH];  // 0 = empty, 1-7 = color
} Board;
```

**Public API**:
```c
// Initialize empty board
void board_init(Board* board);

// Check if piece collides with board or walls
bool board_check_collision(const Board* board, PieceType type,
                           RotationState rotation, int x, int y);

// Lock piece into board at given position
void board_lock_piece(Board* board, PieceType type,
                      RotationState rotation, int x, int y);

// Detect and clear full lines, return number cleared
int board_clear_lines(Board* board);

// Read cell value (for rendering)
int board_get_cell(const Board* board, int x, int y);

// Check if spawn position is blocked
bool board_is_spawn_blocked(const Board* board);
```

**Implementation Notes**:
- `board_check_collision` tests all 4 blocks of piece against board boundaries and existing blocks
- `board_clear_lines` scans all rows, removes full rows, shifts above rows down
- No game logic (scoring, levels) - purely spatial operations

---

### 3. Game Module (`game.h`, `game.c`)

**Purpose**: Game state machine, scoring, level progression, piece management.

**Responsibilities**:
- Maintain game state (playing, paused, game over)
- Manage current piece (position, rotation, type)
- Manage next piece queue
- Handle scoring (line clears, drops)
- Track level and lines cleared
- Implement gravity (automatic piece descent)
- Process player actions (move, rotate, drop)

**Key Data Structures**:
```c
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} GameState;

typedef struct {
    Board board;
    GameState state;

    // Current piece
    PieceType current_piece;
    RotationState current_rotation;
    int piece_x, piece_y;

    // Next piece
    PieceType next_piece;

    // Stats
    int score;
    int level;
    int lines_cleared;

    // Timing
    double gravity_timer;
    double lock_delay_timer;
} Game;
```

**Public API**:
```c
// Initialize new game
void game_init(Game* game);

// Spawn next piece (returns false if game over)
bool game_spawn_piece(Game* game);

// Update game state (gravity, lock delay)
void game_update(Game* game, double delta_time);

// Player actions (return true if action succeeded)
bool game_move_left(Game* game);
bool game_move_right(Game* game);
bool game_move_down(Game* game);  // Soft drop
bool game_rotate(Game* game);
void game_hard_drop(Game* game);

// State management
void game_toggle_pause(Game* game);
bool game_is_over(const Game* game);

// Queries
int game_get_score(const Game* game);
int game_get_level(const Game* game);
int game_get_lines(const Game* game);
double game_get_gravity_speed(const Game* game);  // Seconds per row

// Next piece preview
PieceType game_get_next_piece(const Game* game);
```

**Implementation Notes**:
- Gravity speed formula: `speed = 0.8 - ((level - 1) * 0.007)` seconds per row (NES Tetris approximation)
- Scoring: 1 line = 100×level, 2 lines = 300×level, 3 lines = 500×level, 4 lines = 800×level (Tetris!)
- Soft drop awards 1 point per row
- Hard drop awards 2 points per row
- Level increases every 10 lines cleared
- Lock delay: 0.5 seconds before piece locks after landing

---

### 4. Input Module (`input.h`, `input.c`)

**Purpose**: Non-blocking keyboard input handling, key mapping.

**Responsibilities**:
- Configure ncurses for non-blocking input
- Read keyboard input without blocking game loop
- Map keycodes to game actions
- Handle special keys (arrows, spacebar)

**Key Data Structures**:
```c
typedef enum {
    INPUT_NONE,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_DOWN,
    INPUT_ROTATE,
    INPUT_HARD_DROP,
    INPUT_PAUSE,
    INPUT_QUIT
} InputAction;
```

**Public API**:
```c
// Initialize input system (configure ncurses)
void input_init(void);

// Poll for input (non-blocking, returns INPUT_NONE if no key pressed)
InputAction input_poll(void);

// Cleanup input system
void input_cleanup(void);
```

**Implementation Notes**:
- Uses `nodelay(stdscr, TRUE)` for non-blocking `getch()`
- Uses `keypad(stdscr, TRUE)` to enable arrow keys
- Returns immediately if no key pressed (doesn't wait for input)

---

### 5. Timing Module (`timing.h`, `timing.c`)

**Purpose**: Frame loop timing, delta time calculation, consistent game speed.

**Responsibilities**:
- Maintain target frame rate (60 FPS)
- Calculate delta time between frames
- Sleep for remaining frame time to avoid busy-wait
- Handle timer overflow gracefully

**Key Data Structures**:
```c
typedef struct {
    double last_frame_time;
    double target_frame_duration;
} Timer;
```

**Public API**:
```c
// Initialize timer with target FPS
void timer_init(Timer* timer, int target_fps);

// Start frame (call at beginning of game loop)
void timer_start_frame(Timer* timer);

// Get delta time since last frame (in seconds)
double timer_get_delta(const Timer* timer);

// Wait for frame to complete (sleep for remaining time)
void timer_wait_frame(Timer* timer);

// Get current time in seconds
double timer_get_time(void);
```

**Implementation Notes**:
- Uses `clock_gettime(CLOCK_MONOTONIC)` for precise timing
- Target: 60 FPS = 16.67ms per frame
- If frame takes longer than target, delta time is capped to avoid giant jumps

---

### 6. Render Module (`render.h`, `render.c`)

**Purpose**: ncurses rendering, UI layout, color management.

**Responsibilities**:
- Initialize ncurses (colors, screen setup)
- Draw game board with colors
- Draw current piece and ghost piece (P1)
- Draw next piece preview
- Draw score, level, lines UI
- Draw pause/game-over overlays
- Handle screen refresh

**Key Data Structures**:
```c
typedef struct {
    WINDOW* game_win;
    WINDOW* stats_win;
    WINDOW* next_win;
    int color_pairs[8];  // Background + 7 piece colors
} Renderer;
```

**Public API**:
```c
// Initialize ncurses and create windows
void render_init(Renderer* renderer);

// Clear screen
void render_clear(Renderer* renderer);

// Draw entire game state
void render_draw_game(Renderer* renderer, const Game* game);

// Draw UI panels (score, level, next piece)
void render_draw_stats(Renderer* renderer, const Game* game);

// Draw pause overlay
void render_draw_pause(Renderer* renderer);

// Draw game over screen
void render_draw_game_over(Renderer* renderer, int final_score);

// Refresh screen (call once per frame)
void render_refresh(Renderer* renderer);

// Cleanup ncurses
void render_cleanup(Renderer* renderer);
```

**Implementation Notes**:
- Board drawn in main window (centered)
- Stats in right panel (score, level, lines)
- Next piece preview in top-right panel
- Each piece color gets ncurses color pair (1-7)
- Empty cells drawn as dark blocks (`[]` or `··`)
- Filled cells drawn with color (`██`)
- Ghost piece (P1) drawn with dim color or outline

---

### 7. Sound Module (`sound.h`, `sound.c`)

**Purpose**: Terminal bell wrapper for audio feedback.

**Responsibilities**:
- Play terminal bell on line clears
- No dependency on audio libraries

**Public API**:
```c
// Play terminal bell
void sound_beep(void);
```

**Implementation Notes**:
- Calls `beep()` from ncurses
- Fallback to `printf("\a")` if `beep()` not available

---

### 8. Main Module (`main.c`)

**Purpose**: Entry point, initialization, game loop orchestration.

**Responsibilities**:
- Initialize all modules
- Run main game loop
- Coordinate input → update → render cycle
- Handle cleanup on exit

**Main Game Loop**:
```c
int main(void) {
    // Initialize
    Timer timer;
    Renderer renderer;
    Game game;

    timer_init(&timer, 60);
    render_init(&renderer);
    input_init();
    game_init(&game);

    // Game loop
    while (!should_quit) {
        timer_start_frame(&timer);

        // Input
        InputAction action = input_poll();
        handle_input(&game, action);

        // Update
        double delta = timer_get_delta(&timer);
        game_update(&game, delta);

        // Render
        render_clear(&renderer);
        render_draw_game(&renderer, &game);
        render_draw_stats(&renderer, &game);
        if (game_is_paused(&game)) {
            render_draw_pause(&renderer);
        }
        render_refresh(&renderer);

        // Timing
        timer_wait_frame(&timer);
    }

    // Cleanup
    render_cleanup(&renderer);
    input_cleanup();
    return 0;
}
```

---

## Data Architecture

### Memory Management Strategy

**Static Allocation Only**:
- No `malloc()` or `free()` during gameplay
- All game state allocated on stack or in static storage
- Board is fixed-size array (10×20 = 200 ints)
- Piece shapes are compile-time constants

**Rationale**:
- Eliminates memory leaks
- Predictable performance (no heap fragmentation)
- Simpler code (no cleanup logic)
- Suitable for game with fixed memory footprint

### Data Flow

```
Input (Keyboard)
    ↓
game_move_* / game_rotate / game_hard_drop
    ↓
board_check_collision (validate move)
    ↓
Update game.piece_x, game.piece_y, game.current_rotation
    ↓
game_update (gravity timer)
    ↓
board_lock_piece (when piece lands)
    ↓
board_clear_lines
    ↓
Update game.score, game.lines_cleared, game.level
    ↓
render_draw_game (read game state)
    ↓
Output (Terminal Display)
```

### State Persistence

**None** - All state is runtime-only:
- No save/load functionality
- No high score file
- Session high score (P1) stored in memory only

---

## API Design

### Error Handling Strategy

**No Exceptions** (C doesn't have them):
- Boolean return values for operations that can fail
- Null pointer checks at module boundaries
- Assertions for impossible states (debug builds only)

**Example**:
```c
bool game_move_left(Game* game) {
    if (game->state != GAME_STATE_PLAYING) {
        return false;  // Can't move in paused/game-over state
    }

    int new_x = game->piece_x - 1;
    if (board_check_collision(&game->board, game->current_piece,
                              game->current_rotation, new_x, game->piece_y)) {
        return false;  // Collision detected
    }

    game->piece_x = new_x;
    return true;
}
```

### API Conventions

1. **Naming**: `module_verb_noun` format (e.g., `game_move_left`, `board_clear_lines`)
2. **Parameters**: Output parameters last, input parameters first
3. **Const Correctness**: Read-only parameters marked `const`
4. **Ownership**: Caller owns memory for all structures (no hidden allocations)

---

## Game Loop Architecture

### Frame-Based Loop

**Target**: 60 FPS (16.67ms per frame)

**Loop Structure**:
1. **Input Phase**: Poll keyboard, convert to actions
2. **Update Phase**: Apply actions, run gravity, detect collisions
3. **Render Phase**: Draw game state to screen
4. **Wait Phase**: Sleep for remaining frame time

### Timing Considerations

**Gravity Timer**:
- Independent of frame rate
- Accumulates delta time each frame
- When timer exceeds `game_get_gravity_speed()`, piece drops one row
- Resets timer after drop

**Lock Delay**:
- 0.5 second grace period after piece lands
- Allows player to slide piece or rotate before locking
- Timer resets if piece moves off ground

**Input Handling**:
- Non-blocking: Never wait for input
- No input buffering (only process current frame's input)
- Key repeat handled by terminal (OS-level)

---

## Build System

### Makefile Structure

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2
LDFLAGS = -lncurses

SRCS = main.c board.c piece.c game.c input.c timing.c render.c sound.c
OBJS = $(SRCS:.c=.o)
TARGET = ntris

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
```

### Compilation Flags

| Flag | Purpose |
|------|---------|
| `-Wall -Wextra` | Enable all warnings |
| `-Werror` | Treat warnings as errors |
| `-std=c99` | Enforce C99 standard |
| `-O2` | Optimize for performance |
| `-lncurses` | Link ncurses library |

### Build Artifacts

- **Source**: `*.c`, `*.h` (in project root)
- **Objects**: `*.o` (generated during build)
- **Binary**: `ntris` (final executable)

---

## Testing Strategy

### Unit Testing

**Module-Level Self-Tests**:
Each module includes a `_test()` function for isolated testing:

```c
// piece.c
bool piece_test(void) {
    // Test 1: Verify piece shapes are valid
    for (PieceType type = 0; type < PIECE_COUNT; type++) {
        const PieceShape* shape = piece_get_shape(type, ROT_0);
        assert(shape != NULL);
    }

    // Test 2: Verify rotation wrapping
    assert(piece_rotate_cw(ROT_270) == ROT_0);

    return true;
}
```

**Compilation Check**:
```bash
gcc -fsyntax-only -Wall -Wextra -Werror -std=c99 module.c
```

### Integration Testing

**Manual Playtesting Checklist**:
- [ ] All 7 pieces spawn correctly
- [ ] Movement in all directions works
- [ ] Rotation with wall kicks functions
- [ ] Lines clear and score updates
- [ ] Level progression increases speed
- [ ] Game over triggers when spawn blocked
- [ ] Pause and resume work
- [ ] Hard drop places piece correctly
- [ ] Terminal bell sounds on line clear

### Validation Tools

| Tool | Purpose |
|------|---------|
| `valgrind --leak-check=full ./ntris` | Detect memory leaks (should be 0) |
| `gdb ./ntris` | Debug crashes |
| `splint *.c` | Static analysis (optional) |

---

## Implementation Phases

### Phase 1: Foundation (P0.1)

**Goal**: Core data structures and utility modules.

**Modules**:
1. **piece** - Tetromino shapes and rotation tables
2. **board** - Grid storage and collision detection
3. **timing** - Frame loop timing

**Acceptance Criteria**:
- [ ] All piece shapes defined with correct coordinates
- [ ] Board collision detection works for all 7 pieces
- [ ] Timing module maintains consistent 60 FPS

**Dependencies**: None

---

### Phase 2: Game Logic (P0.2)

**Goal**: Game state machine and piece movement.

**Modules**:
1. **game** - Game state, scoring, level progression
2. **input** - Keyboard handling

**Acceptance Criteria**:
- [ ] Game initializes with first piece
- [ ] Piece movement (left, right, down) works
- [ ] Rotation with wall kicks functions
- [ ] Gravity moves piece down automatically
- [ ] Lines clear and score updates
- [ ] Level increases every 10 lines
- [ ] Game over detected

**Dependencies**: Phase 1 (piece, board, timing)

---

### Phase 3: Presentation (P0.3)

**Goal**: Full UI rendering and audio feedback.

**Modules**:
1. **render** - ncurses drawing
2. **sound** - Terminal bell

**Acceptance Criteria**:
- [ ] Board renders with colors
- [ ] Current piece visible
- [ ] Next piece preview shown
- [ ] Score/level/lines displayed
- [ ] Pause overlay works
- [ ] Game over screen displays final score
- [ ] Terminal bell on line clear

**Dependencies**: Phase 2 (game)

---

### Phase 4: Integration (P0.4)

**Goal**: Complete game loop in main.

**Modules**:
1. **main** - Entry point and game loop

**Acceptance Criteria**:
- [ ] Game runs from start to game over
- [ ] All controls functional
- [ ] No crashes or freezes
- [ ] Clean exit with 'q'
- [ ] Valgrind reports 0 leaks

**Dependencies**: Phase 3 (render, sound)

---

### Phase 5: Polish Features (P1)

**Goal**: Enhanced UX features.

**Features**:
1. **Ghost piece** - Preview of landing position
2. **Start screen** - Title and level selection
3. **Session high score** - Track best score

**Acceptance Criteria**:
- [ ] Ghost piece visible during gameplay
- [ ] Start screen allows level 1-10 selection
- [ ] High score persists during session
- [ ] High score displayed on game over

**Dependencies**: Phase 4 (complete P0)

---

## Architectural Decisions

### ADR-001: Use C99 Instead of C++

**Status**: Accepted

**Context**: Need to choose between C99 and C++ for implementation.

**Decision**: Use C99.

**Rationale**:
- Simpler language = fewer edge cases
- No hidden costs (no constructors, destructors, exceptions)
- Easier to reason about memory and performance
- ncurses has cleaner C API than C++ wrappers
- Project doesn't need OOP features

**Consequences**:
- Manual memory management (mitigated by static allocation strategy)
- No RAII for cleanup (mitigated by explicit cleanup functions)
- Less type safety (mitigated by const correctness)

---

### ADR-002: Static Allocation Only

**Status**: Accepted

**Context**: Need memory management strategy for game state.

**Decision**: Use static allocation exclusively (no `malloc`/`free`).

**Rationale**:
- Fixed memory footprint (board is 10×20, pieces are constant)
- Eliminates memory leaks entirely
- Simpler code (no allocation failure paths)
- Better cache locality (data packed together)

**Consequences**:
- Cannot have arbitrarily large data structures
- All limits must be compile-time constants
- Stack size must accommodate all game state (~10 KB)

---

### ADR-003: Procedural Architecture Over Entity-Component-System

**Status**: Accepted

**Context**: Need to choose architectural pattern for game structure.

**Decision**: Use modular procedural architecture.

**Rationale**:
- Simple game with few entity types (only current piece + board)
- No need for ECS flexibility
- Easier to debug (clear call stacks)
- Matches C's strengths (function calls, structs)

**Consequences**:
- Less extensible to complex features (fine for Tetris scope)
- Clearer code for this specific use case

---

### ADR-004: No Configuration Files

**Status**: Accepted

**Context**: Decide whether to support config files for controls/colors.

**Decision**: No configuration files.

**Rationale**:
- Reduces scope and complexity
- Standard controls are universal (arrow keys)
- Colors are standard Tetris palette
- Avoids file I/O error handling

**Consequences**:
- Controls are hardcoded (can be changed in source)
- No user customization (acceptable for MVP)

---

### ADR-005: Frame-Based Loop Over Event-Driven

**Status**: Accepted

**Context**: Choose between frame-based loop and event-driven architecture.

**Decision**: Frame-based loop at 60 FPS.

**Rationale**:
- Predictable performance (fixed frame budget)
- Consistent gravity timing
- Simpler rendering (redraw entire screen each frame)
- Standard for games

**Consequences**:
- CPU usage higher than event-driven (mitigated by frame wait)
- More predictable timing behavior

---

### ADR-006: Compile-Time Piece Rotation Tables

**Status**: Accepted

**Context**: Decide whether to compute piece rotations at runtime or compile-time.

**Decision**: Store all rotation states in compile-time constant arrays.

**Rationale**:
- Only 7 pieces × 4 rotations = 28 states (small data)
- Eliminates rotation computation code
- Faster lookup (no math, just array indexing)
- Easier to verify correctness (visual inspection of coordinates)

**Consequences**:
- Larger data section (acceptable, ~1 KB)
- Less flexible to custom piece definitions (not needed)

---

### ADR-007: Minimal Abstraction Over Board Representation

**Status**: Accepted

**Context**: Decide how to represent board cells (bitfields vs. ints vs. structs).

**Decision**: Use simple integer array (`int cells[20][10]`).

**Rationale**:
- Simple to understand and debug
- Fast access (no indirection)
- Sufficient for color storage (values 0-7)
- No need for per-cell metadata

**Consequences**:
- Uses more memory than bitfields (acceptable, 800 bytes)
- Clearer code and faster development

---

### ADR-008: Single-File Headers (No Header-Only Libraries)

**Status**: Accepted

**Context**: Decide whether to use header-only pattern or separate `.c` files.

**Decision**: Every module has `.h` (declarations) and `.c` (implementation).

**Rationale**:
- Clear separation of interface and implementation
- Faster incremental compilation (no header parsing every time)
- Standard C practice

**Consequences**:
- More files (acceptable)
- Need to maintain header/implementation sync

---

## Summary

This architecture provides:

✅ **Clear module boundaries** - Each module has single responsibility
✅ **Predictable performance** - Static allocation, fixed frame rate
✅ **Simple build process** - Single Makefile, no external tools
✅ **Testable components** - Each module can be tested in isolation
✅ **Maintainable codebase** - Self-documenting structure, minimal comments needed
✅ **Extensible to P1 features** - Ghost piece and start screen fit cleanly into existing modules

**Next Steps**:
1. Create GitHub Issues for each module (Phase 1-5)
2. Implement modules in dependency order (piece → board → game → render → main)
3. Validate each phase before proceeding to next
4. Integrate and test end-to-end

---

**Document Status**: Complete
**Last Updated**: 2026-01-29
**Author**: Claude (Architecture Agent) + Human Stakeholder
