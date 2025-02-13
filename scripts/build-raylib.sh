#!/usr/bin/env bash

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
EXAMPLES="hello_world image_viewer scroll_container text_input dropdown modal grid"
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
