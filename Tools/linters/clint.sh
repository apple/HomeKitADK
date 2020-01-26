#!/bin/bash -e

ADK_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../"

# Create a random filename to store our generated patch
prefix="static-check-clang-format"
suffix="$(date +%s)"
PATCH="/tmp/$prefix-$suffix.patch"

finish() {
    rm -f "$PATCH"
}
trap finish EXIT

# shellcheck source=Tools/
source "$ADK_ROOT/Tools/download.sh"

usage()
{
    echo "A tool to check and fix code style in source files."
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
    echo "$0 -c                      # Fix all files in this repository, in parallel"
    exit 1
}

CLANG_FORMAT=clang-format
CORRECT=false
SEARCH_PATH=$ADK_ROOT
PARALLEL=8
SOURCE_FILES=()

while getopts "hcf:d:" opt; do
    case ${opt} in
        f ) SOURCE_FILES+=("$OPTARG");;
        d ) SEARCH_PATH="$OPTARG";;
        c ) CORRECT=true;;
        h ) usage;;
        \? ) usage;;
    esac
done

# Install clang-format
CLANG_VERSION="9.0.0"
if ! "$CLANG_FORMAT" --version | grep -q $CLANG_VERSION; then
    echo "clang-format not installed or the version is incorrect. Installing..."

    CLANG_BIN_FILE=
    DEST=
    case "$(uname)" in
        "Darwin")
            CLANG_BIN_FILE="clang+llvm-$CLANG_VERSION-x86_64-darwin-apple"
            DEST="/usr/local/bin"
            SUDO=
            ;;
        "Linux")
            CLANG_BIN_FILE="clang+llvm-$CLANG_VERSION-x86_64-linux-gnu-ubuntu-18.04"
            DEST="/usr/bin"
            SUDO="sudo"
            ;;
        *)
            echo "Unsupported system"
            exit 1
            ;;
    esac

    download "$CLANG_BIN_FILE.tar.xz" "http://releases.llvm.org/$CLANG_VERSION/$CLANG_BIN_FILE.tar.xz" \
        && tar --xz -xvf "$CLANG_BIN_FILE.tar.xz" "$CLANG_BIN_FILE/bin/clang-format" \
        && "$SUDO" cp -f "$CLANG_BIN_FILE/bin/clang-format" "$DEST/clang-format" \
        && rm -rf "$CLANG_BIN_FILE" "$CLANG_BIN_FILE.tar.xz"
fi

if [ ${#SOURCE_FILES[@]} -gt 0 ]; then
    for file in "${SOURCE_FILES[@]}"; do
        if [[ ! -f "$file" ]]; then
            echo "Skipping invalid file '$file' (does not exist)"
            continue
        fi
    done
else
    echo "Looking for files in $SEARCH_PATH directory..."
    while IFS=  read -r -d $'\0'; do
        SOURCE_FILES+=("$REPLY")
    done < <(find "$SEARCH_PATH"  \
        \( \
            -name "*.h" \
            -o -name "*.c" \
            -o -name "*.cpp" \
            -o -name "*.hpp" \
            -o -name "*.cc" \
            -o -name "*.hh" \
            -o -name "*.cxx" \
            -o -name "*.m" \
            -o -name "*.mm" \
        \) \
        -not -path "*Output*" \
        -not -path "*External*" \
        ! -type l \
        -print0
    )
fi

# Exit early if no files are going to be checked.
if [[ ${#SOURCE_FILES[@]} -eq 0 ]]; then
    echo "No source files to lint!"
    usage
fi

if ( $CORRECT ); then
    printf "%s\0" "${SOURCE_FILES[@]}" | xargs -t -0 -P ${PARALLEL} -I{} -- "$CLANG_FORMAT" -style=file {} -i
else
    for file in "${SOURCE_FILES[@]}"; do
        echo "Checking file: $file"
        "$CLANG_FORMAT" -style=file "$file" | \
            diff -u "$file" - | \
            sed -e "1s|--- |--- a/|" -e "2s|+++ -|+++ b/$file|" >> "$PATCH"
    done
fi

# If user asked to fix the style issues then we are done
if $CORRECT; then
    exit 0
fi

# If no patch has been generated, clean up the file stub and exit
if [ ! -s "$PATCH" ] ; then
    echo "No code style issues found"
    exit 0
fi

exit 1
