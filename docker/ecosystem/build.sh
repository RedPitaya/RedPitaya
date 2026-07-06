#!/bin/bash
set -e

function print_ok(){ echo -e "\033[92m[OK]\e[0m"; }
function print_fail(){ echo -e "\033[91m[FAIL]\e[0m"; }

echo "Start build process..."

BUILD_ENV_IMG="rp-build-x86:latest"
RP_ARM_IMG="rp-ubuntu-arm:latest"

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PROJECT_ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)

echo "Project root directory: ${PROJECT_ROOT}"

GIT_COMMIT_SHORT=$(git -C "${PROJECT_ROOT}" rev-parse --short HEAD 2>/dev/null || echo "unknown")

echo "Registering QEMU binfmt..."
docker run --privileged --rm tonistiigi/binfmt --install all
print_ok

echo "Building Docker images..."
docker build -f "${SCRIPT_DIR}/Dockerfile.x86_build" -t ${BUILD_ENV_IMG} "${SCRIPT_DIR}"
docker build -f "${SCRIPT_DIR}/Dockerfile.rp_ubuntu" -t ${RP_ARM_IMG} "${SCRIPT_DIR}"
echo -n "Docker images built successfully. "
print_ok

echo "Running Makefile inside Red Pitaya OS container..."
docker run --rm \
    -v "${PROJECT_ROOT}:/workspace" \
    -w /workspace \
    ${RP_ARM_IMG} \
    bash -c "
        make -f Makefile REVISION=${GIT_COMMIT_SHORT} ENABLE_PRODUCTION_TEST=0 BUILD_NUMBER=1
    "

if [[ $? -eq 0 ]]; then
    echo -n "ARM build complete. "
    print_ok
else
    echo -n "ARM build failed. "
    print_fail
    exit 1
fi

echo "Running Makefile.x86..."
docker run --rm \
    -v "${PROJECT_ROOT}:/workspace" \
    -w /workspace \
    ${BUILD_ENV_IMG} \
    make -f Makefile.x86

echo "Packaging zip archive..."
docker run --rm \
    -v "${PROJECT_ROOT}:/workspace" \
    -w /workspace \
    ${BUILD_ENV_IMG} \
    make -f Makefile.x86 zip

echo -n "Build completed successfully! "
print_ok
