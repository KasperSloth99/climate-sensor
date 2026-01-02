#!/bin/bash -eu

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
source "${SCRIPT_DIR}"/common.sh

BOARD="esp32c3_supermini"
BUILD_DIR="build_${BOARD}"
WEST_ARGS=()
DO_FLASH=false
CLEAN_BUILD=false
PURGE_ENV=false
RUN_SETUP=false
AUTOCONNECT=false
ESP32C3_DEV="/dev/ttyACM0"

usage() {
  echo "usage:  $0 [-h|--help]"
  echo "-c | --clean                    (Optional)            Do a clean build - Default: ${CLEAN_BUILD}"
  echo "-f | --flash                    (Optional)            Flash the target(s) - Default: ${DO_FLASH}"
  echo "-p | --purge                    (Optional)            Clean the build environment - Default: ${PURGE_ENV}"
  echo "--sdk [SDK_PATH]                (Optional)            SDK target directory - Default: ${ZEPHYR_SDK_INSTALL_DIR}"
  echo "-s | --setup                    (Optional)            Setup the build environment - Default: ${RUN_SETUP}"
  echo "-m | --menu                     (Optional)            Open menuconfig"
  echo "-h | --help                     (Optional)            Help information"
}

while [ $# -gt 0 ]; do
  case $1 in
    -c | --clean)
      CLEAN_BUILD=true
      shift 1
      ;;

    -f | --flash)
      DO_FLASH=true
      shift 1
      ;;

    -p | --purge)
      PURGE_ENV=true
      CLEAN_BUILD=true
      shift 1
      ;;

    -m | --menu)
      WEST_ARGS+=("-t")
      WEST_ARGS+=("menuconfig")
      shift 1
      ;;

    --sdk)
      if [ -z "${2}" ]; then
        echo "Usage: --sdk [SDK_PATH]"
        exit 1
      fi
      ZEPHYR_SDK_INSTALL_DIR="${2}"
      shift 2
      ;;

    -s | --setup)
      RUN_SETUP=true
      shift 1
      ;;

    -ac | --autoconnect)
      AUTOCONNECT=true
      shift 1
      ;;

    -h | --help)
      usage
      exit 0
      ;;

    *)
      usage
      exit 1
      ;;
  esac
done

if [ ${PURGE_ENV} = true ]; then
  echo "Cleaning up python virtual environment.."
  if [[ -n "$VIRTUAL_ENV" ]]; then
    deactivate
  fi
  rm -rf "${PYTHON_VENV_DIR}"

  echo "Cleaning up zephyr environment.."
  rm -rf "${SCRIPT_DIR}"/*.tar*
  rm -rf "${SCRIPT_DIR}"/build*/
fi

if [ ${CLEAN_BUILD} = true ]; then
  rm -rf "${SCRIPT_DIR}"/build*/
  # We need sudo if running inside docker since files are generated with "sudo west twister.."
  rm -rf "${FIRMWARE_DIR}"/twister-out*/
  WEST_ARGS+=("--pristine")
fi

if [ ${RUN_SETUP} = true ]; then
  "${SCRIPT_DIR}"/setup.sh
fi

# Activate local python virtual environment if available
if [[ -f $PYTHON_VENV_DIR/bin/activate ]]; then
  echo "Activating python virtual environment"
  source "${PYTHON_VENV_DIR}"/bin/activate
fi

if ! command -v west &>/dev/null; then
  echo -e "Missing west, please run script with --setup flag"
  exit 1
fi

if [ ! -d "${ZEPHYR_SDK_INSTALL_DIR}" ]; then
  echo -e "Missing zephyr SDK, please run script with --setup flag"
  exit 1
fi

# Ensure submodules are present and up to date
cd "${FIRMWARE_DIR}"
if [ ! -d "${WEST_WORKSPACE}" ]; then
  west init -l "${PROJECT_DIR}"
fi
west update
west blobs fetch hal_espressif

west build "${SRC_DIR}" "${WEST_ARGS[@]}" -b "${BOARD}" -d "${SCRIPT_DIR}/${BUILD_DIR}"
if [ $DO_FLASH = true ]; then
  west flash -d "${SCRIPT_DIR}/${BUILD_DIR}" --esp-device "${ESP32C3_DEV}"
  if [ $AUTOCONNECT = true ]; then
    picocom -b 115200 "${ESP32C3_DEV}"
  fi
fi