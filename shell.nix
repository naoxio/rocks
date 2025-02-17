{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    pkg-config
    SDL2
    SDL2_ttf
    SDL2_image
    SDL2_gfx
    raylib
    gcc
    gdb
    gnumake
  ];

  shellHook = ''
    export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [
      pkgs.SDL2
      pkgs.SDL2_ttf
      pkgs.SDL2_image
      pkgs.SDL2_gfx
      pkgs.raylib
    ]}"

    echo "Rocks development environment ready!"
    echo "Available make commands:"
    echo "  make        - Build everything"
    echo "  make sdl    - Build only SDL2 version"
    echo "  make raylib - Build only Raylib version"
    echo "  make clean  - Clean all build artifacts"
  '';
}
