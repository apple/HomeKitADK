#!/bin/bash -e

set -eu -o pipefail

ADK_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../"

fail() {
    echo "$1"
    echo 'Use Provision --help to display usage information.'
    false
}

if [[ -z "${1:-}" ]] || [[ "${1:-}" == "-h" ]] || [[ "${1:-}" == "--help" ]]; then
    echo 'HomeKit Accessory Setup Provisioning Tool'
    echo ''
    echo 'USAGE'
    echo '     Provision [OPTION]... [<Host>:]<Destination key-value store>'
    echo ''
    echo 'DESCRIPTION'
    echo '     This tool generates and provisions accessory setup information for HomeKit.'
    echo ''
    echo '     EACH ACCESSORY NEEDS TO BE PROVISIONED WITH UNIQUE ACCESSORY SETUP'
    echo '     INFORMATION BEFORE IT MAY BE USED.'
    echo ''
    echo '     Destination key-value store is the location of the key-value store, e.g.'
    echo '     the directory .HomeKitStore. If Host is specified, a remote shell program'
    echo '     is used as the transfer.'
    echo ''
    echo '     The --category, --display, and --nfc options must match the accessory'
    echo '     capabilities and configuration.'
    echo ''
    echo 'OPTIONS'
    echo '     The following options are available:'
    echo ''
    echo '     --display'
    echo '        Accessory is connected to a display that supports showing a setup code.'
    echo '        A random setup code is generated for each pairing attempt, so a fixed'
    echo '        setup code cannot be specified if this option is selected.'
    echo ''
    echo '     --nfc'
    echo '        Accessory has a programmable NFC tag.'
    echo ''
    echo '     --category <Category>'
    echo '        The accessory category.'
    echo ''
    echo '        An accessory with support for multiple categories should advertise the'
    echo '        primary category. An accessory for which a primary category cannot be'
    echo '        determined or the primary category isn'"'"'t among the well defined'
    echo '        categories falls in the Other category.'
    echo ''
    echo '        Well defined categories:'
    echo '          1  Other.'
    echo '          2  Bridges.'
    echo '          3  Fans.'
    echo '          4  Garage Door Openers.'
    echo '          5  Lighting.'
    echo '          6  Locks.'
    echo '          7  Outlets.'
    echo '          8  Switches.'
    echo '          9  Thermostats.'
    echo '         10  Sensors.'
    echo '         11  Security Systems.'
    echo '         12  Doors.'
    echo '         13  Windows.'
    echo '         14  Window Coverings.'
    echo '         15  Programmable Switches.'
    echo '         16  Range Extenders.'
    echo '         17  IP Cameras.'
    echo '         18  Video Doorbells.'
    echo '         19  Air Purifiers.'
    echo '         20  Heaters.'
    echo '         21  Air Conditioners.'
    echo '         22  Humidifiers.'
    echo '         23  Dehumidifiers.'
    echo '         28  Sprinklers.'
    echo '         29  Faucets.'
    echo '         30  Shower Systems.'
    echo ''
    echo '     --setup-code <Setup code>'
    echo '        Provisions accessory setup information that allows pairing using the'
    echo '        specified setup code (e.g. for development).'
    echo '        Format is XXX-XX-XXX with X being a digit from 0-9.'
    echo '        - Setup codes that only consist of a repeating digit are not allowed.'
    echo '        - 123-45-678 and 876-54-321 are not allowed.'
    echo '        If this option is not present, a random setup code is provisioned.'
    echo ''
    echo '     --setup-id <Setup ID>'
    echo '        Provisions accessory setup information using a specific setup ID.'
    echo '        Format is XXXX with X being a digit from 0-9 or a character from A-Z.'
    echo '        - Lowercase characters are not allowed.'
    echo '        If this option is not present, a random setup ID is provisioned.'
    echo ''
    echo '     --mfi-token <Software Token UUID> <Software Token>'
    echo '     --mfi-token preserve'
    echo '        Provisions information required for Software Authentication.'
    echo '        Format for <Software Token UUID> is XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX'
    echo '        with X being a digit from 0-9 or a character from A-Z.'
    echo '        Format for <Software Token> is base64.'
    echo '        The <Software Token UUID> and initial <Software Token> are deployed'
    echo '        to the key-value store.'
    echo '        Alternatively, if the preserve option is specified, a previously'
    echo '        provisioned Software Token will not be deleted.'
    echo ''
    echo '     -e'
    echo '        Specify the remote shell to use.'
    echo '        If this option is not present, ssh is used.'
    echo ''
    echo 'OUTPUT'
    echo '     The destination key-value store is provisioned with information that is'
    echo '     compatible with the provided HAP platform implementation. If that'
    echo '     implementation is modified, this tool may need to be modified as well.'
    echo '     It is left up to the accessory manufacturer to implement a production ready'
    echo '     provisioning concept.'
    echo ''
    echo '     Upon completion, information to be printed on labels is printed to stdout.'
    echo '     These labels must be affixed to the accessory and its packaging.'
    echo '     If an accessory is connected to a display, random setup information is used'
    echo '     for every pairing attempt, and no label information is generated.'
    echo ''
    echo '     If a QR code label is supported (no display or programmable NFC tag),'
    echo '     it is printed to stdout using ANSI characters (requires qrencode).'
    echo '     This QR code is to be used for development purposes only!'
    echo '     Please refer to Works with Apple HomeKit Identity Guidelines for more'
    echo '     details on identity guidelines for including QR codes on the accessory.'
    exit
fi

################################################################################
# Configuration.
################################################################################
accessorySetupGenerator=$(find "$ADK_ROOT"/Output -name AccessorySetupGenerator.OpenSSL)
if [[ -z "$accessorySetupGenerator" ]]; then
  echo "AccessorySetupGenerator tool isn't available. Please run \"make tools\" from ADK root."
  exit
fi

sdkDomainsFile="$ADK_ROOT/PAL/Raspi/HAPPlatformKeyValueStore+SDKDomains.h"

################################################################################
# Parse command line.
################################################################################
supportsDisplay=0
supportsProgrammableNFC=0
category=0
fixedSetupCode=""
fixedSetupID=""
preserveMFiToken=0
mfiTokenUUID=""
mfiToken=""
remoteShell=""
destination=""
host=""
keyValueStore=""
while (( $# )); do
    case "${1}" in
        "--display")
            shift
            if (( supportsDisplay )); then
                fail "--display specified multiple times."
            fi
            supportsDisplay=1
            ;;

        "--nfc")
            shift
            if (( supportsProgrammableNFC )); then
                fail "--nfc specified multiple times."
            fi
            supportsProgrammableNFC=1
            ;;

        "--category")
            shift
            if (( category )); then
                fail "--category specified multiple times."
            fi
            if (( $# < 1 )); then
                fail "--category specified without accessory category identifier."
            fi
            categoryID="${1}"
            shift
            if [ ! "${categoryID}" -eq "${categoryID}" ]; then
                fail "--category specified with malformed accessory category identifier."
            fi
            if (( ! categoryID )) || (( categoryID > 255 )); then
                fail "--category specified with out-of-range accessory category identifier."
            fi
            category=${categoryID}
            ;;

        "--setup-code")
            shift
            if [ "${fixedSetupCode}" != "" ]; then
                fail "--setup-code specified multiple times."
            fi
            if (( $# < 1 )); then
                fail "--setup-code specified without setup code."
            fi
            fixedSetupCode="${1}"
            shift
            ;;

        "--setup-id")
            shift
            if [ "${fixedSetupID}" != "" ]; then
                fail "--setup-id specified multiple times."
            fi
            if (( $# < 1 )); then
                fail "--setup-id specified without setup ID."
            fi
            fixedSetupID="${1}"
            shift
            ;;

        "--mfi-token")
            shift
            if [ "${mfiTokenUUID}" != "" ] || [ "${mfiToken}" != "" ] || [ "${preserveMFiToken}" != "0" ]; then
                fail "--mfi-token specified multiple times."
            fi
            if [ "${1:-}" == "preserve" ]; then
                preserveMFiToken=1
                shift
                continue
            fi
            if (( $# < 2 )); then
                fail "--mfi-token specified without Software Token UUID or Software Token."
            fi
            mfiTokenUUIDString="${1}"
            mfiTokenBase64String="${2}"
            shift 2
            mfiTokenUUIDRegex=""
            mfiTokenUUIDRegex="${mfiTokenUUIDRegex}([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})-"
            mfiTokenUUIDRegex="${mfiTokenUUIDRegex}([0-9A-F]{2})([0-9A-F]{2})-([0-9A-F]{2})([0-9A-F]{2})-"
            mfiTokenUUIDRegex="${mfiTokenUUIDRegex}([0-9A-F]{2})([0-9A-F]{2})-([0-9A-F]{2})([0-9A-F]{2})"
            mfiTokenUUIDRegex="${mfiTokenUUIDRegex}([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})"
            if [[ ! "${mfiTokenUUIDString}" =~ ${mfiTokenUUIDRegex} ]]; then
                fail "--mfi-token specified with malformed Software Token UUID."
            fi
            # Reversed mfiTokenUUID network byte order.
            mfiTokenUUID=""
            mfiTokenUUID="${mfiTokenUUID}${BASH_REMATCH[16]}${BASH_REMATCH[15]}${BASH_REMATCH[14]}${BASH_REMATCH[13]}"
            mfiTokenUUID="${mfiTokenUUID}${BASH_REMATCH[12]}${BASH_REMATCH[11]}${BASH_REMATCH[10]}${BASH_REMATCH[ 9]}"
            mfiTokenUUID="${mfiTokenUUID}${BASH_REMATCH[ 8]}${BASH_REMATCH[ 7]}${BASH_REMATCH[ 6]}${BASH_REMATCH[ 5]}"
            mfiTokenUUID="${mfiTokenUUID}${BASH_REMATCH[ 4]}${BASH_REMATCH[ 3]}${BASH_REMATCH[ 2]}${BASH_REMATCH[ 1]}"
            if ! mfiToken="$(base64 --decode <<< "${mfiTokenBase64String}" | xxd -ps -u | tr -d '\n')"; then
                fail "--mfi-token specified with malformed Software Token."
            fi
            ;;

        "-e")
            shift
            if [ "${remoteShell}" != "" ]; then
                fail "-e specified multiple times."
            fi
            if (( $# < 1 )); then
                fail "-e specified without remote shell."
            fi
            remoteShell="${1}"
            shift
            ;;

        *)
            if [ "${destination}" != "" ]; then
                fail "Too many arguments specified."
            fi
            destination="${1}"
            shift
            host="${destination%:*}"
            keyValueStore="${destination##*:}"
            ;;

    esac
done
if (( ! category )); then
    fail "--category not specified."
fi
if (( supportsDisplay )) && [ "${fixedSetupCode}" != "" ]; then
    fail "--setup-code cannot be specified if --display is selected."
fi
if [ "${keyValueStore}" == "" ]; then
    fail "Destination key-value store not specified."
fi
if [ "${destination}" == "${keyValueStore}" ] && [ "${remoteShell}" != "" ]; then
    fail "-e specified without Host."
fi
if [ "${destination}" != "${keyValueStore}" ]; then
    remoteShell="${remoteShell:-ssh}"
fi

################################################################################
# Generate accessory setup information.
################################################################################
flags=""
flags="${flags}"' --ip'
flags="${flags}"' --category '"${category}"
if [ "${fixedSetupCode}" != "" ]; then
    flags="${flags}"' --setup-code '"${fixedSetupCode}"
fi
if [ "${fixedSetupID}" != "" ]; then
    flags="${flags}"' --setup-id '"${fixedSetupID}"
fi

# shellcheck disable=SC2086
accessorySetup=$("${accessorySetupGenerator}" ${flags})
# shellcheck disable=SC2206
accessorySetup=(${accessorySetup})

accessorySetupVersion="${accessorySetup[0]}"
if [ "${accessorySetupVersion}" != "1" ]; then
    fail "Incompatible with Accessory Setup Generator."
fi

setupCode="${accessorySetup[1]}"
srpSalt="${accessorySetup[2]}"
srpVerifier="${accessorySetup[3]}"
setupID="${accessorySetup[4]}"
setupPayload="${accessorySetup[5]}"

################################################################################
# Provision accessory setup information.
################################################################################
provisioningDomainID="$(grep "#define kSDKKeyValueStoreDomain_Provisioning " "${sdkDomainsFile}")"
[[ "${provisioningDomainID}" =~ "(HAPPlatformKeyValueStoreDomain) "0x([0-9]+) ]]
provisioningDomain="${BASH_REMATCH[1]}"

setupInfoKeyID="$(grep "#define kSDKKeyValueStoreKey_Provisioning_SetupInfo " "${sdkDomainsFile}")"
[[ "${setupInfoKeyID}" =~ "(HAPPlatformKeyValueStoreKey) "0x([0-9]+) ]]
setupInfoKey="${BASH_REMATCH[1]}"

setupIDKeyID="$(grep "#define kSDKKeyValueStoreKey_Provisioning_SetupID " "${sdkDomainsFile}")"
[[ "${setupIDKeyID}" =~ "(HAPPlatformKeyValueStoreKey) "0x([0-9]+) ]]
setupIDKey="${BASH_REMATCH[1]}"

setupCodeKeyID="$(grep "#define kSDKKeyValueStoreKey_Provisioning_SetupCode " "${sdkDomainsFile}")"
[[ "${setupCodeKeyID}" =~ "(HAPPlatformKeyValueStoreKey) "0x([0-9]+) ]]
setupCodeKey="${BASH_REMATCH[1]}"

mfiTokenUUIDKeyID="$(grep "#define kSDKKeyValueStoreKey_Provisioning_MFiTokenUUID " "${sdkDomainsFile}")"
[[ "${mfiTokenUUIDKeyID}" =~ "(HAPPlatformKeyValueStoreKey) "0x([0-9]+) ]]
mfiTokenUUIDKey="${BASH_REMATCH[1]}"

mfiTokenKeyID="$(grep "#define kSDKKeyValueStoreKey_Provisioning_MFiToken " "${sdkDomainsFile}")"
[[ "${mfiTokenKeyID}" =~ "(HAPPlatformKeyValueStoreKey) "0x([0-9]+) ]]
mfiTokenKey="${BASH_REMATCH[1]}"

setupInfoFile="${keyValueStore}/${provisioningDomain}.${setupInfoKey}"
setupIDFile="${keyValueStore}/${provisioningDomain}.${setupIDKey}"
setupCodeFile="${keyValueStore}/${provisioningDomain}.${setupCodeKey}"
mfiTokenUUIDFile="${keyValueStore}/${provisioningDomain}.${mfiTokenUUIDKey}"
mfiTokenFile="${keyValueStore}/${provisioningDomain}.${mfiTokenKey}"

command=""
command="${command}"'rm -rf '"${setupInfoFile}"' '"${setupIDFile}"' '"${setupCodeFile}"' && '
if (( ! preserveMFiToken )); then
    command="${command}"'rm -rf '"${mfiTokenUUIDFile}"' '"${mfiTokenFile}"' && '
fi
command="${command}"'mkdir -p '"${keyValueStore}"' && '
if (( ! supportsDisplay )); then
    command="${command}"'echo -n "'"${srpSalt}${srpVerifier}"'" | '
    # shellcheck disable=SC2016
    command="${command}"'perl -ne '"'"'s/([0-9a-f]{2})/print chr hex $1/gie'"'"' > '"${setupInfoFile}"' && '
    if (( supportsProgrammableNFC )); then
        command="${command}"'echo -en "'"${setupCode}"'\0" > '"${setupCodeFile}"' && '
    fi
fi
command="${command}"'echo -en "'"${setupID}"'\0" > '"${setupIDFile}"' && '
if [ "${mfiTokenUUID}" != "" ] && [ "${mfiToken}" != "" ]; then
    command="${command}"'echo -n "'"${mfiTokenUUID}"'" | '
    # shellcheck disable=SC2016
    command="${command}"'perl -ne '"'"'s/([0-9a-f]{2})/print chr hex $1/gie'"'"' > '"${mfiTokenUUIDFile}"' && '
    command="${command}"'echo -n "'"${mfiToken}"'" | '
    # shellcheck disable=SC2016
    command="${command}"'perl -ne '"'"'s/([0-9a-f]{2})/print chr hex $1/gie'"'"' > '"${mfiTokenFile}"' && '
fi
command="${command}"'true'

if [ "${destination}" != "${keyValueStore}" ]; then
    if ! ${remoteShell} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null "${host}" "${command}"; then
        fail "Failed to provision ${destination}."
    fi
else
    if ! eval "${command}"; then
        fail "Failed to provision ${destination}."
    fi
fi

################################################################################
# Display information to print on label.
################################################################################
if (( ! supportsDisplay )); then
    echo "================================================================================"
    echo "                             Setup code: ${setupCode}"
    echo "                          Setup payload: ${setupPayload}"
    echo "================================================================================"
    if (( ! supportsProgrammableNFC )); then
        if command -v qrencode > /dev/null; then
            qrencode -t ANSI256 "${setupPayload}"
        else
            echo "qrencode not installed (brew install qrencode). Not generating QR code."
        fi
        echo "================================================================================"
    else
        if command -v qrencode > /dev/null; then
            nfcLabel=""
            nfcLabel="${nfcLabel}"'                                                         \n'
            nfcLabel="${nfcLabel}"'                                                         \n'
            nfcLabel="${nfcLabel}"'                                                         \n'
            nfcLabel="${nfcLabel}"'                                                         \n'
            nfcLabel="${nfcLabel}"'         XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         \n'
            nfcLabel="${nfcLabel}"'        XXX                                   XXX        \n'
            nfcLabel="${nfcLabel}"'        XX                                     XX        \n'
            nfcLabel="${nfcLabel}"'        XX                                     XX        \n'
            nfcLabel="${nfcLabel}"'        XX                      XX             XX        \n'
            nfcLabel="${nfcLabel}"'        XX                 XX    XX            XX        \n'
            nfcLabel="${nfcLabel}"'        XX             XX   XX    XX           XX        \n'
            nfcLabel="${nfcLabel}"'        XX         XX   XX   XX    XX          XX        \n'
            nfcLabel="${nfcLabel}"'        XX          XX   XX   XX   XX          XX        \n'
            nfcLabel="${nfcLabel}"'        XX           XX   XX   XX   XX         XX        \n'
            nfcLabel="${nfcLabel}"'        XX           XX   XX   XX   XX         XX        \n'
            nfcLabel="${nfcLabel}"'        XX           XX   XX   XX   XX         XX        \n'
            nfcLabel="${nfcLabel}"'        XX          XX   XX   XX   XX          XX        \n'
            nfcLabel="${nfcLabel}"'        XX         XX   XX   XX    XX          XX        \n'
            nfcLabel="${nfcLabel}"'        XX             XX   XX    XX           XX        \n'
            nfcLabel="${nfcLabel}"'        XX                 XX    XX            XX        \n'
            nfcLabel="${nfcLabel}"'        XX                      XX             XX        \n'
            nfcLabel="${nfcLabel}"'        XX                                     XX        \n'
            nfcLabel="${nfcLabel}"'        XX                                     XX        \n'
            nfcLabel="${nfcLabel}"'        XXX                                   XXX        \n'
            nfcLabel="${nfcLabel}"'         XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         \n'
            nfcLabel="${nfcLabel}"'                                                         \n'
            nfcLabel="${nfcLabel}"'                                                         \n'
            nfcLabel="${nfcLabel}"'                                                         \n'
            nfcLabel="${nfcLabel}"'                                                         \n'
            nfcLabel="${nfcLabel// /\\x1B[48;5;231m \\x1B[0m}"
            nfcLabel="${nfcLabel//X/\\x1B[48;5;16m \\x1B[0m}"
            echo -e -n "${nfcLabel}"
            echo "================================================================================"
        fi
    fi
fi
