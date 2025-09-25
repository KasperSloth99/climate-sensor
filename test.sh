#!/bin/bash -eu

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
source "${SCRIPT_DIR}"/common.sh

EXTRA_TEST_ARGS=()
TEST_PLATFORM="native_sim"
RUN_SETUP=false

usage() {
  echo "usage:  $0 [-h|--help]"
  echo "-v | -vv | -vvv                 (Optional)            Run twister with -v (Can be called multiple times)"
  echo "-s | --setup                    (Optional)            Setup the build environment - Default: ${RUN_SETUP}"
  echo "-h | --help                     (Optional)            Help information"
}

while [ $# -gt 0 ]; do
  case $1 in
    -v | -vv | -vvv)
      EXTRA_TEST_ARGS+=("$1")
      shift 1
      ;;
    -s | --setup)
      RUN_SETUP=true
      shift 1
      ;;
    --h | --help)
      usage
      exit 0
      ;;
    *)
      usage
      exit 1
      ;;
  esac
done

if [ "${RUN_SETUP}" = true ]; then
  "${SCRIPT_DIR}"/setup.sh
fi

# Activate local python virtual environment if available
if [[ -f $PYTHON_VENV_DIR/bin/activate ]]; then
  echo "Activating python virtual environment"
  source "${PYTHON_VENV_DIR}"/bin/activate
fi

cd "${FIRMWARE_DIR}"
if [ ! -d "${WEST_WORKSPACE}" ]; then
  west init -l "${SMCU_DIR}"
fi
west update

${SUDO_PREFIX} rm -rf "${FIRMWARE_DIR}"/twister-out*/

${SUDO_PREFIX} west twister -T "${FIRMWARE_DIR}/smcu/tests" -i -p ${TEST_PLATFORM} ${EXTRA_TEST_ARGS[@]}