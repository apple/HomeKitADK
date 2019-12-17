#!/bin/bash -e

set -eu -o pipefail

# Get script directory: https://stackoverflow.com/a/246128/151706
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

# Set environment variables.
export ROOT="${DIR}"
if [ -z "${CACHE:-}" ]; then
    export CACHE="${ROOT}/.cache"
fi
export SSH_TMP="${ROOT}/.ssh_temp"

export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

IDENTITY_FILE="${SSH_TMP}/raspisetup"
SSH_OPTS=(-i "$IDENTITY_FILE" -o 'StrictHostKeyChecking=no' -o 'UserKnownHostsFile=/dev/null' -o 'ConnectTimeout=30' -o 'ServerAliveInterval=10000')


########################################################################################################################

do_sleep() {
    set +x
    COUNT=${1}
    sp="/-\|"
    echo -n '      '
    while [[ ${COUNT} -gt -1 ]]; do
       printf '\r%.1s %3d' "$sp" "$COUNT"
       sp=${sp#?}${sp%???}
       COUNT=$((COUNT - 1))
       sleep 1
    done
    printf "\r     \n"
    set -x
}

################################################################################
# Download function.
################################################################################
# $1: file
# $2: url
# $3: SHA-256 hash.
download() {
    mkdir -p "${CACHE}"
    local FILE="${CACHE}/$1"
    (
        local lockFile="${FILE}.lock"
        local uuid
        uuid="$(uuidgen)"
        trap '[ "$(cat "'"${lockFile}"'")" == "'"${uuid}"'" ] && rm -f "'"${lockFile}"'"' ERR EXIT
        while ! ( set -C; echo "${uuid}" > "${lockFile}" ) 2>/dev/null; do echo -n "." && sleep 0.5; done

        if [ -f "${FILE}" ]; then
            (echo "$3 *${FILE}" | shasum -a 256 -cw) || rm "${FILE}"
        fi
        if [ ! -f "${FILE}" ]; then
            # URL encode: http://stackoverflow.com/a/7506695
            local url=$2

            # Download & check hash.
            # This is known to abort with `curl: (56) LibreSSL SSL_read: SSL_ERROR_SYSCALL, errno 54`
            # when multiple downloads are started in quick succession. Therefore, retry loop.
            local retryBackoff=0
            local i=0
            while :; do
                if (( ++i > 10 )); then
                    echo "${1} download failed too often."
                    false
                fi
                if (( !retryBackoff )); then
                    retryBackoff=1
                else
                    echo "Retrying ${1} download in ${retryBackoff}s."
                    sleep "${retryBackoff}"
                    retryBackoff=$(( retryBackoff * 2 ))
                    maximumBackoff=1800
                    if (( retryBackoff >= maximumBackoff )); then
                        retryBackoff=${maximumBackoff}
                    fi
                fi

                if command -v curl > /dev/null; then
                    local command=(
                        "curl" "-L" "${url}" "-o" "${FILE}")
                    if command -v caffeinate >/dev/null; then
                        caffeinate -ims "${command[@]}" || continue
                    else
                        "${command[@]}" || continue
                    fi
                elif command -v wget > /dev/null; then
                    wget -O "${FILE}" "${url}" || continue
                else
                    echo -e "\x1B[31m/!\ No download tool installed. Please install curl or wget.\x1B[0m"
                    false
                fi
                break
            done
            echo "$3 *${FILE}" | shasum -a 256 -cw
        fi
    )
}

# Runs a command on the Raspi.
# $1: Hostname.
# $2: Password.
deploy_ssh_key() {
    mkdir -p "${SSH_TMP}"
    ssh-keygen -t rsa -N '' -f "${IDENTITY_FILE}" -C raspi@setup.local

    expect <<EOF
    set timeout -1
    spawn ssh-copy-id ${SSH_OPTS[@]} -f pi@${1}.local
    expect "password:"
    send "${2}\n"
    expect eof
    lassign [wait] pid spawnID osError value
    exit \$value
EOF
}


# $1: File name / folder name.
# $2: Hostname of Raspberry Pi.
# $3: Password for user account `pi`.
deploy() {
    local RASPI=${2}
    expect <<EOF
    set timeout -1
    spawn scp ${SSH_OPTS[@]} -r {${1}} pi@${RASPI}.local:~
    expect eof
    lassign [wait] pid spawnID osError value
    exit \$value
EOF
}

# Runs a command on the Raspi.
# $1: Hostname.
# $2: Password.
# $3: Command.
RaspiExec() {
    expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} "pi@${1}" {${3}}
    expect eof
    lassign [wait] pid spawnID osError value
    exit \$value
EOF
}

# $1: Hostname of Raspberry Pi.
# $2: Password for user account `pi`.
reboot_pi() {
    local RASPI=${1}
    expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} pi@${RASPI}.local
    expect "pi@${RASPI}:"
    send "sudo reboot\n"
    expect eof
EOF
    do_sleep 60
}

setup_sd_card() {
    local CONFIRM
    local SD_CARD_DISK

    # Find disk name.
    echo "================================================================================"
    echo "Please power off Raspberry Pi and remove SD card."
    read -r -s -p "Press [return] to continue."
    echo
    echo "================================================================================"
    diskutil list
    echo "================================================================================"
    echo "Please insert Raspberry Pi SD card into host."
    read -r -s -p "Press [return] to continue."
    echo
    echo "================================================================================"
    sleep 10
    diskutil list
    echo "================================================================================"
    read -r -p "Please type Raspberry Pi SD card device name: /dev/disk" SD_CARD_DISK
    read -r -s -p "Erase this disk: \`/dev/disk$SD_CARD_DISK\`? [Y/N]" -n 1 CONFIRM
    echo
    [ "$CONFIRM" == "y" ] || [ "$CONFIRM" == "Y" ]
    echo "================================================================================"

    # Erase disk image.
    set -x
    diskutil unmountDisk "/dev/disk$SD_CARD_DISK"
    sudo diskutil eraseDisk FAT32 RASPBIAN MBRFormat "/dev/disk$SD_CARD_DISK"
    diskutil unmountDisk "/dev/disk$SD_CARD_DISK"
    sleep 5

    # Get disk image.
    # https://www.raspberrypi.org/downloads/raspbian/
    local URL_BASE="http://director.downloads.raspberrypi.org/raspbian_lite/images"
    local URL_SUB="raspbian_lite-2019-07-12"
    local VERSION="2019-07-10-raspbian-buster"
    echo "Setting up SD card: /dev/disk$SD_CARD_DISK"
    download  "${VERSION}-lite.zip" "${URL_BASE}/${URL_SUB}/${VERSION}-lite.zip" 9e5cf24ce483bb96e7736ea75ca422e3560e7b455eee63dd28f66fa1825db70e
    unzip "${CACHE}/${VERSION}-lite.zip" -d .tmp

    # Deploy disk image.
    echo "(Check \`dd\` progress with ^T)"
    sudo dd bs=1m if=".tmp/${VERSION}-lite.img" of="/dev/rdisk$SD_CARD_DISK"
    rm ".tmp/${VERSION}-lite.img"
    sleep 5
    set +x

    # Enable SSH.
    touch /Volumes/boot/ssh

    # Disable expand filesystem on boot.
    perl -i -p0e 's/ init=\/usr\/lib\/raspi-config\/init_resize\.sh//gs' "/Volumes/boot/cmdline.txt"

    sudo diskutil eject "/dev/rdisk$SD_CARD_DISK"
    sleep 5
}

# $1: current hostname of Raspberry Pi.
# $2: current password of Raspberry Pi.
# $3: total size to be used on disk in MB.
setup_raspberry_pi() {
    local RASPI="${1}"
    local PW="${2}"
    local SIZE_MEGABYTES="${3}"

    # Expand file system.
    # See /usr/bin/raspi-config.
    echo "Resizing file system to use up to: ${SIZE_MEGABYTES} MB"
    rootPart="$(RaspiExec "${RASPI}.local" "${PW}" \
        "mount | sed -n 's|^/dev/\(.*\) on / .*|\1|p'" | tail -n 1)"
    rootPart="${rootPart/$'\r'/}"
    partNum="${rootPart#mmcblk0p}"
    if [ "${partNum}" == "${rootPart}" ]; then
        echo "Unexpected error while partitioning (1): ${partNum} == ${rootPart}"
        false
    fi
    if [ "${partNum}" != "2" ]; then
        echo "Unexpected error while partitioning (2): ${partNum} != 2"
        false
    fi
    lastPartNum="$(RaspiExec "${RASPI}.local" "${PW}" \
        "sudo parted /dev/mmcblk0 -ms unit s p | tail -n 1 | cut -f 1 -d:" | tail -n 1)"
    lastPartNum="${lastPartNum/$'\r'/}"
    if [ "${lastPartNum}" != "${partNum}" ]; then
        echo "Unexpected error while partitioning (3): ${lastPartNum} != ${partNum}"
        false
    fi

    cmd="sudo parted /dev/mmcblk0 -ms unit s p | grep \"^/dev/mmcblk0\" | cut -f 4 -d: | sed 's/[^0-9]//g'"
    sectorSize="$(RaspiExec "${RASPI}.local" "${PW}" "${cmd}" | tail -n 1)"
    sectorSize="${sectorSize/$'\r'/}"
    # all sizes in sectors:
    cmd="sudo parted /dev/mmcblk0 -ms unit s p | grep \"^${partNum}\" | cut -f 2 -d: | sed 's/[^0-9]//g'"
    partStart="$(RaspiExec "${RASPI}.local" "${PW}" "${cmd}" | tail -n 1)"
    partStart="${partStart/$'\r'/}"
    diskSize=$(((SIZE_MEGABYTES * 1000000) / 512)) # modulo size is wasted
    echo "Size to be used: ${diskSize}s"
    partSize=$((diskSize - partStart))
    echo "Data partition size: ${partSize}s"
    if (( partSize < 0 )); then
        echo "Requested disk size to small: ${SIZE} MB. Must be greater than $(( partStart * sectorSize ))"
        false
    fi
    RaspiExec "${RASPI}.local" "${PW}" \
        "sudo fdisk /dev/mmcblk0 <<<\$'p\nd\n${partNum}\nn\np\n${partNum}\n${partStart}\n+${partSize}\np\nw\n'" || true
    reboot_pi "${RASPI}" "${PW}"
    RaspiExec "${RASPI}.local" "${PW}" "sudo resize2fs /dev/${rootPart}"
    echo "File system resized."
    reboot_pi "${RASPI}" "${PW}"

    # set date
    currentTime="$(date -u)"
    echo "${currentTime}"

    expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} pi@${RASPI}.local
    expect "pi@${RASPI}:"

    send "sudo date -u -s '${currentTime}'\n"
    expect "pi@${RASPI}:"

    send "sudo raspi-config\n"

    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\r"
    expect "Would you like the camera interface to be enabled?"
    send "\x1B\[D"
    send "\r"
    expect "The camera interface is enabled"
    send "\r"

    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "Would you like the SPI interface to be enabled?"
    send "\x1B\[D"
    send "\r"
    expect "The SPI interface is enabled"
    send "\r"

    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "Would you like the ARM I2C interface to be enabled?"
    send "\x1B\[D"
    send "\r"
    expect "The ARM I2C interface is enabled"
    send "\r"

    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "Would you like a login shell to be accessible"
    send "\r"
    expect "The serial login shell is enabled"
    send "\r"

    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\x1B\[B"
    send "\x1B\[B"
    send "\r"
    expect "How much memory (MB) should the GPU have?  e.g. 16/32/64/128/256"
    send "\010\010\010"
    send "256"
    send "\r"

    expect "Raspberry Pi Software Configuration Tool (raspi-config)"
    send "\t"
    send "\t"
    send "\r"
    expect "Would you like to reboot now?"
    send "\r"
    expect eof
EOF
    do_sleep 60

    expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} pi@${RASPI}.local
    expect "pi@${RASPI}:"

    send "echo -en "
    send "'--- /etc/modules\\\n'"
    send "'+++ /etc/modules\\\n'"
    send "'@@ -4,3 +4,4 @@\\\n'"
    send "' # at boot time, one per line. Lines beginning with \"#\" are ignored.\\\n'"
    send "' \\\n'"
    send "' i2c-dev\\\n'"
    send "'+bcm2835-v4l2\\\n'"
    send " | sudo patch -p0 -d /\n"
    expect "pi@${RASPI}:"
    send "sudo reboot\n"
    expect eof
EOF
    do_sleep 60
}

# $1: current hostname of Raspberry Pi.
# $2: new hostname of Raspberry Pi.
# $3: new password of Raspberry Pi.
change_hostname_and_password() {
    local CURRENT_HOSTNAME="${1}"
    local NEW_HOSTNAME="${2}"
    local NEW_PASSWORD="${3}"
    export LC_ALL=en_GB.UTF-8
    export LANG=en_GB.UTF-8
    expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} pi@${CURRENT_HOSTNAME}.local
    expect "pi@${CURRENT_HOSTNAME}:"
    send "sudo su\n"
    expect "root@${CURRENT_HOSTNAME}:/home/pi#"
    send "sed -i s/${CURRENT_HOSTNAME}/${NEW_HOSTNAME}/ /etc/hostname\n"
    expect "root@${CURRENT_HOSTNAME}:/home/pi#"
    send "sed -i \"s/127.0.1.1.*${CURRENT_HOSTNAME}/127.0.1.1\\\t${NEW_HOSTNAME}/g\" /etc/hosts\n"
    expect "root@${CURRENT_HOSTNAME}:/home/pi#"
    send "rm /etc/ssh/ssh_host_*\n"
    expect "root@${CURRENT_HOSTNAME}:/home/pi#"
    send "dpkg-reconfigure openssh-server\n"
    expect "root@${CURRENT_HOSTNAME}:/home/pi#"
    send "passwd pi\n"
    expect "New password:"
    send "${NEW_PASSWORD}\n"
    expect "Retype new password:"
    send "${NEW_PASSWORD}\n"
    expect "passwd: password updated successfully"
    expect "root@${CURRENT_HOSTNAME}:/home/pi#"
    send "reboot\n"
    expect eof
EOF
    do_sleep 60
}

create_image() {
    local IMG_NAME="${1}"
    local DISK_SIZE="${2}"
    local CONFIRM
    local SD_CARD_DISK

    # Find disk name.
    echo "================================================================================"
    echo "Please power off Raspberry Pi and remove SD card."
    read -r -s -p "Press [return] to continue."
    echo
    echo "================================================================================"
    diskutil list
    echo "================================================================================"
    echo "Please insert Raspberry Pi SD card into host."
    read -r -s -p "Press [return] to continue."
    echo
    echo "================================================================================"
    sleep 10
    diskutil list
    echo "================================================================================"
    read -r -p "Please type Raspberry Pi SD card device name: /dev/disk" SD_CARD_DISK
    read -r -s -p "Create image from this disk: \`/dev/disk$SD_CARD_DISK\`? [Y/N]" -n 1 CONFIRM
    echo
    [ "$CONFIRM" == "y" ] || [ "$CONFIRM" == "Y" ]
    echo "================================================================================"

    set -x
    diskutil unmountDisk "/dev/disk$SD_CARD_DISK"
    sleep 5

    # Create disk image file.
    echo "(Check \`dd\` progress with ^T)"
    sudo dd bs=1m count="${DISK_SIZE}" if="/dev/rdisk$SD_CARD_DISK" of="${IMG_NAME}"
    sleep 5
    set +x

    sudo diskutil eject "/dev/rdisk$SD_CARD_DISK"
    sleep 5
}

########################################################################################################################

# Prepare temporary folder.
cd "${ROOT}"
rm -rf .tmp
mkdir -p .tmp

if [ "${SETUP_SKIP_SD:-0}" != "1" ]; then
    setup_sd_card

    echo "================================================================================"
    echo "1. Put SD card into Raspberry Pi."
    echo "2. Connect Raspberry Pi to host via Ethernet / USB."
    echo "3. Power Raspberry Pi."
    read -r -s -p "Press [return] to continue."
    echo
    echo "================================================================================"
    do_sleep 60
fi

PI_HOSTNAME="raspberrypi"
PI_PASSWORD="${PASSWORD:-raspberry}"
PI_COUNTRY="${PI_COUNTRY:-}"
SD_SIZE_MB="3200"

if [ "${SETUP_SKIP_BASE_SETUP:-0}" != "1" ]; then
    if [ -z ${SETUP_HOSTNAME+x} ]; then
        R="$(head /dev/urandom | LC_ALL=C tr -cd '0-9' | head -c 4)"
        SETUP_HOSTNAME="raspi-${R}"
    fi

    echo "SETUP_HOSTNAME: ${SETUP_HOSTNAME}"
    SETUP_PASSWORD="${SETUP_HOSTNAME}"

    deploy_ssh_key "${PI_HOSTNAME}" "${PI_PASSWORD}"
    change_hostname_and_password "${PI_HOSTNAME}" "${SETUP_HOSTNAME}" "${SETUP_PASSWORD}"
    setup_raspberry_pi "${SETUP_HOSTNAME}" "${SETUP_PASSWORD}" "${SD_SIZE_MB}"

    echo "================================================================================"
    echo "Raspberry Pi base setup complete."
    echo "================================================================================"
else
    SETUP_HOSTNAME="${SETUP_HOSTNAME:-${PI_HOSTNAME}}"
    SETUP_PASSWORD="${SETUP_PASSWORD:-${SETUP_HOSTNAME}}"
fi

# Install packages.
#
# To continue an in-progress installation without starting from a clean SD card:
# - SETUP_SKIP_SD=1 SETUP_SKIP_BASE_SETUP=1 SETUP_HOSTNAME=raspi-#### SETUP_PASSWORD=raspi-#### ./RaspiSetup
#
# List of current dependency packages and their usage:
#   Network         hostapd, dnsmasq, libavahi-client-dev
#   USB             libusb-1.0, libusb-dev
#   QR code         qrencode
#   Audio           libopus-dev, libasound2-dev
#   Camera          libraspberrypi-dev, raspberrypi-kernel-headers
#   GPIO            wiringpi
#   Cypto           libssl-dev
#   Build           clang, debhelper
#   Database        libsqlite3-dev
if [ "${SETUP_SKIP_PACKAGES:-0}" != "1" ]; then
    DEP_PKGS="libnss-mdns \
        hostapd \
        dnsmasq \
        libavahi-client-dev \
        libusb-1.0 \
        libusb-dev \
        qrencode \
        libopus-dev \
        libasound2-dev \
        wiringpi \
        libssl-dev \
        automake \
        debhelper \
        libsqlite3-dev \
        libraspberrypi-dev \
        raspberrypi-kernel-headers "

    BLD_PKGS="clang \
        cmake \
        git \
        valgrind \
        vim \
        tmux "

    echo "================================================================================"
    echo "Please connect the Raspberry Pi to the internet via wifi or by sharing your"
    echo "internet connection."
    echo "On Mac: System Preferences > Sharing > Internet Sharing"
    echo
    read -r -s -p "Press [return] to continue."
    echo

    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo reboot\n"
        expect eof
EOF
    echo
    do_sleep 60

    echo
    echo "================================================================================"
    echo "Installing dependencies"
    echo "================================================================================"

    echo
    echo "================================================================================"
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo apt-get update\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo apt-get install -y ${DEP_PKGS} ${BLD_PKGS}\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF


    # install faac from source
    echo "================================================================================"
    echo "Installing faac."
    echo "================================================================================"
    VERSION="1.29.9.2-2"
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo perl -i -p0e 's/#deb-src/deb-src/gs' /etc/apt/sources.list\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo apt-get update\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "mkdir faac-${VERSION}\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "cd faac-${VERSION}\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "apt-get -b source libfaac0 faac\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo dpkg -i libfaac0_${VERSION}_armhf.deb libfaac-dev_${VERSION}_armhf.deb faac_${VERSION}_armhf.deb\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "cd ..\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "rm -r faac-${VERSION}\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF

    # Install libnfc. Dependency for accessory setup programmable NFC tag.
    echo "================================================================================"
    echo "Installing libnfc."
    echo "================================================================================"
    # https://github.com/nfc-tools/libnfc/releases
    VERSION="1.7.1"
    download \
        "libnfc-${VERSION}.tar.bz2" \
        "https://github.com/nfc-tools/libnfc/releases/download/libnfc-${VERSION}/libnfc-${VERSION}.tar.bz2" \
        945e74d8e27683f9b8a6f6e529557b305d120df347a960a6a7ead6cb388f4072
    deploy "${CACHE}/libnfc-${VERSION}.tar.bz2" "${SETUP_HOSTNAME}" "${SETUP_PASSWORD}"
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "tar xjf libnfc-${VERSION}.tar.bz2\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "rm libnfc-${VERSION}.tar.bz2\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "cd libnfc-${VERSION}\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "./configure --sysconfdir=/etc --prefix=/usr\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "make -j4\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo make install\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "cd ..\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "rm -r libnfc-${VERSION}\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF

    # Install mDNSResponder. Dependency for Service Discovery.
    echo "================================================================================"
    echo "Installing mDNSResponder."
    echo "================================================================================"
    # https://opensource.apple.com
    VERSION="878.260.1"
    download \
        "mDNSResponder-${VERSION}.tar.gz" \
        "https://opensource.apple.com/tarballs/mDNSResponder/mDNSResponder-${VERSION}.tar.gz" \
        3cc71582e8eee469c2de8ecae1d769e7f32b3468dfb7f2ca77f1dee1f30a7d1e
    deploy "${CACHE}/mDNSResponder-${VERSION}.tar.gz" "${SETUP_HOSTNAME}" "${SETUP_PASSWORD}"
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "tar xzf mDNSResponder-${VERSION}.tar.gz\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "rm mDNSResponder-${VERSION}.tar.gz\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "make -C mDNSResponder-${VERSION}/mDNSPosix os=linux\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo make -C mDNSResponder-${VERSION}/mDNSPosix install os=linux\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "rm -r mDNSResponder-${VERSION}\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF

    echo "================================================================================"
    echo "Installing OpenMAX IL client library."
    echo "================================================================================"
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "cd /opt/vc/src/hello_pi/\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "make -C libs/ilclient\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF


    echo "================================================================================"
    echo "Creating host-local service."
    echo "================================================================================"
    echo
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "cat <<EOF | sudo tee /etc/systemd/system/host-local.service\n"
        send "\[Unit\]\n"
        send "Description=Hostname at local\n"
        send "After=dhcpcd.service\n"
        send "\n"
        send "\[Service\]\n"
        send "ExecStart=/usr/bin/dns-sd -R RASPI _hostname._tcp local 65335\n"
        send "Restart=on-failure\n"
        send "\n"
        send "\[Install\]\n"
        send "WantedBy=multi-user.target\n"
        send "\n"
        send "EOF\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF


    echo "================================================================================"
    echo "Configuring DHCP."
    echo "================================================================================"
    echo
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo perl -i -p0e 's/#option ntp_servers/option ntp_servers/gs' /etc/dhcpcd.conf\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF

    echo "================================================================================"
    echo "Configuring mdns service."
    echo "================================================================================"
    echo
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "cat <<EOF | sudo tee /usr/share/dhcpcd/hooks/99-restart-mdns\n"
        send "restart_mdns()\n"
        send "{\n"
        send "    systemctl restart mdns.service\n"
        send "}\n"
        send "\n"
        send "if \\\\\\\$if_up; then\n"
        send "    restart_mdns\n"
        send "fi\n"
        send "\n"
        send "EOF\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF

    # Configure Raspbian.
    echo "================================================================================"
    echo "Configuring Raspbian."
    echo "================================================================================"
    echo
    expect <<EOF
        set timeout -1
        spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo rm -f /etc/systemd/system/dhcpcd.service.d/wait.conf\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo systemctl enable host-local.service\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo systemctl mask avahi-daemon.service\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "sudo systemctl mask avahi-daemon.socket\n"
        expect "pi@${SETUP_HOSTNAME}:"
        send "exit\n"
        expect eof
EOF

fi
# END Install packages

# Collect regulatory domain.
while [ -z "${PI_COUNTRY}" ]; do
    echo "Please enter the country in which the Raspberry Pi will be used ([return] to show list of country codes)."
    read -r -p "Country code: " PI_COUNTRY
    if [ -z "${PI_COUNTRY}" ]; then
        # Print all country codes.
        i=0
        colWidth=43
        colsPerLine=$(( $(tput cols) / (colWidth + 3) ))
        if (( !colsPerLine )); then
            colsPerLine=1
        fi
        numLines=$(grep -c -E '^[A-Z]{2}\t' "/usr/share/zoneinfo/iso3166.tab")
        linesPerCol=$(( (numLines + (colsPerLine - 1) ) / colsPerLine ))
        paddingLines=$(( linesPerCol * colsPerLine - numLines ))

        countries="$(grep -E '^[A-Z]{2}\t' "/usr/share/zoneinfo/iso3166.tab")"$'\n'
        for (( i=0; i<paddingLines; i++ )); do
            countries="${countries}"$'~~\t(padding)\n'
        done

        echo -n "${countries}" | \
            awk -F'\t' '{ printf "%d\t%s\n", (NR - 1) % '"${linesPerCol}"', $0 }' | \
            LC_ALL=C sort -n | \
            awk -F'\t' '
                NR > 1 && !((NR - 1) % '${colsPerLine}') { printf "\n" }
                $2 != "~~" { printf "\x1B[32m%s\x1B[0m %-'"${colWidth}"'s", $2, $3 }'
        echo
    elif [[ ! "${PI_COUNTRY}" =~ [A-Z]{2} ]]; then
        echo "Invalid country code."
        PI_COUNTRY=""
    elif ! grep -E '^'"${PI_COUNTRY}"'\t' "/usr/share/zoneinfo/iso3166.tab"; then
        echo "Unknown country code."
        PI_COUNTRY=""
    fi
done
echo

expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} pi@${SETUP_HOSTNAME}.local
    expect "pi@${SETUP_HOSTNAME}:"
    send "sudo su\n"
    expect "root@${SETUP_HOSTNAME}:/home/pi#"
    # Set country settings.
    send "cat <<EOF > '/etc/wpa_supplicant/wpa_supplicant.conf'\n"
    send "country=${PI_COUNTRY}\n"
    send "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n"
    send "update_config=1\n"
    send "EOF\n"
    expect "root@${SETUP_HOSTNAME}:/home/pi#"

    # Backup initial configuration file for wpa_supplicant.
    send "cp '/etc/wpa_supplicant/wpa_supplicant.conf' '/etc/wpa_supplicant/wpa_supplicant.conf.orig'\n"
    expect "root@${SETUP_HOSTNAME}:/home/pi#"

    # Unblock Wi-Fi if an image is deployed that disables Wi-Fi until country code is set.
    send "if \\[ -f '/run/wifi-country-unset' \\]; then rfkill unblock wifi; fi\n"
    expect "root@${SETUP_HOSTNAME}:/home/pi#"
    send "exit\n"
    expect "pi@${SETUP_HOSTNAME}:"
    send "exit\n"
    expect eof
EOF

echo "================================================================================"
read -r -p "Please enter hostname for Raspberry Pi ([return] to keep '${PI_HOSTNAME}'): " NEW_PI_HOSTNAME
NEW_PI_HOSTNAME="${NEW_PI_HOSTNAME:-${PI_HOSTNAME}}"
echo
while [[ ! ${NEW_PI_HOSTNAME} =~ ^[a-z0-9-]*$ ]]; do
    echo "A hostname may contain only ASCII letters 'a' through 'z' (case-insensitive),"
    echo "the digits '0' through '9', and the hyphen. Please try again."
    echo
    read -r -p "Please enter hostname for Raspberry Pi : " NEW_PI_HOSTNAME
    echo
done

read -r -s -p "Please enter desired password for user 'pi' ([return] to keep default): " NEW_PI_PASSWORD
NEW_PI_PASSWORD="${NEW_PI_PASSWORD:-${PI_PASSWORD}}"
echo
if [ -z "${NEW_PI_PASSWORD}" ] || [ "${NEW_PI_PASSWORD}" != "${PI_PASSWORD}" ]; then
    read -r -s -p "Please retype password: " NEW_PI_PASSWORD2
    echo
else
    NEW_PI_PASSWORD2="${PI_PASSWORD}"
fi
while [ "${NEW_PI_PASSWORD}" != "${NEW_PI_PASSWORD2}" ]; do
    echo "Sorry, empty password or passwords do not match, please try again."
    echo
    read -r -s -p "Please enter desired password for user 'pi': " NEW_PI_PASSWORD
    echo
    if [ -n "${NEW_PI_PASSWORD}" ]; then
        read -r -s -p "Please retype password: " NEW_PI_PASSWORD2
        echo
    else
        NEW_PI_PASSWORD2="not${NEW_PI_PASSWORD}"
    fi
done

# Complete setup.
echo "================================================================================"
echo "Rebooting Raspberry Pi."
change_hostname_and_password "${SETUP_HOSTNAME}" "${NEW_PI_HOSTNAME}" "${NEW_PI_PASSWORD}"

echo "================================================================================"
echo "Expand FS on next boot."
# See /usr/bin/raspi-config.
expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} pi@${NEW_PI_HOSTNAME}.local
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "ROOT_PART=\\\$(mount | sed -n 's|^/dev/\\\(.*\\\) on / .*|\\\1|p')\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "sudo perl -i -p0e 's/\\\\n/ init=\\\/usr\\\/lib\\\/raspi-config\\\/init_resize\\\.sh\\\\n/gs' '/boot/cmdline.txt'\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "cat <<EOF | sudo tee /etc/init.d/resize2fs_once\n"
    send "#!/bin/sh\n"
    send "### BEGIN INIT INFO\n"
    send "# Provides:          resize2fs_once\n"
    send "# Required-Start:\n"
    send "# Required-Stop:\n"
    send "# Default-Start: 3\n"
    send "# Default-Stop:\n"
    send "# Short-Description: Resize the root filesystem to fill partition\n"
    send "# Description:\n"
    send "### END INIT INFO\n"
    send "\n"
    send ". /lib/lsb/init-functions\n"
    send "\n"
    send "case \"\\\\\\\$1\" in\n"
    send "  start)\n"
    send "    log_daemon_msg \"Starting resize2fs_once\" &&\n"
    send "    resize2fs /dev/\\\$ROOT_PART &&\n"
    send "    update-rc.d resize2fs_once remove &&\n"
    send "    rm /etc/init.d/resize2fs_once &&\n"
    send "    log_end_msg \\\\\\\$?\n"
    send "    ;;\n"
    send "  *)\n"
    send "    echo \"Usage: \\\\\\\$0 start\" >&2\n"
    send "    exit 3\n"
    send "    ;;\n"
    send "esac\n"
    send "EOF\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "sudo chmod +x /etc/init.d/resize2fs_once\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "sudo update-rc.d resize2fs_once defaults\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "exit\n"
    expect eof
EOF

# Write raspbian package time stamp.
echo "================================================================================"
echo "Write raspbian package time stamp."
currentTime="$(date -u)"
expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} pi@${NEW_PI_HOSTNAME}.local
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "sudo su\n"
    expect "root@${NEW_PI_HOSTNAME}:"
    send "date --date='${currentTime}' -u --iso-8601 > /etc/raspbian-package-timestamp\n"
    expect "root@${NEW_PI_HOSTNAME}:"
    send "exit\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "exit\n"
    expect eof
EOF

# Delete history.
echo "================================================================================"
echo "Delete history."
expect <<EOF
    set timeout -1
    spawn ssh ${SSH_OPTS[@]} pi@${NEW_PI_HOSTNAME}.local
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "sudo su\n"
    expect "root@${NEW_PI_HOSTNAME}:"
    send "history -cw\n"
    expect "root@${NEW_PI_HOSTNAME}:"
    send "exit\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "rm -rf ~/.ssh\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "sudo rm /root/.bash_history\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "rm ~/.bash_history\n"
    expect "pi@${NEW_PI_HOSTNAME}:"
    send "history -cw && sudo halt\n"
    expect eof
EOF

rm -rf .tmp
rm -rf .ssh_tmp

echo "================================================================================"
echo "Setup complete."
echo "================================================================================"
echo "Please enter name for Raspberry Pi image file ([return] to skip creating image)."
read -r -p "File name: " imageFileName
if [[ -n "${imageFileName}" ]]; then
    create_image "${imageFileName}" "${SD_SIZE_MB}"
    echo "SD card image with size ${SD_SIZE_MB} MB written to file:"
    echo "${imageFileName}"
    echo "================================================================================"
else
    echo "Skipped image creation."
    echo "Power cycle Raspi, linux partition will be resized to full SD card."
    echo "================================================================================"
    echo "Log in with \`ssh pi@${NEW_PI_HOSTNAME}.local\` and the previously supplied password."
    echo "================================================================================"
fi


echo
echo "================================================================================"
echo "Docker Import"
echo "================================================================================"
echo
read -r -p "Do you want to import this image to docker (this step is necessary for building raspi binaries)? [Y/N] " CONFIRM
echo
if [[ $CONFIRM =~ ^[Yy]$ ]]; then
    echo "================================================================================"
    echo "Ensure SD card is inserted and power cycle Raspi."
    echo "If this is the first boot after FS expansion, wait 60 seconds then"
    echo "power cycle Raspi a second time."
    read -r -s -p "Press [return] to continue."
    echo

    ("${ROOT}/raspi_docker_import.sh" -n "${NEW_PI_HOSTNAME}" -p "$NEW_PI_PASSWORD" )

else
    echo "Skipped docker import."
    echo "================================================================================"
fi

echo
echo "Done!"
echo
