# Simple Shell (C++)

A minimal, educational shell implemented in C++ that supports sequential commands, conditional AND (`&&`), a single pipe (`|`), and basic output/error redirection. Designed to compile on Windows via MSYS2 (UCRT64) or on Linux/macOS.

## Overview
- Prompt shows `[username@hostname:current_directory]$`
- Enter `exit` to quit
- Executes external programs using `execvp`

See implementation in [simple_shell.cpp](simple_shell.cpp).

## Features
- Sequential commands with `;`
- Conditional execution with `&&` (runs next only if previous returns `0`)
- Single pipeline with `|` (left → right)
- Redirection:
  - Stdout: `>`, `1>`, `>>`, `1>>`
  - Stderr: `2>`, `2>>`
- Quote cleanup: removes single `'` and double `"` quotes in tokens

## Prerequisites (Windows)
- MSYS2 with UCRT64 toolchain installed (provides POSIX APIs like `fork`, `execvp`, `pipe`)
  - Typical compiler path: `C:\msys64\ucrt64\bin\g++.exe`
- VS Code with C/C++ extension (optional, for convenient building)

On Linux/macOS, any modern `g++` (C++17) should work.

## Build

### Option A: VS Code Task
A build task is already configured. With [simple_shell.cpp](simple_shell.cpp) open:
1. Run the default build task: `Terminal → Run Build Task...` (or `Ctrl+Shift+B`).
2. This compiles the active file and produces `simple_shell.exe` next to it.

### Option B: Manual (Windows/MSYS2)
Use MSYS2 UCRT64 `g++`:

```powershell
"C:\msys64\ucrt64\bin\g++.exe" -std=c++17 -Wall -Wextra -O2 simple_shell.cpp -o simple_shell.exe
```

### Option C: Linux/macOS

```bash
g++ -std=c++17 -Wall -Wextra -O2 simple_shell.cpp -o simple_shell
```

## Run

### Windows
```powershell
./simple_shell.exe
```

### Linux/macOS
```bash
./simple_shell
```

Type commands at the prompt. Use `exit` to quit.

## Usage Examples
- Sequential:
  - `echo first; echo second`
- Conditional AND:
  - `true && echo will_run`
  - `false && echo will_not_run`
- Single pipe:
  - `ls | grep cpp`
- Redirection:
  - `echo hello > out.txt`
  - `echo append >> out.txt`
  - `ls nonexist 2> err.txt`
  - `echo both 1>> out.log`

## Limitations
- Only a single `|` pipe between two commands (no multi-stage pipelines)
- No input redirection (`<`)
- No background jobs (`&`) or `||` (OR) operator
- Builtins like `cd` are not implemented; running `cd` as a child does not change the parent shell's directory
- Minimal error handling; failed `execvp` prints `exec failed`

## Internals
- Parsing helpers: token cleanup and splitting via delimiters `;`, `&&`, `|`
- `executeBase()`: applies redirection and calls `execvp`
- `execSingle()`: forks and runs a single command
- `execPipe()`: forks two children and connects them via `pipe`
- `printPrompt()`: shows `[user@host:cwd]$`

All core logic is in [simple_shell.cpp](simple_shell.cpp).

## Troubleshooting
- "exec failed": command not found or not in PATH
- On Windows, ensure MSYS2 UCRT64 environment is installed and selected; pure Win32 environments lack POSIX functions (`fork`, `pipe`)
- Redirection targets are created in the current working directory

## License
Educational use. Adapt as needed for learning and experimentation.
