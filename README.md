# C-SyntaxChecker

## Overview

C-SyntaxChecker is a tool designed to analyze C code for common syntax issues. It checks for bracket matching, keyword usage, function definitions, and more. This tool can help developers maintain clean and error-free code.

## Features

- **Bracket Checking:** Validates that all opening and closing brackets match.
- **Keyword Usage:** Ensures keywords are used correctly throughout the code.
- **Function Counting:** Counts the number of functions and prototypes.
- **Variable Counting:** Tracks declared variables.
- **Print and Scan Functions Check:** Validates usage of print and scan functions.

## ASCII Art Banner

<pre>
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

</pre>

## Usage

### Prerequisites

- Make sure you have `gcc` installed for compiling the C program.
- Ensure the `bash` shell is available for running the script.

### Steps to Run

1. **Clone the repository or download the source files.**
2. **Open a terminal (or PowerShell) and navigate to the folder containing `main.c` and `synCheck.sh`.**

   ```bash
   cd cSyn
