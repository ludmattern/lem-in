# LEM-IN - Ant Farm Pathfinding Simulator

A C11 ant farm simulator that finds optimal paths to move `n` ants from start to end room in minimum turns.

## Quick Start

```bash
make                    # Build
./lem-in < map_file     # Run
make test              # Test
```

## Input Format

```text
number_of_ants
##start
start_room x y
##end  
end_room x y
room1 x1 y1
room2 x2 y2
room1-room2
room2-room3
```

## Architecture

```text
srcs/
├── main.c          # Program entry point
├── parser.c        # Core parsing logic
├── parse_line.c    # Room/link parsing
├── input.c         # Input handling
├── validator.c     # Input validation
├── hash.c          # Hash table (djb2)
├── error.c         # Error handling
├── output.c        # Output formatting
└── pathfinding.c   # Algorithm (TODO)
```

## Features

### Input Validation

- **Ant count**: Positive integer, first non-comment line
- **Room names**: Cannot start with `L` or `#`, no spaces/dashes
- **Structure**: Exactly one `##start` and `##end` room
- **Links**: Both rooms must exist, no self-links

### Performance

- **Hash table**: O(1) room lookup with djb2 algorithm
- **Memory safe**: RAII pattern, automatic cleanup
- **Scalable**: Supports 20K rooms, 200K links

## Build Options

```bash
make debug     # Debug build with sanitizers
make release   # Standard build
make clean     # Clean objects
make fclean    # Full clean
```

## Testing

Manual tests :
```bash
./lem-in < resources/valid_maps/perfect0      # Valid case
./lem-in < resources/invalid_maps/0_ants     # Invalid case
```

Automated tests :
```bash
make test      # Run comprehensive test suite
```

## Status

- **Parsing & Validation** - Complete
- **Pathfinding Algorithm** - TODO
- **Ant Movement Simulation** - TODO

---

**Current**: Input parsing complete with comprehensive validation  
**Next**: Implement BFS/Dijkstra pathfinding algorithm
