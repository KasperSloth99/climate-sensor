#!/bin/bash -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
source "${SCRIPT_DIR}"/common.sh

HOST_ARCH="x86_64"
ZEPHYR_SDK_TAR="${ZEPHYR_SDK}_linux-${HOST_ARCH}_minimal.tar.xz"
ZEPHYR_GIT="https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_TOOLCHAIN_VERSION}"

echo "Creating python virtual environment"
python3 -m venv "${PYTHON_VENV_DIR}"
if [ -f "${PYTHON_VENV_DIR}"/bin/activate ]; then
  source "${PYTHON_VENV_DIR}"/bin/activate
else
  echo -e "Unable to activate venv"
  exit 1
fi

echo "Checking zephyr requirements"
pip install --no-cache-dir -q -r https://raw.githubusercontent.com/zephyrproject-rtos/zephyr/main/scripts/requirements.txt
echo "Done"

echo "Checking zephyr sdk in ${ZEPHYR_SDK_INSTALL_DIR}"
if [ ! -d "${ZEPHYR_SDK_INSTALL_DIR}" ]; then
  echo "Zephyr SDK not found in ${ZEPHYR_SDK_INSTALL_DIR}, installing"
  wget -q --show-progress "${ZEPHYR_GIT}/${ZEPHYR_SDK_TAR}"
  wget -q --show-progress -O - "${ZEPHYR_GIT}/sha256.sum" | shasum --check --ignore-missing
  sudo tar xf "${ZEPHYR_SDK_TAR}" -C "${ZEPHYR_SDK_INSTALL_DIR_ROOT}"
  rm "${ZEPHYR_SDK_TAR}"
  sudo "${ZEPHYR_SDK_INSTALL_DIR}"/setup.sh -c -h -t riscv64-zephyr-elf
fi

echo "Done"
