# Read-Write-Buffer
Buffered File I/O with O_PREAPPEND Flag
# Buffered File I/O with Custom Flag (O_PREAPPEND)

## Overview

This project develops a buffered I/O library in C, enhancing the basic file handling operations to include buffered reading, writing, and a custom implementation of a new flag, `O_PREAPPEND`. This flag allows data to be written at the beginning of the file without overwriting the existing content, effectively prepending data to files in a managed way.

## Project Description

### Objective

The primary objective is to understand and implement a buffered I/O system that incorporates a unique approach to file writing using the `O_PREAPPEND` flag, alongside standard buffered read and write functionalities.

### Implementation Details

- **Buffered File Structure**: Utilizes `buffered_file_t` to manage buffers for read and write operations separately, maintaining buffer sizes and current positions, along with original file flags.
- **Buffered Open Function**: Handles file opening while managing custom flag removal to maintain compatibility with standard file operations.
- **Buffered Write Function**: Manages data writing to a buffer and implements logic to handle buffer flushing when full or when specific conditions are met.
- **Buffered Read Function**: Fills the read buffer from the file when empty and serves read requests from this buffer, maintaining efficiency and reducing direct file access calls.
- **Flush and Close Functions**: Ensure all buffered data is written to the file upon program completion or when buffers are full, and handles file closure while ensuring data integrity.

### Features

1. **Custom Preappend Flag**:
   - Implements `O_PREAPPEND` to allow writing at the beginning of the file, ensuring no existing data is overwritten by managing data shifts within the file.

2. **Dynamic Buffer Management**:
   - Uses separate buffers for reading and writing operations to optimize I/O performance and data management.

3. **Error Handling**:
   - Robust error handling capabilities to address potential issues during file operations, buffer management, or system call failures.

4. **Command-Line Interface**:
   - Supports passing command-line arguments to define messages and the number of write operations, demonstrating flexible and dynamic use cases.

## Usage

To compile the project, use a standard C compiler such as GCC:

```bash
gcc -o buffered_io part2.c
To run the program and direct outputs to a file, use the command:

bash
Copy code
./buffered_io "Message1" "Message2" "Message3" 5 > output2.txt
This command specifies three different messages to write and sets each message to be written five times.

Examples and Tests
The program is designed to handle edge cases, such as simultaneous read and write operations, and manage buffer states effectively when switching between operations.

Contributions
Contributions are welcome. If you have ideas on how to improve the buffering logic or extend the functionality of the O_PREAPPEND flag, please fork this repository and submit a pull request.

License
This project is open-sourced under the MIT License.

Contact
For further information or to report issues, please contact me at [RanWurembrand@gmail.com].
