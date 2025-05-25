#!/bin/bash

set -e

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

echo "Script directory: $SCRIPT_DIR"

source "${SCRIPT_DIR}/poky/oe-init-build-env"

if ! command -v bitbake &> /dev/null; then
    echo "Bitbake is not installed. Please source the environment setup script first."
    exit 1
fi

echo "Adding bitbake layers..."

pushd ./poky/build &> /dev/null
bitbake-layers add-layer \
    ${SCRIPT_DIR}/meta-air-quality \
    ${SCRIPT_DIR}/meta-raspberrypi \
    ${SCRIPT_DIR}/meta-openembedded/meta-oe \
    ${SCRIPT_DIR}/meta-openembedded/meta-python \
    ${SCRIPT_DIR}/meta-openembedded/meta-networking
popd &> /dev/null
