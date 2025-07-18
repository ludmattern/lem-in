# Lem-in Visualizer

A SDL2 graphical visualizer for the 42 School Lem-in project.

## 🎯 Features

- **Graphical display** of rooms and connections
- **Real-time ant animation**
- **Interactive controls** (space to advance to next turn)
- **Modern interface** with earth brown background and green lawn stripe
- **Elliptical rooms** with displayed names
- **Thick connections** between rooms
- **Animated ants** moving between rooms

## 🛠️ Compilation

### Compile the visualizer alone:
```bash
cd visualizer
make
```

### Compile Lem-in AND the visualizer:
```bash
# From the main lem-in directory
make bonus
```

## 🚀 Usage

### Method 1: With the visualizer rule (recommended)
```bash
# From the main lem-in directory
make visualizer MAP=resources/visualizer_maps/clean_test1
```

### Method 2: Manual pipeline
```bash
# From the main lem-in directory
./lem-in < resources/visualizer_maps/clean_test1 | visualizer/visualizer
```

### Method 3: With a generated map
```bash
# Generate a map
./gen/generator_osx --flow-ten > test_map.txt

# Launch the visualizer
make visualizer MAP=test_map.txt
```

## 🎮 Controls

- **Space** : Advance to next turn (when all ants are stopped)
- **R** : Restart animation from the beginning
- **Escape** or **Close window** : Quit the visualizer

## 📁 File structure

```
visualizer/
├── srcs/
│   ├── main.c          # Main entry point
│   ├── init.c          # SDL2 initialization and variables
│   ├── parser.c        # Map and movement parsing
│   ├── renderer.c      # Graphical display
│   └── animation.c     # Ant animation
├── include/
│   └── visualizer.h    # Headers and structures
├── Makefile            # Compilation rules
└── README.md          # This file
```

## 🎨 Graphical interface

- **Background** : Earth brown with green lawn stripe at the top
- **Rooms** : Dark brown ellipses with displayed names
- **Connections** : Thick dark brown lines
- **Ants** : Black dots moving between rooms
- **Interface** : Current turn number displayed at the top

## 🔧 Compilation options

```bash
# Debug mode with sanitizers
make debug

# Optimized release mode
make release

# Clean object files
make clean

# Complete cleanup
make fclean
```

## 📝 Map examples

The visualizer works with all valid Lem-in maps:

```bash
# Simple map
make visualizer MAP=resources/valid_maps/simple_test

# Complex map
make visualizer MAP=resources/visualizer_maps/clean_test2

# Generated map
./gen/generator_osx --flow-thousand > big_map.txt
make visualizer MAP=big_map.txt
```

## ⚠️ Prerequisites

- **SDL2** and **SDL2_ttf** installed
- **GCC** with C11 support
- **Make** for compilation

### SDL2 installation on macOS:
```bash
brew install sdl2 sdl2_ttf
```

## 🐛 Troubleshooting

### "Map file not found" error
Check that the map path is correct:
```bash
ls -la resources/visualizer_maps/
```

### SDL2 compilation error
Check that SDL2 is installed:
```bash
pkg-config --cflags sdl2 sdl2_ttf
```

### Visualizer doesn't display
Check that Lem-in generates output correctly:
```bash
./lem-in < resources/visualizer_maps/clean_test1
```

## 📊 Performance

The visualizer is optimized for:
- **Maps up to 256 rooms**
- **Up to 512 ants**
- **Up to 512 connections**
- **Smooth animation** at 50ms per frame

## 🤝 Contribution

To improve the visualizer:
1. Modify files in `srcs/`
2. Add prototypes in `include/visualizer.h`
3. Recompile with `make re`
4. Test with `make visualizer MAP=test_map`

---

**Developed for the 42 School Lem-in project** 🎓 