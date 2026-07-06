$ErrorActionPreference = "Stop"

function Print-Ok { Write-Host "[OK]" -ForegroundColor Green }
function Print-Fail { Write-Host "[FAIL]" -ForegroundColor Red }

Write-Host "Start build process..." -ForegroundColor Cyan

$BUILD_ENV_IMG = "rp-build-x86:latest"
$RP_ARM_IMG = "rp-ubuntu-arm:latest"

$SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Definition
$PROJECT_ROOT = Resolve-Path "$SCRIPT_DIR\..\.." | Select-Object -ExpandProperty Path

Write-Host "Project root directory: $PROJECT_ROOT" -ForegroundColor Yellow

try {
    $GIT_COMMIT_SHORT = git -C "$PROJECT_ROOT" rev-parse --short HEAD 2>$null
} catch {
    $GIT_COMMIT_SHORT = "unknown"
}

Write-Host "Building Docker images..." -ForegroundColor Cyan
docker build -f "$SCRIPT_DIR\Dockerfile.x86_build" -t $BUILD_ENV_IMG "$SCRIPT_DIR"
docker build -f "$SCRIPT_DIR\Dockerfile.rp_ubuntu" --platform linux/arm/v7 -t $RP_ARM_IMG "$SCRIPT_DIR"
Write-Host "Docker images built successfully. " -NoNewline; Print-Ok

Write-Host "Step 1: Running Makefile.x86 setup..." -ForegroundColor Cyan
docker run --rm `
    -v "${PROJECT_ROOT}:/workspace" `
    -w /workspace `
    $BUILD_ENV_IMG `
    bash -c "find . -type f \( -name '*.sh' -o -name 'Makefile*' \) -exec dos2unix {} + 2>/dev/null; make -f Makefile.x86"
Print-Ok

Write-Host "Step 2: Running Makefile inside Red Pitaya OS container..." -ForegroundColor Cyan
docker run --rm `
    --platform linux/arm/v7 `
    -v "${PROJECT_ROOT}:/workspace" `
    -w /workspace `
    $RP_ARM_IMG `
    bash -c "make -f Makefile CROSS_COMPILE=\"\" REVISION=$GIT_COMMIT_SHORT ENABLE_PRODUCTION_TEST=0 BUILD_NUMBER=1"
Print-Ok

Write-Host "Step 3: Packaging zip archive..." -ForegroundColor Cyan
docker run --rm `
    -v "${PROJECT_ROOT}:/workspace" `
    -w /workspace `
    $BUILD_ENV_IMG `
    make -f Makefile.x86 zip

Write-Host "Build completed successfully! " -NoNewline; Print-Ok
