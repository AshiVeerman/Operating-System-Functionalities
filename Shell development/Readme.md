# Minimal Unix Shell (MTL458 Shell)

## Overview
This program implements a simple Unix-like shell with features such as:
1. Executing standard commands.
2. Piping between commands.
3. Maintaining a history of commands.
4. Handling directory navigation (`cd`).

---

## Features
1. **Standard Command Execution**:
   - Executes commands using `execvp`.
   - Handles invalid commands gracefully by displaying "Invalid Command".

2. **Command History**:
   - Stores the last 100 commands.
   - Displays history using the `history` command.

3. **Piping**:
   - Supports single pipes (e.g., `ls | grep txt`).

4. **Directory Navigation**:
   - `cd <path>`: Navigate to a specified directory.
   - `cd ~`: Navigate to the home directory.
   - `cd -`: Navigate to the previous directory.

5. **Graceful Exit**:
   - Use `exit` to terminate the shell.

---

## How to Compile and Run
1. **Compile the Program**:
   ```bash
   gcc -o shell shell.c

---

## Usage

### Running Commands
- **Execute Basic Commands**:
  Type any valid Unix command and press Enter.
  ```bash
  ls
  pwd
  whoami

---

### Error Handling

The shell handles errors gracefully by displaying the message **"Invalid Command"** in the following scenarios:
1. Invalid commands that do not exist.
2. Errors in using `cd`:
- Non-existent directories.
- Incorrect usage of `cd -` without a previous directory.
3. Incorrect or unsupported piping.
4. Syntax errors in command input.

	Example of invalid command:
    ```bash
    MTL458 > invalidCommand
    Invalid Command

---

### Limitations

1. **Pipe Support**:
   - Only single piping (`|`) is supported.
   - Multi-level pipes like `cmd1 | cmd2 | cmd3` are not implemented.

2. **Command History**:
   - Limited to the last 100 commands.
   - Does not persist across sessions.

3. **Background Processes**:
   - No support for running commands in the background using `&`.

4. **Advanced Features**:
   - Features like job control (`jobs`, `fg`, `bg`) or redirection (`>`, `<`, `>>`) are not supported.

---

### Future Enhancements

1. **Multi-Level Piping**:
   - Implement support for commands with multiple pipes (e.g., `cmd1 | cmd2 | cmd3`).

2. **Background Process Support**:
   - Add the ability to execute commands in the background using `&`.

3. **History Persistence**:
   - Save command history to a file and load it on startup.

4. **Redirection Support**:
   - Implement input (`<`) and output (`>`, `>>`) redirection.

5. **Error Descriptions**:
   - Provide more descriptive error messages for debugging.

6. **Environment Variable Expansion**:
   - Expand commands containing variables (e.g., `$HOME`, `$PATH`).

7. **Job Control**:i
   - Add support for managing jobs with `jobs`, `fg`, and `bg`.
