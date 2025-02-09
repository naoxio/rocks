{ pkgs, ... }:
{
  packages = with pkgs; [
    pkg-config
    SDL2
    SDL2_ttf
    SDL2_image
    SDL2_gfx
    gcc
    gdb
  ];

  languages.c.enable = true;

  # Define build script
  scripts.build.exec = ''
    # Ensure build directory exists
    mkdir -p build

    # Get SDL flags
    SDL_FLAGS=$(pkg-config --cflags sdl2 SDL2_ttf SDL2_image SDL2_gfx)
    SDL_LIBS=$(pkg-config --libs sdl2 SDL2_ttf SDL2_image SDL2_gfx)

    echo "Building rocks library..."
    
    # Compile main library source files
    for file in src/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        gcc -c $file -o build/$(basename $file .c).o \
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
        gcc -c $file -o build/$(basename $file .c).o \
          $SDL_FLAGS \
          -I./include \
          -I./clay \
          -DROCKS_USE_SDL2 \
          -D_CRT_SECURE_NO_WARNINGS
      fi
    done

    # Compile renderer files
    for file in src/renderer/sdl2_*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        gcc -c $file -o build/$(basename $file .c).o \
          $SDL_FLAGS \
          -I./include \
          -I./clay \
          -DROCKS_USE_SDL2 \
          -D_CRT_SECURE_NO_WARNINGS
      fi
    done

    # Create static library
    echo "Creating static library..."
    ar rcs build/librocks.a build/*.o

    # Build examples
    echo "Building examples..."
    EXAMPLES="hello_world image_viewer scroll_container text_input"
    for example in $EXAMPLES; do
      if [ -f "examples/$example.c" ]; then
        echo "Building $example..."
        gcc "examples/$example.c" -o "build/$example" \
          build/librocks.a \
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
      cp -r examples/assets build/
    fi

    echo "Build completed successfully!"
  '';

  scripts.clean.exec = ''
    rm -rf build
    echo "Clean completed successfully!"
  '';

  enterShell = ''
    echo "Rocks development environment ready!"
    echo "Available commands:"
    echo "  devenv build  - Build the project"
    echo "  devenv clean  - Clean build artifacts"
  '';
}