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

# Function to display error messages
error_message() {
    echo -e "\e[31mError: $1\e[0m"
    exit 1
}

# Function to display success messages
success_message() {
    echo -e "\e[32m$1\e[0m"
}

# Function to display progress
show_progress() {
    local pid=$1
    local delay=0.1
    local spinstr='|/-\'
    while ps -p $pid > /dev/null; do
        local temp=${spinstr#?}
        printf " [%c]  " "$spinstr"
        local spinstr=$temp${spinstr%"$temp"}
        sleep $delay
        printf "\b\b\b\b\b\b"
    done
    printf "    \b\b\b\b"
}

# Check for required tools
command -v gcc >/dev/null 2>&1 || error_message "gcc is required but not installed"
command -v make >/dev/null 2>&1 || error_message "make is required but not installed"

# Create output directory if it doesn't exist
mkdir -p output

# Find all C and C++ files in the current directory
files=($(find . -maxdepth 1 -type f \( -name "*.c" -o -name "*.cpp" \)))

if [ ${#files[@]} -eq 0 ]; then
    error_message "No .c or .cpp files found in the current directory"
fi

# Process each file
for file in "${files[@]}"; do
    echo "Processing $file..."
    
    # Compile the file
    echo "Compiling $file..."
    gcc -o "${file%.*}" "$file" 2> "output/${file##*/}.compile.log"
    
    if [ $? -ne 0 ]; then
        error_message "Compilation failed for $file. Check output/${file##*/}.compile.log for details"
    fi
    
    success_message "Compilation successful for $file"
    
    # Run the syntax checker
    echo "Running syntax checker..."
    ./synCheck.exe "$file" > "output/${file##*/}.analysis.txt" &
    checker_pid=$!
    
    # Show progress while the checker is running
    show_progress $checker_pid
    
    # Wait for the checker to complete
    wait $checker_pid
    
    if [ $? -ne 0 ]; then
        error_message "Syntax checking failed for $file"
    fi
    
    success_message "Syntax checking completed for $file"
done

# Generate summary report
echo "Generating summary report..."
{
    echo "Syntax Check Summary Report"
    echo "=========================="
    echo "Date: $(date)"
    echo
    echo "Files Processed:"
    for file in "${files[@]}"; do
        echo "- $file"
    done
    echo
    echo "Results:"
    for file in "${files[@]}"; do
        echo "- $file: output/${file##*/}.analysis.txt"
    done
} > "output/summary.txt"

success_message "Analysis complete! Check the output directory for results."
echo "Summary report: output/summary.txt"

# Keep the window open
read -p "Press [Enter] to exit..."
