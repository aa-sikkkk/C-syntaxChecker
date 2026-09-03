<p align="center">
 <img height="350" width="350" src="https://github.com/user-attachments/assets/960a6c4d-97bc-404e-9089-92539b92a2c9"
</p>
<p align="center">
  <a href="https://github.com/aa-sikkkk/C-syntaxChecker/stargazers"><img src="https://img.shields.io/github/stars/aa-sikkkk/C-syntaxChecker?style=social" alt="Stars"></a>
  <a href="https://github.com/aa-sikkkk/C-syntaxChecker/issues"><img src="https://img.shields.io/github/issues/aa-sikkkk/C-syntaxChecker" alt="Issues"></a>
  <img src="https://img.shields.io/github/license/aa-sikkkk/C-syntaxChecker" alt="License">
</p>


## Overview

**C-SyntaxChecker** is a command-line and graphical tool that analyzes and validates the syntax of **C and C++** source files. Built on a real token-based **lexer** (not naive string matching), it catches common syntax errors, detects real code-quality issues, and produces detailed reports in text, HTML, and JSON.

The project has two interfaces:
- **CLI** (`cSyn/synCheck.exe`) - scriptable, supports plain text / HTML / JSON reports and multiple files.
- **GUI** (`cSyn/GUI/synCheckGUI`) - tabbed interface with syntax highlighting, progress tracking, and batch analysis.

## Table of Contents

- [Features](#features)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Installation & Compilation](#installation--compilation)
- [Using the CLI](#using-the-cli)
- [Using the GUI](#using-the-gui)
- [Running the Tests](#running-the-tests)
- [Troubleshooting](#troubleshooting)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

## Features

### Syntax Validation
- **Bracket Checking:** Validates that all `()`, `[]`, `{}` pairs match (handles nesting correctly).
- **Semicolon Checking:** Detects missing semicolons.
- **Keyword Usage:** Ensures statements and keywords are used correctly.
- **Type Safety & Naming:** Warns on loose type casts and violations of naming conventions.

### Structural Analysis
- **Function Counting:** Counts functions and prototypes.
- **Variable Counting:** Tracks declared variables.
- **Print/Scan Checks:** Validates `printf`/`scanf`-family usage (including format-string argument counts).
- **File Operations Check:** Detects `fopen`/`fclose` and other file-op functions.
- **C++ Constructs Check:** Detects classes and templates in C++ files.

### Advanced Static Analysis
- **Memory leak detection** and **returns of `malloc`/`fopen` not checked for NULL**
- **Null-pointer dereference detection** - catches `*p` where `p` is uninitialized **or** explicitly set to `NULL`/`0`
- **Array bounds & buffer overflow checking**
- **Unused / uninitialized / shadowed variable detection**
- **Format-string validation**
- **Switch fallthrough detection**
- **Dead code detection**
- **Cyclomatic complexity** and **nesting-depth** measurement

### Report Output (CLI)
| Format | Flag | File | Notes |
|--------|------|------|-------|
| Plain text | *(default)* | `output.txt` | Human-readable diagnostics |
| HTML | `-html` | `output.html` | Styled, self-contained, XSS-safe, includes stats & source |
| JSON | `-json` | `output.json` | Machine-readable, validated JSON |

### Graphical User Interface
- Modern tabbed interface with **syntax highlighting** (when GtkSourceView is available)
- Real-time progress tracking
- Multiple file analysis support
- Source and results views
- Status bar and menu bar

## How It Works

C-SyntaxChecker is **token-based**, not regex/`strstr`-based:

1. **Lexing** - a hand-written lexer (`cSyn/lexer.c`) breaks the source into tokens
   (identifiers, keywords, numbers, operators, strings, comments, brackets, ...).
2. **Analysis** - `cSyn/analysis.c` runs a battery of checks over the token stream,
   including bracket matching, semicolon detection, memory/pointer/array checks,
   complexity metrics, and more.
3. **Reporting** - diagnostics are rendered to plain text, HTML, or JSON
   (`cSyn/report.c`).

Because analysis operates on tokens rather than raw text, it avoids the classic
false positives of simple substring matching and can track syntax structure
(e.g. it correctly ignores brackets and semicolons inside comments and strings).

## Project Structure

```
C-syntaxChecker/
+-- Makefile                  # Top-level orchestrator (cli / gui / test / clean)
+-- cSyn/
|   +-- main.c                # CLI entry point (flag parsing, file loop)
|   +-- lexer.c / lexer.h     # Tokenizer - breaks source into tokens
|   +-- analysis.c / analysis.h  # All static analyses + diagnostics
|   +-- report.c / report.h   # HTML + JSON report generators
|   +-- synCheck.exe          # Built CLI binary
|   +-- Makefile              # Builds the CLI
|   +-- GUI/
|       +-- main.c            # GTK3 GUI
|       +-- Makefile          # Builds the GUI (auto-detects GtkSourceView)
|       +-- synCheckGUI       # Built GUI binary
+-- unittest/
    +-- test_syntax_checker.c # Unit test harness (19 tests)
    +-- Makefile              # Builds / runs the tests
```

All core logic lives in the shared `lexer` + `analysis` library, which the CLI and
GUI both use - so the two interfaces report identical results.

## ASCII Art Banner

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




## [Documentations](https://github.com/aa-sikkkk/C-syntaxChecker/wiki)

cSynCheck's documentations are hosted on this repository's [Wiki page](https://github.com/aa-sikkkk/C-syntaxChecker/wiki). It includes comprehensive explanations for how to use the [GUI](https://github.com/aa-sikkkk/C-syntaxChecker/wiki/How-to-Use-the-Tool!) and the [CLI](https://github.com/aa-sikkkk/C-syntaxChecker/wiki/How-to-Use-the-Tool!). The Wiki is open to edits by the community, so you, yes you, can also correct errors or add new contents to the documentations.


## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, feel free to open an issue or submit a pull request.

## License

This project is licensed under the GNU General Public License. See the [LICENSE](LICENSE) file for details.

## Installation & Compilation

### Prerequisites

| Component | Required for |
|-----------|--------------|
| GCC (GNU C compiler) | Everything |
| `pkg-config` | Auto-detecting GTK/GtkSourceView when building the GUI |
| GTK3 dev libraries | The **GUI** (optional - CLI needs none) |
| GtkSourceView 3 **or** 4 dev | Syntax highlighting in the **GUI** (optional) |

> **Tip:** The CLI has **no third-party dependencies** - only GCC is required.

### Installing Dependencies

#### Windows (using MSYS2/MinGW):
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk3 mingw-w64-x86_64-gtksourceview3 pkg-config
```

#### Ubuntu/Debian:
```bash
sudo apt-get install gcc libgtk-3-dev libgtksourceview-3.0-dev pkg-config build-essential
```

#### Fedora:
```bash
sudo dnf install gcc gtk3-devel gtksourceview3-devel pkgconfig
```

### Compiling the Program

1. Clone the repository:
```bash
git clone https://github.com/aa-sikkkk/C-syntaxChecker.git
cd C-syntaxChecker
```

2. **Build everything** from the project root:
```bash
make all        # builds the CLI (cSyn/synCheck.exe)
make gui        # builds the GUI (cSyn/GUI/synCheckGUI)
make test       # builds and runs the unit test suite
make clean      # removes all build artifacts
```

3. **Or build just the CLI** (no GUI required):
```bash
cd cSyn
gcc -Wall -Wextra -g -o synCheck.exe main.c lexer.c analysis.c report.c
```

4. **Build the GUI** (auto-detects GtkSourceView):
```bash
cd cSyn/GUI
make                 # uses system pkg-config
# Or target a prebuilt GTK tree (e.g. the GTK official build layout):
make GTK_BUILD=C:/gtk-build/gtk/x64/release
# Check what was detected:
make install-info
```

### GtkSourceView Detection

The GUI build automatically enables syntax highlighting when it can find
**GtkSourceView 4 or 3**:

- If **GtkSourceView 4** is found -> syntax highlighting via the v4 API.
- Else if **GtkSourceView 3** is found -> syntax highlighting via the v3 API.
- Otherwise -> falls back gracefully to a plain `GtkTextView` (still fully functional).

On Windows, if you use a prebuilt GTK tree (e.g. the GTK build layout at
`C:\gtk-build\gtk\x64\release`), pass it via `GTK_BUILD` for empty
GTK/GtkSourceView to be picked up correctly.

### Runtime PATH (Windows GUI)

At run time the GUI loads GTK DLLs (e.g. `gtk-3-vs17.dll`,
`gtksourceview-4-0.dll`). Ensure the GTK `bin` directory is on your `PATH`, or
launch it with the DLLs visible:

```powershell
$env:PATH = "C:\gtk-build\gtk\x64\release\bin;$env:PATH"
cd cSyn\GUI
.\synCheckGUI.exe
```

## Using the CLI

The CLI runs all token-based analyses and writes reports:

```bash
# Plain text report (output.txt)
cd cSyn
./synCheck.exe path/to/file.c

# HTML + JSON reports too
./synCheck.exe -html -json path/to/file.c

# Analyze multiple files
./synCheck.exe -html file1.c file2.c file3.cpp

# Show help
./synCheck.exe --help
```

Options:
- `-html` - write a styled HTML report to `output.html`
- `-json` - write a machine-readable JSON report to `output.json` (validated)
- `-h, --help` - show usage

Flags may appear before or between file names.

## Using the GUI

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

## Running the Tests

The repository includes a unit-test harness covering **19 scenarios** for the
CLI's analysis engine (brackets, semicolons, functions, variables, memory,
pointers, array bounds, type safety, naming, and a full mixed-issue case).

Run the full suite from the project root:

```bash
make test
```

Or directly:

```bash
cd unittest
gcc -Wall -g -o test_syntax_checker.exe test_syntax_checker.c
./test_syntax_checker.exe
```

A passing run prints `All tests completed successfully!` and exits with code `0`.

## Troubleshooting

**The GUI binary built, but when I run it nothing appears / a DLL error shows.**
The GTK runtime DLLs are not on your `PATH`. Add the GTK `bin` directory
(see the PATH section above) before launching.

**The GUI builds without syntax highlighting (plain text).**
GtkSourceView is not being detected. Install it (or pass `GTK_BUILD=...` pointing
at a prebuilt tree), then rebuild with `make clean && make`.

**`make` is not installed.**
Build the pieces directly with `gcc` as shown in the Compiling section. Only
the top-level orchestration needs GNU Make.

**The CLI reports something oddly or crashes on a large file.**
Files are processed token-by-token with a bounded line length; if you hit an
outlier, please open an issue with the input file so it can be reproduced.

