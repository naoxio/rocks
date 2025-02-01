#!/bin/bash

# Function to clean and build the project with the selected renderer
build_project() {
    local renderer=$1
    # Clean and configure the project with the selected renderer
    xmake clean
    xmake config --renderer=$renderer
    xmake
}

# Ask the user which renderer to use
echo "Which renderer backend would you like to use?"
echo "1. SDL2"
echo "2. Raylib"
read -p "Enter the number of your choice: " renderer_choice

# Determine the renderer based on user input
case $renderer_choice in
    1)
        renderer="sdl2"
        ;;
    2)
        renderer="raylib"
        ;;
    *)
        echo "Invalid choice. Please run the script again and select a valid option."
        exit 1
        ;;
esac

# Build the project with the selected renderer
build_project $renderer

# Ask the user which example to launch
echo "Which example would you like to launch?"
echo "1. hello_world"
echo "2. image_viewer"
echo "3. scroll_container"
read -p "Enter the number of your choice: " example_choice

# Navigate to the build directory
cd build/linux/x86_64/release/

# Launch the selected example
case $example_choice in
    1)
        ./hello_world
        ;;
    2)
        ./image_viewer
        ;;
    3)
        ./scroll_container
        ;;
    *)
        echo "Invalid choice. Please run the script again and select a valid option."
        ;;
esac