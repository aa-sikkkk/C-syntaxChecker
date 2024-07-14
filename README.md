# 🎉 C-SyntaxChecker 🎉

## 📝 Overview

C-SyntaxChecker is a tool designed to analyze C and C++ code for common syntax issues. It checks for bracket matching, keyword usage, function definitions, and more. This tool can help developers maintain clean and error-free code.

## 🌟 Features

- **Bracket Checking:** Validates that all opening and closing brackets match.
- **Keyword Usage:** Ensures keywords are used correctly throughout the code.
- **Function Counting:** Counts the number of functions and prototypes.
- **Variable Counting:** Tracks declared variables.
- **Print and Scan Functions Check:** Validates usage of print and scan functions.
- **File Operations Check:** Identifies file operation functions like fopen and fclose.
- **Semicolon Checking:** Detects missing semicolons in the code.
- **C++ Constructs Check:** Checks for C++ specific constructs like classes and templates.
- **C++ Specific Checks:** Checks for class and template usage in C++ files.

## 🎨 ASCII Art Banner

```
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
```




## 🚀 Usage

**Note you can use Both <code>CLI</code> and <code>GUI</code> Version of the tool as per your convenience!**


### Prerequisites

- Make sure you have `gcc` installed for compiling the C program.
- Ensure the `bash` shell is available for running the script.
- Install GCC Compiler Download and install GCC for Windows.
- Install GTK+ Libraries
- Download the GTK+ runtime environment for Windows from GTK Website.
- Install <code>libgtk-3.dll</code> on your system. Ensure it's accessible in your system's PATH.

### Steps to Run CLI

1. **Clone the repository or download the source files.**
    ```bash
   git clone https://github.com/aa-sikkkk/C-syntaxChecker.git or downalod from https://github.com/aa-sikkkk/C-syntaxChecker/releases/
   ```
3. **Open a terminal (or PowerShell) and navigate to the folder containing `main.c` and `synCheck.sh`.**

   ```bash
   cd cSyn
   gcc main.c -o synCheck
   ./synCheck input_file.c
   ```
   
## 🚀 GUI
Steps to Run (GUI Version)
   - Download the executable file <CODE>analyzer.exe</CODE> from the [releases section](https://github.com/aa-sikkkk/C-syntaxChecker/releases/).
   - Ensure you have the necessary <CODE>GTK libraries</CODE> installed.
   - For Windows, install <CODE>libgtk-3-dev</CODE> and set the appropriate <CODE>environment variables</CODE>.
   - Open a terminal or command prompt and navigate to the directory containing analyzer.exe.
   - ./analyzer.exe
<p align="center">
   <img src="https://github.com/user-attachments/assets/791d5333-4628-474d-951c-1cc7739550d7">
</p>


## 🤝 Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, feel free to open an issue or submit a pull request.

## 📜 License

This project is licensed under the GNU General Public License. See the [LICENSE](LICENSE) file for details.

