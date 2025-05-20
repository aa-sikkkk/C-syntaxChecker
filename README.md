<p align="center">
 <img height="350" width="350" src="https://github.com/user-attachments/assets/960a6c4d-97bc-404e-9089-92539b92a2c9"
</p>
<p align="center">
  <a href="https://github.com/aa-sikkkk/C-syntaxChecker/stargazers"><img src="https://img.shields.io/github/stars/aa-sikkkk/C-syntaxChecker?style=social" alt="Stars"></a>
  <a href="https://github.com/aa-sikkkk/C-syntaxChecker/issues"><img src="https://img.shields.io/github/issues/aa-sikkkk/C-syntaxChecker" alt="Issues"></a>
  <img src="https://img.shields.io/github/license/aa-sikkkk/C-syntaxChecker" alt="License">
</p>


## 📝 Overview

C-SyntaxChecker is a  tool designed to analyze and validate the syntax of C and C++ codebases. It ensures code quality by detecting common syntax errors and providing detailed reports.

![Code-Analysis-Report-05-20-2025_05_20_PM](https://github.com/user-attachments/assets/9ac7e70e-dd50-4527-abdc-a6af689bac52)


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
- **Advanced Code Analysis:**
  - Memory leak detection
  - Pointer usage validation
  - Array bounds checking
  - Type safety verification
  - Naming convention checks
- **Graphical User Interface:**
  - Modern tabbed interface with syntax highlighting
  - Real-time progress tracking
  - Multiple file analysis support
  - Source code and results side-by-side view
  - Status bar for operation feedback
  - Menu bar for easy navigation

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



## 🤝 Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, feel free to open an issue or submit a pull request.

## 📜 License

This project is licensed under the GNU General Public License. See the [LICENSE](LICENSE) file for details.

## 🛠️ Installation & Compilation

### Prerequisites
- GCC compiler
- GTK3 development libraries
- GtkSourceView3 development libraries
- pkg-config

### Installing Dependencies

#### Windows (using MSYS2/MinGW):
```bash
pacman -S mingw-w64-x86_64-gtk3 mingw-w64-x86_64-gtksourceview3 pkg-config
```

#### Ubuntu/Debian:
```bash
sudo apt-get install libgtk-3-dev libgtksourceview-3.0-dev pkg-config build-essential
```

#### Fedora:
```bash
sudo dnf install gtk3-devel gtksourceview3-devel pkgconfig
```

### Compiling the Program

1. Clone the repository:
```bash
git clone https://github.com/aa-sikkkk/C-syntaxChecker.git
cd C-syntaxChecker
```

2. Compile the GUI version:
```bash
cd cSyn/GUI
make
```

3. Run the program:
```bash
./synCheckGUI
```

## 🖥️ Using the GUI

The enhanced GUI provides a user-friendly interface for code analysis:

1. **File Selection:**
   - Use the "Select Files" button to choose one or more C/C++ files
   - Use the "Select Output Directory" button to specify where to save results

2. **Analysis:**
   - Click "Analyze" to start the code analysis
   - Watch the progress bar for real-time updates
   - View the status bar for current operation details

3. **Viewing Results:**
   - Source code is displayed in the "Source Code" tab with syntax highlighting
   - Analysis results appear in the "Analysis Results" tab
   - Results include detailed information about code quality and potential issues

4. **Navigation:**
   - Use the menu bar for quick access to common operations
   - Switch between tabs to view source code and analysis results
   - Scroll through results to see all detected issues

