#!/bin/bash

# Banner to make the console more interesting
echo "  _|_|_|            _|_|_|                        _|              "
echo "_|                _|        _|    _|  _|_|_|    _|_|_|_|    _|_|_|  _|    _|  "
echo "_|    _|_|_|_|_|    _|_|    _|    _|  _|    _|    _|      _|    _|    _|_|    "
echo "_|                      _|  _|    _|  _|    _|    _|      _|    _|  _|    _|  "
echo "  _|_|_|          _|_|_|      _|_|_|  _|    _|      _|_|    _|_|_|  _|    _|  "
echo "                                  _|                  "
echo "                              _|_|  "
echo "  _|_|_|  _|                            _|              "
echo "_|        _|_|_|      _|_|      _|_|_|  _|  _|      _|_|    _|  _|_|  "
echo "_|        _|    _|  _|_|_|_|  _|        _|_|      _|_|_|_|  _|_|      "
echo "_|        _|    _|  _|        _|        _|  _|    _|        _|        "
echo "  _|_|_|  _|    _|    _|_|_|    _|_|_|  _|    _|    _|_|_|  _|        "

# This checks if file main.c exists in the current directory
if [ ! -f "main.c" ]; then
    echo "Error: main.c not found in the current directory."
    exit 1
fi

# This compiles the main.c file
echo "Compiling main.c..."
gcc -o main main.c

# This checks if the compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed. Please fix the errors in main.c."
    exit 1
fi

# Runs the shell script and shows the progress of the syntax checker
echo "Running syntax checker..."
while :; do
    echo -n "."
    sleep 1
done &  # Runs the progress indicator in the background

# Executes the compiled program and saves the output to output.txt
./main > output.txt

# stops the progress indicator
kill $!

# checks if the output.txt file was created
if [ -f "output.txt" ]; then
    echo -e "\nSyntax checking completed. Results saved to output.txt."
else
    echo "Error: output.txt was not created."
fi

# This stops the script from closing immediately
read -p "Press [Enter] to exit..."
