# Directory File Tree Walker (DFTW)

## Overview
DFTW is a C program that recursively traverses a directory structure and performs various file operations based on command-line arguments. The tool leverages the `nftw()` system call to explore file trees and provides functionalities like counting files and directories, calculating file sizes, copying directory structures, and moving directories.

## Features
- **Count Files**: Lists the total number of files present in a given directory and its subdirectories.
- **Count Directories**: Lists the total number of directories present in a given directory and its subdirectories.
- **Calculate File Size**: Computes the total size of all files in a directory tree (in bytes).
- **Copy Directory Structure**: Copies an entire subdirectory, maintaining its structure at the destination. Optionally, you can exclude files by their extension.
- **Move Directory**: Moves an entire subdirectory to another location, deleting the source after the move.

## Command-Line Arguments

### Count Files
Counts and lists the total number of files in the directory tree rooted at `root_dir`.
1. **Count Files**:  $ dftw -nf /home/user/documents
