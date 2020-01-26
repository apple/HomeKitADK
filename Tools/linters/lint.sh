#!/bin/bash -e

ADK_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../"

usage()
{
    echo "A tool to check and fix code style"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "OPTIONS:"
    echo "-f  - Lint the provided file. If no option is provided all files will be linted."
    echo "-d  - Lint the files in the specified directory."
    echo "-c  - Correct the style issues in the specified files or directories."
    echo ""
    echo "EXAMPLES: "
    echo "$0 -f <file_name>          # Lint the specified file"
    echo "$0 -f <file_name> -f <file2> # Lint the two specified files"
    echo "$0 -f <file_name> -c       # Lint and fix the specified file"
    echo "$0 -d <dir_name>           # Lint files in the specified directory"
    echo "$0 -d <dir_name> -c        # Lint and fix all files in the specified directory"
    echo "$0                         # Lint all files in this repository"
    echo "$0 -c                      # Fix all files in this repository"
    exit 1
}

CORRECT=false
SEARCH_PATH=
FILE_ARGS=()
VERBOSE=false

while getopts "hcvf:d:" opt; do
    case ${opt} in
        f ) FILE_ARGS+=( "$OPTARG" );;
        d ) SEARCH_PATH="$OPTARG";;
        c ) CORRECT=true;;
        v ) VERBOSE=true;;
        h ) usage;;
        \? ) usage;;
    esac
done

# If files are passed as args, sort them into source files and shell files.
SOURCE_FILES=()
SHELL_FILES=()
if [ ${#FILE_ARGS[@]} -gt 0 ]; then
    for file in "${FILE_ARGS[@]}"; do
        if [[ ! -f "$file" ]]; then
            echo "Skipping invalid file '$file' (does not exist)"
            continue
        fi

        # Skip files that aren't of the right kind.
        case $file in
            *.h)    SOURCE_FILES+=( "$file" );;
            *.c)    SOURCE_FILES+=( "$file" );;
            *.cpp)  SOURCE_FILES+=( "$file" );;
            *.hpp)  SOURCE_FILES+=( "$file" );;
            *.cc)   SOURCE_FILES+=( "$file" );;
            *.hh)   SOURCE_FILES+=( "$file" );;
            *.cxx)  SOURCE_FILES+=( "$file" );;
            *.m)    SOURCE_FILES+=( "$file" );;
            *.mm)   SOURCE_FILES+=( "$file" );;
            *.sh)   SHELL_FILES+=( "$file" );;
            *)      echo "Skipping lint on '$file' because it is not a source file." ;;
        esac
    done

    ( $VERBOSE ) && echo " - ${#SOURCE_FILES[@]} source files"
    ( $VERBOSE ) && echo " - ${#SHELL_FILES[@]} shell files"

    # Exit early if no files are going to be checked.
    if [[ ${#SOURCE_FILES[@]} -eq 0 && ${#SHELL_FILES[@]} -eq 0 ]]; then
        echo "No files to lint!"
        exit 0
    fi
fi

# Set up args to forward with the right values.
ARG_SEARCH_PATH=()
ARG_CORRECT=

if [ -n "$SEARCH_PATH" ]; then
    ARG_SEARCH_PATH=("-d" "$SEARCH_PATH")
fi

if ( $CORRECT ); then
    ARG_CORRECT=-c
fi

# 1. Do shell lint (if there are shell files to check).
if [ ${#SHELL_FILES[@]} -gt 0 ] || [ ${#FILE_ARGS[@]} -eq 0 ]; then
    "$ADK_ROOT"/Tools/linters/shlint.sh "${ARG_SEARCH_PATH[@]}" "${SHELL_FILES[@]/#/-f}"
fi

# 2. Do source code lint (if there are shell files to check).
if [ ${#SOURCE_FILES[@]} -gt 0 ] || [ ${#FILE_ARGS[@]} -eq 0 ]; then
    "$ADK_ROOT"/Tools/linters/clint.sh "${ARG_SEARCH_PATH[@]}" "${ARG_CORRECT}" "${SOURCE_FILES[@]/#/-f}"
fi
