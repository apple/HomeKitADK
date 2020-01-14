#!/bin/bash -e

ADK_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../"

usage()
{
  echo "This script helps provision and install a given application on a connected device."
  echo ""
  echo "Usage: $0 [options]"
  echo ""
  echo "OPTIONS:"
  echo "-d  - [required] Device type. Possible value are: raspi"
  echo "-a  - [required] Path to the application to install"
  echo "-i  - [optional] install only, do not run the application (raspi only)"
  echo "-n  - [optional] raspberry pi host name"
  echo "-p  - [optional] raspberry pi password"
  echo "-l  - [optional] Install path on device. Default: \"~\""
  exit 1
}

DEVICE=
APPLICATION=
HOSTNAME=
PASSWORD=
INSTALL_ONLY=
INSTALL_PATH="~"
while getopts "hd:a:n:p:il:" opt; do
  case ${opt} in
    d ) DEVICE=$OPTARG
      ;;
    a ) APPLICATION=$OPTARG
      ;;
    n ) HOSTNAME=$OPTARG
      ;;
    p ) PASSWORD=$OPTARG
      ;;
    i ) INSTALL_ONLY=1
      ;;
    l ) INSTALL_PATH=$OPTARG
      ;;
    h ) usage
      ;;
    \? ) usage
      ;;
  esac
done


if [[ -z "$DEVICE" || -z "$APPLICATION" ]]; then
  usage
fi

if [[ ! -f "$APPLICATION" ]]; then
  echo "File path $APPLICATION doesn't exist"
  exit 1
fi

TMP_DIR="$HOME/tmp/"

setup() {
  mkdir -p "$TMP_DIR"

  # Install wget if not already installed
  if ! brew ls --versions wget > /dev/null; then
    brew install wget
  fi
}

makeTools() {
  # Make all the tools needed by provisioning script
  pushd "$ADK_ROOT" > /dev/null
  make tools
}

raspi() {
  # Clear any existing homekit store and create install path if necessary
  expect <<EOF
  set timeout -1
  spawn ssh -o ConnectTimeout=30 -o ServerAliveInterval=10000 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null pi@${HOSTNAME}.local
  expect "password: "
  send "${PASSWORD}\n"
  expect "pi@${HOSTNAME}:"
  send "rm -rf ${INSTALL_PATH}/.HomeKitStore\n"
  expect "pi@${HOSTNAME}:"
  send "mkdir -p ${INSTALL_PATH}\n"
  expect "pi@${HOSTNAME}:"
  send "exit\n"
  expect eof
EOF

  expect <<EOF
  set timeout -1
  spawn scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -r "$APPLICATION" pi@${HOSTNAME}.local:${INSTALL_PATH}/
  expect "password: "
  send "${PASSWORD}\n"
  expect eof
  lassign [wait] pid spawnID osError value
  exit \$value
EOF

  # Generate a provisioning file
  expect <<EOF
  spawn ${ADK_ROOT}/Tools/provision_raspi.sh --category 5 --setup-code 111-22-333 pi@${HOSTNAME}.local:${INSTALL_PATH}/.HomeKitStore
  expect "password: "
  send "${PASSWORD}\n"
  expect eof
EOF

  if [[ -z "$INSTALL_ONLY" ]]; then
    # Run it
    expect <<EOF
    set timeout -1
    spawn ssh -o ConnectTimeout=30 -o ServerAliveInterval=10000 -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null pi@${HOSTNAME}.local
    expect "password: "
    send "${PASSWORD}\n"
    expect "pi@${HOSTNAME}:"
    send "cd ${INSTALL_PATH}\n"
    expect "pi@${HOSTNAME}:"
    send "sudo ./$(basename "$APPLICATION")\n"
    expect eof
EOF
  fi

}

LC_DEVICE=$(echo "$DEVICE" | awk '{print tolower($0)}')

case "$LC_DEVICE" in
  "raspi")
    makeTools
    raspi
    ;;
  *)
    echo "Device $DEVICE is not supported."
    exit 1
    ;;
esac
