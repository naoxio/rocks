{ pkgs, ... }:
{
  packages = with pkgs; [
    pkg-config
    SDL2
    SDL2_ttf
    SDL2_image
    SDL2_gfx
    raylib
    gcc
    gdb
  ];

  languages.c.enable = true;

  # Define build scripts for SDL version
  scripts.build-sdl.exec = ''
    # Ensure build directory exists
    mkdir -p build/sdl

    # Get SDL flags
    SDL_FLAGS=$(pkg-config --cflags sdl2 SDL2_ttf SDL2_image SDL2_gfx)
    SDL_LIBS=$(pkg-config --libs sdl2 SDL2_ttf SDL2_image SDL2_gfx)

    echo "Building rocks library with SDL renderer..."
    
    # Compile main library source files
    for file in src/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        gcc -c $file -o build/sdl/$(basename $file .c).o \
          $SDL_FLAGS \
          -I./include \
          -I./clay \
          -DROCKS_USE_SDL2 \
          -D_CRT_SECURE_NO_WARNINGS
      fi
    done

    # Compile component files
    for file in src/components/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        gcc -c $file -o build/sdl/$(basename $file .c).o \
          $SDL_FLAGS \
          -I./include \
          -I./clay \
          -DROCKS_USE_SDL2 \
          -D_CRT_SECURE_NO_WARNINGS
      fi
    done

    # Compile SDL renderer files
    for file in src/renderer/sdl2_*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        gcc -c $file -o build/sdl/$(basename $file .c).o \
          $SDL_FLAGS \
          -I./include \
          -I./clay \
          -DROCKS_USE_SDL2 \
          -D_CRT_SECURE_NO_WARNINGS
      fi
    done

    # Create static library
    echo "Creating SDL static library..."
    ar rcs build/sdl/librocks.a build/sdl/*.o

    # Build examples
    echo "Building SDL examples..."
    EXAMPLES="hello_world image_viewer scroll_container text_input"
    for example in $EXAMPLES; do
      if [ -f "examples/$example.c" ]; then
        echo "Building $example..."
        gcc "examples/$example.c" -o "build/sdl/$example" \
          build/sdl/librocks.a \
          $SDL_FLAGS $SDL_LIBS \
          -I./include \
          -I./clay \
          -DROCKS_USE_SDL2 \
          -D_CRT_SECURE_NO_WARNINGS \
          -lm -ldl -lpthread
      fi
    done

    # Copy assets
    if [ -d "examples/assets" ]; then
      echo "Copying assets..."
      cp -r examples/assets build/sdl/
    fi

    echo "SDL build completed successfully!"
  '';

  # Define build scripts for Raylib version
  scripts.build-raylib.exec = ''
    # Ensure build directory exists
    mkdir -p build/raylib

    # Get Raylib flags
    RAYLIB_FLAGS=$(pkg-config --cflags raylib)
    RAYLIB_LIBS=$(pkg-config --libs raylib)

    echo "Building rocks library with Raylib renderer..."
    
    # Compile main library source files
    for file in src/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        gcc -c $file -o build/raylib/$(basename $file .c).o \
          $RAYLIB_FLAGS \
          -I./include \
          -I./clay \
          -DROCKS_USE_RAYLIB \
          -D_CRT_SECURE_NO_WARNINGS
      fi
    done

    # Compile component files
    for file in src/components/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        gcc -c $file -o build/raylib/$(basename $file .c).o \
          $RAYLIB_FLAGS \
          -I./include \
          -I./clay \
          -DROCKS_USE_RAYLIB \
          -D_CRT_SECURE_NO_WARNINGS
      fi
    done

    # Compile Raylib renderer files - explicitly compile raylib_renderer.c
    echo "Compiling raylib renderer..."
    gcc -c src/renderer/raylib_renderer.c -o build/raylib/raylib_renderer.o \
      $RAYLIB_FLAGS \
      -I./include \
      -I./clay \
      -DROCKS_USE_RAYLIB \
      -D_CRT_SECURE_NO_WARNINGS

    # Create static library including all object files
    echo "Creating Raylib static library..."
    ar rcs build/raylib/librocks.a build/raylib/*.o

    # Build examples with explicit linking of all necessary libraries
    echo "Building Raylib examples..."
    EXAMPLES="hello_world image_viewer scroll_container text_input dropdown"
    for example in $EXAMPLES; do
      if [ -f "examples/$example.c" ]; then
        echo "Building $example..."
        gcc "examples/$example.c" -o "build/raylib/$example" \
          build/raylib/librocks.a \
          $RAYLIB_FLAGS $RAYLIB_LIBS \
          -I./include \
          -I./clay \
          -DROCKS_USE_RAYLIB \
          -D_CRT_SECURE_NO_WARNINGS \
          -lm -ldl -lpthread
      fi
    done

    # Copy assets
    if [ -d "examples/assets" ]; then
      echo "Copying assets..."
      cp -r examples/assets build/raylib/
    fi

    echo "Raylib build completed successfully!"
  '';

  scripts.clean.exec = ''
    rm -rf build
    echo "Clean completed successfully!"
  '';

  enterShell = ''
    echo "Rocks development environment ready!"
    echo "Available commands:"
    echo "  build-sdl    - Build the project with SDL renderer"
    echo "  build-raylib - Build the project with Raylib renderer"
    echo "  clean        - Clean build artifacts"
  '';
}