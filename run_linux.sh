#!/bin/bash

# Clean and build the project
xmake clean
xmake

# Ask the user which example to launch
echo "Which example would you like to launch?"
echo "1. hello_world"
echo "2. image_viewer"
echo "3. scroll_container"
read -p "Enter the number of your choice: " choice

cd build/linux/x86_64/release/
# Launch the selected example
case $choice in
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