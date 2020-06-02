#!/bin/bash -e

ADK_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../"

# shellcheck source=Tools/
source "$ADK_ROOT/Tools/download.sh"

usage()
{
    echo "A tool to check and fix code style in shell scripts."
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "OPTIONS:"
    echo "-f  - Lint the provided file. If no option is provided all files will be linted."
    echo "-d  - Lint the files in the specified directory."
    echo ""
    echo "EXAMPLES: "
    echo "$0 -f <file_name>          # Lint the specified file"
    echo "$0 -f <file_name> -f <file2> # Lint the two specified files"
    echo "$0 -d <dir_name>           # Lint files in the specified directory"
    echo "$0                         # Lint all files in this repository"
    exit 1
}

SHELLCHECK=shellcheck
SEARCH_PATH=$ADK_ROOT
PARALLEL=8
SHELL_FILES=()

while getopts "hf:d:" opt; do
    case ${opt} in
        f ) SHELL_FILES+=( "$OPTARG" );;
        d ) SEARCH_PATH="$OPTARG";;
        h ) usage;;
        \? ) usage;;
    esac
done

# Install
SHELLCHECK_VERSION="0.7.0"
if ! $SHELLCHECK --version | grep -q $SHELLCHECK_VERSION; then
    echo "Shellcheck version incorrect. Installing..."

    SHELLCHECK_BIN_FILE=
    DEST=
    case "$(uname)" in
        "Darwin")
            SHELLCHECK_BIN_FILE="shellcheck-v$SHELLCHECK_VERSION.darwin.x86_64"
            DEST="/usr/local/bin"
            SUDO=
            ;;
        "Linux")
            SHELLCHECK_BIN_FILE="shellcheck-v$SHELLCHECK_VERSION.linux.x86_64"
            DEST="/usr/bin"
            SUDO="sudo"
            ;;
        *)
            echo "Unsupported system"
            exit 1
            ;;
    esac

    download shellcheck.tar.xz \
          "https://github.com/koalaman/shellcheck/releases/download/v$SHELLCHECK_VERSION/$SHELLCHECK_BIN_FILE.tar.xz" \
        && tar -xvf "shellcheck.tar.xz" \
        && chmod 755 "shellcheck-v$SHELLCHECK_VERSION/shellcheck" \
        && "$SUDO" cp -f "shellcheck-v$SHELLCHECK_VERSION/shellcheck" "$DEST/$SHELLCHECK" \
        && rm -rf shellcheck*
fi

if [ ${#SHELL_FILES[@]} -gt 0 ]; then
    for file in "${SHELL_FILES[@]}"; do
        if [[ ! -f "$file" ]]; then
            echo "Skipping invalid file '$file' (does not exist)"
            continue
        fi
    done
else
    echo "Looking for files in $SEARCH_PATH directory..."
    while IFS=  read -r -d $'\0'; do
        SHELL_FILES+=("$REPLY")
    done < <(find "$SEARCH_PATH" -type f \
        \( \
            -perm -u=x \
            -o -perm -g=x \
            -o -perm -o=x \
        \) \
        -not -path "*\.tmp*" \
        -not -path "*External*" \
        -exec bash -c 'grep -r "^#!.*\/bin\/bash" $1 1> /dev/null' _ {} \; \
        -print0 \
        -o -name "*.sh" \
        -not -path "*\.tmp*" \
        -print0
    )
fi

# Exit early if no files are going to be checked.
if [[ ${#SHELL_FILES[@]} -eq 0 ]]; then
    echo "No files to lint!"
    usage
fi

printf "%s\0" "${SHELL_FILES[@]}" \
    | xargs -t -0 -P ${PARALLEL} -I{} -- \
        $SHELLCHECK \
        --external-sources \
        --exclude=SC1091 \
        --shell=bash \
        {}
