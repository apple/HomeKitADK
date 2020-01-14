#!/bin/bash -e

ADK_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../"

# shellcheck source=Tools/
source "$ADK_ROOT/Tools/download.sh"

# Install shellcheck
SHELLCHECK_VERSION="0.7.0"
if ! shellcheck --version | grep -q $SHELLCHECK_VERSION; then
  echo "Shellcheck version incorrect. Installing..."

  SHELLCHECK_BIN_FILE=
  DEST=
  case "$(uname)" in
    "Darwin")
      SHELLCHECK_BIN_FILE="shellcheck-stable.darwin.x86_64"
      DEST="/usr/local/bin"
      SUDO=
      ;;
    "Linux")
      SHELLCHECK_BIN_FILE="shellcheck-stable.linux.x86_64"
      DEST="/usr/bin"
      SUDO="sudo"
      ;;
    *)
      echo "Unsupported system"
      exit 1
      ;;
  esac

  download shellcheck.tar.xz "https://shellcheck.storage.googleapis.com/$SHELLCHECK_BIN_FILE.tar.xz" \
    && tar -xvf "shellcheck.tar.xz" \
    && chmod 755 shellcheck-stable/shellcheck \
    && "$SUDO" cp -f shellcheck-stable/shellcheck "$DEST/shellcheck" \
    && rm -rf shellcheck*
fi

find "$ADK_ROOT" -type f \
   \( -perm -u=x -o -perm -g=x -o -perm -o=x \) \
  -not -path "*\.tmp*" \
  -exec bash -c 'grep -r "^#!.*\/bin\/bash" $1 1> /dev/null' _ {} \; \
  -print0 \
  -o -name "*.sh" \
  -not -path "*\.tmp*" \
  -print0 \
| xargs -0 \
    shellcheck \
      --external-sources \
      --exclude=SC1091 \
      --shell=bash \
