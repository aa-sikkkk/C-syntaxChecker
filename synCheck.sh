#!/bin/bash

# Banner to make the console more interesting
cat << "EOF"
  _|_|_|            _|_|_|                        _|              
_|                _|        _|    _|  _|_|_|    _|_|_|_|    _|_|_|  _|    _|  
_|    _|_|_|_|_|    _|_|    _|    _|  _|    _|    _|      _|    _|    _|_|    
_|                      _|  _|    _|  _|    _|    _|      _|    _|  _|    _|  
  _|_|_|          _|_|_|      _|_|_|  _|    _|      _|_|    _|_|_|  _|    _|  
                                  _|                  
                              _|_|  
  _|_|_|  _|                            _|              
_|        _|_|_|      _|_|      _|_|_|  _|  _|      _|_|    _|  _|_|  
_|        _|    _|  _|_|_|_|  _|        _|_|      _|_|_|_|  _|_|      
_|        _|    _|  _|        _|        _|  _|    _|        _|        
  _|_|_|  _|    _|    _|_|_|    _|_|_|  _|    _|    _|_|_|  _|        
EOF

# Check if either input_file.c or input_file.cpp exists in the current directory
if [ -f "input_file.c" ]; then
    input_file="input_file.c"
elif [ -f "input_file.cpp" ]; then
    input_file="input_file.cpp"
else
    echo "Error: Neither input_file.c nor input_file.cpp found in the current directory."
    exit 1
fi

# This compiles the input file
echo "Compiling $input_file..."
gcc -o main "$input_file"

# This checks if the compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed. Please fix the errors in $input_file."
    exit 1
fi
echo "Compilation successful."

# Runs the shell script and shows the progress of the syntax checker
echo "Running syntax checker..."
(
    while true; do
        echo -n "."
        sleep 1
    done
) &

# Store the PID of the progress indicator
PROGRESS_PID=$!

# Executes the compiled program and saves the output to output.txt
./main > output.txt

# Stops the progress indicator
kill $PROGRESS_PID
wait $PROGRESS_PID 2>/dev/null

# Checks if the output.txt file was created
if [ -f "output.txt" ]; then
    echo -e "\nSyntax checking completed. Results saved to output.txt."
else
    echo -e "\nError: output.txt was not created."
    exit 1
fi

# This stops the script from closing immediately
read -p "Press [Enter] to exit..."
