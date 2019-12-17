#!/bin/bash -e

set -eu -o pipefail

usage()
{
  echo "This script is used to import a RaspberryPi rootfs from an active device"
  echo "into docker."
  echo ""
  echo "Usage: $0 -n HOSTNAME [-i SSH_IDENTITY | -p SSH_PASSWORD]"
  echo ""
  echo "OPTIONS:"
  echo "-n  - [required] raspberry pi host name"
  echo "-p  - [optional] raspberry pi password"
  echo "-i  - [optional] ssh identity file"
  echo ""
  exit 1
}


HOSTNAME=
PASSWORD=
IDENTITY_FILE=
while getopts "hn:i:p:" opt; do
  case ${opt} in
    n ) HOSTNAME=$OPTARG
      ;;
    p ) PASSWORD=$OPTARG
      ;;
    i ) IDENTITY_FILE=$OPTARG
      ;;
    h ) usage
      ;;
    \? ) usage
      ;;
  esac
done

if [[ -z "$HOSTNAME" ]]; then
  usage
fi

if [[ -z "$PASSWORD" && -z "$IDENTITY_FILE" ]]; then
  usage
fi

if [[ -n "$PASSWORD" ]]; then
  SSH_OPTS=(-o 'StrictHostKeyChecking=no' -o 'UserKnownHostsFile=/dev/null' -o 'ConnectTimeout=30' -o 'ServerAliveInterval=10000')
else
  SSH_OPTS=(-i "$IDENTITY_FILE" -o 'StrictHostKeyChecking=no' -o 'UserKnownHostsFile=/dev/null' -o 'ConnectTimeout=30' -o 'ServerAliveInterval=10000')
fi


# create rootfs tarball
expect <<EOF
  set timeout -1
  spawn ssh ${SSH_OPTS[@]} pi@${HOSTNAME}.local
  expect {
    "password: " {
      send "${PASSWORD}\n"
      exp_continue
    }
    "pi@${HOSTNAME}:"
  }
  send "sudo su\n"
  expect "root@${HOSTNAME}:"
  send "mkdir /temp\n"
  expect "root@${HOSTNAME}:"
  send "tar --numeric-owner --exclude=/proc --exclude=/sys --exclude=/temp -cf /temp/raspi-base.tar / \n"
  expect "root@${HOSTNAME}:"
  send "exit\n"
  expect "pi@${HOSTNAME}:"
  send "exit\n"
  expect eof
EOF

mkdir -p .tmp

# copy tar file to local disk
expect <<EOF
  set timeout -1
  spawn scp ${SSH_OPTS[@]} pi@${HOSTNAME}.local:/temp/raspi-base.tar .tmp/
  expect {
    "password: " {
      send "${PASSWORD}\n"
      exp_continue
    }
    eof
  }
  lassign [wait] pid spawnID osError value
  exit \$value
EOF

set -x
# import tar file to docker
docker import .tmp/raspi-base.tar dev-test/raspiadk-base
rm -rf .tmp
set +x

expect <<EOF
  set timeout -1
  spawn ssh ${SSH_OPTS[@]}  pi@${HOSTNAME}.local
  expect {
    "password: " {
      send "${PASSWORD}\n"
      exp_continue
    }
    "pi@${HOSTNAME}:"
  }
  send "sudo rm -rf /temp \n"
  expect "pi@${HOSTNAME}:"
  send "exit\n"
  expect eof
EOF

