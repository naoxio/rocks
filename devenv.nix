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

  # Define build scripts that call our shell scripts
  scripts.build-sdl.exec = ''
    bash ./scripts/build-sdl.sh
  '';

  scripts.build-raylib.exec = ''
    bash ./scripts/build-raylib.sh
  '';

  scripts.clean.exec = ''
    bash ./scripts/clean.sh
  '';

  enterShell = ''
    echo "Rocks development environment ready!"
    echo "Available commands:"
    echo "  build-sdl    - Build the project with SDL renderer"
    echo "  build-raylib - Build the project with Raylib renderer"
    echo "  clean        - Clean build artifacts"
  '';
}