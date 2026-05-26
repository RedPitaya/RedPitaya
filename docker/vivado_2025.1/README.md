# Documentation: Docker Image for Vivado Jenkins Agent

## Overview

This project provides a Docker image `vivado-jenkins-agent:2025.1` for running a Jenkins agent with AMD Vitis Unified Software Platform (Vivado) installed. The image is designed for headless usage in CI/CD pipelines for FPGA synthesis and development.

---

## File Structure

| File | Purpose |
|------|---------|
| `Dockerfile` | Multi-stage image build |
| `install_config.txt` | Vivado installation configuration |
| `jenkins-agent-setup.sh` | Jenkins agent startup script |

---

## Build Preparation

### 1. Obtaining Vivado Installer Files

Download the Vivado installer from the official AMD (formerly Xilinx) website:

```bash
# Example for version 2025.1 (replace URL with actual one)
wget https://www.xilinx.com/member/forms/download/xef.html?filename=Xilinx_Unified_2025.1_MMDD_YYYY.tar.gz
```

### 2. Extracting the Installer

```bash
# Create directory for the installer
mkdir -p vivado_installer

# Extract the archive (replace with actual filename)
tar -xzf Xilinx_Unified_2025.1_MMDD_YYYY.tar.gz -C vivado_installer

# Verify xsetup file exists
ls -la vivado_installer/xsetup
```

### 3. Placing Configuration Files

Ensure all files are in the same directory:

```bash
ls -la
# Should see:
# - Dockerfile
# - install_config.txt
# - jenkins-agent-setup.sh
# - vivado_installer/
```

---

## Vivado Installation Configuration

### `install_config.txt` — Main Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| `Edition` | Vitis Unified Software Platform | Software edition |
| `Destination` | `/opt/Xilinx` | Installation path |
| `Modules` | `Zynq-7000:1,DocNav:1` | Modules to install |

### Installed Modules

The current configuration installs only:
- **Zynq-7000** — Zynq-7000 SoC support (enabled)
- **DocNav** — Documentation (enabled)

> **Note**: All other FPGA families (Virtex, Kintex, Artix, etc.) are disabled. If needed, uncomment the `Modules` line in `install_config.txt` and configure the required components by changing `:0` to `:1`.

---

## Jenkins Agent Script

### `jenkins-agent-setup.sh` — Functionality

1. Starts SSH daemon in background
2. Loads Vivado environment (`settings64.sh`)
3. Connects to Jenkins master (if environment variables are present)
4. Starts interactive Bash (if variables are absent)

### Jenkins Environment Variables

| Variable | Description | Required |
|----------|-------------|----------|
| `JENKINS_URL` | Jenkins master URL | Yes (for agent mode) |
| `JENKINS_SECRET` | Agent secret token | Yes (for agent mode) |
| `JENKINS_AGENT_NAME` | Agent name | Yes (for agent mode) |

---

### Dockerfile: Build Details

#### Multi-stage Build

| Stage | Purpose |
|-------|---------|
| `builder` | Install Vivado from installer |
| `final` | Final image with minimal dependencies |

#### Builder Stage — Installed Packages

- `libncurses6`, `libncursesw6`, `libtinfo6` — library compatibility
- `libstdc++6`, `libc6-i386` — 32-bit compatibility
- `python3`, `python3-pip` — installer utilities
- `openjdk-17-jdk` — for Jenkins agent
- `openssh-server`, `git`, `curl` — CI/CD utilities
- `locales` — locale support

#### Symbolic Links for Compatibility

```bash
ln -s /lib/x86_64-linux-gnu/libtinfo.so.6 /lib/x86_64-linux-gnu/libtinfo.so.5
ln -s /lib/x86_64-linux-gnu/libncurses.so.6 /lib/x86_64-linux-gnu/libncurses.so.5
ln -s /lib/x86_64-linux-gnu/libncursesw.so.6 /lib/x86_64-linux-gnu/libncursesw.so.5
```

#### Locale Configuration

```bash
locale-gen en_US.UTF-8
update-locale LANG=en_US.UTF-8
```

#### Final Image Environment Variables

```dockerfile
ENV LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8
ENV XILINX_VIVADO=/opt/Xilinx/2025.1/Vivado
ENV PATH="${XILINX_VIVADO}/bin:${PATH}"
```

#### BuildKit Mounting Behavior

`--mount=type=bind` mounts in Docker BuildKit exist **only during the execution of the RUN command** and are automatically removed after completion. No explicit `umount` is required.

```dockerfile
# Mounts are available ONLY inside this RUN
RUN --mount=type=bind,source=vivado_installer,target=/tmp/vivado_installer,rw \
    --mount=type=bind,source=install_config.txt,target=/tmp/install_config.txt \
    cd /tmp/vivado_installer && \
    ./xsetup --batch Install --config /tmp/install_config.txt
# Mounts are automatically unmounted here
```

---

## Building the Docker Image

### Basic Build

```bash
docker build -t vivado-jenkins-agent:2025.1 .
```

### Build with Platform Specification

```bash
# For AMD64 (x86_64)
docker build --platform linux/amd64 -t vivado-jenkins-agent:2025.1 .
```

### Build Without Cache (Clean Build)

```bash
docker build --no-cache -t vivado-jenkins-agent:2025.1 .
```

### Build Using BuildKit (Recommended)

```bash
DOCKER_BUILDKIT=1 docker build -t vivado-jenkins-agent:2025.1 .
```

---

## Creating a TAR Archive

### Export Image to TAR

```bash
docker save -o vivado-jenkins-agent-2025.1.tar vivado-jenkins-agent:2025.1
```

### Compress TAR File

```bash
gzip vivado-jenkins-agent-2025.1.tar
# Result: vivado-jenkins-agent-2025.1.tar.gz
```

### Quick Export with Compression (Single Command)

```bash
docker save vivado-jenkins-agent:2025.1 | gzip > vivado-jenkins-agent-2025.1.tar.gz
```

### Maximum Compression

```bash
docker save vivado-jenkins-agent:2025.1 | gzip -9 > vivado-jenkins-agent-2025.1.tar.gz
```

### View Image Information Before Export

```bash
# List images
docker images | grep vivado-jenkins-agent

# Detailed information
docker inspect vivado-jenkins-agent:2025.1

# Layer size
docker history vivado-jenkins-agent:2025.1
```

---

## Automated Build Script

Create `build.sh`:

```bash
#!/bin/bash
set -e

# Colored output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== Vivado Jenkins Agent Builder ===${NC}"

# Check if Vivado installer exists
if [ ! -d "vivado_installer" ] || [ ! -f "vivado_installer/xsetup" ]; then
    echo -e "${RED}Error: Vivado installer not found in ./vivado_installer/${NC}"
    echo "Please extract the Vivado installer to the vivado_installer directory/"
    exit 1
fi

# Check required files
REQUIRED_FILES=("Dockerfile" "install_config.txt" "jenkins-agent-setup.sh")
for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "${RED}Error: File $file not found${NC}"
        exit 1
    fi
done

# Build parameters
IMAGE_NAME="vivado-jenkins-agent"
VERSION="2025.1"
ARCHIVE_NAME="${IMAGE_NAME}-${VERSION}.tar.gz"

echo -e "${YELLOW}Starting Docker image build...${NC}"
echo "Image name: ${IMAGE_NAME}:${VERSION}"

# Build with BuildKit
DOCKER_BUILDKIT=1 docker build \
    --tag ${IMAGE_NAME}:${VERSION} \
    --file Dockerfile \
    .

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Image built successfully${NC}"
else
    echo -e "${RED}✗ Image build failed${NC}"
    exit 1
fi

# Size information
echo -e "${YELLOW}Image information:${NC}"
docker images | grep ${IMAGE_NAME}

# Export to compressed TAR
echo -e "${YELLOW}Exporting image to TAR archive...${NC}"
docker save ${IMAGE_NAME}:${VERSION} | gzip > ${ARCHIVE_NAME}

if [ $? -eq 0 ]; then
    ARCHIVE_SIZE=$(du -h ${ARCHIVE_NAME} | cut -f1)
    echo -e "${GREEN}✓ Image exported to ${ARCHIVE_NAME}${NC}"
    echo -e "${GREEN}Archive size: ${ARCHIVE_SIZE}${NC}"
else
    echo -e "${RED}✗ Image export failed${NC}"
    exit 1
fi

echo -e "${GREEN}=== Build completed successfully ===${NC}"
echo "Files:"
echo "  - Docker image: ${IMAGE_NAME}:${VERSION}"
echo "  - TAR archive: ${ARCHIVE_NAME}"
```

Run the script:

```bash
chmod +x build.sh
./build.sh
```

---

## Loading Image from TAR Archive

On the target machine:

### From Compressed Archive (Recommended)

```bash
gunzip -c vivado-jenkins-agent-2025.1.tar.gz | docker load
```

### From Uncompressed TAR

```bash
docker load -i vivado-jenkins-agent-2025.1.tar
```

### Verify Loaded Image

```bash
docker images | grep vivado-jenkins-agent
```

---

## Running and Usage

### Run as Jenkins Agent

```bash
docker run -d \
  --name vivado-agent \
  -e JENKINS_URL="http://jenkins.example.com:8080" \
  -e JENKINS_SECRET="your-secret-token" \
  -e JENKINS_AGENT_NAME="vivado-agent-01" \
  vivado-jenkins-agent:2025.1
```

### Interactive Run (Debug/Development Mode)

```bash
docker run -it \
  --name vivado-dev \
  -v "$(pwd)":/workspace \
  vivado-jenkins-agent:2025.1
```

### Run with SSH Access

```bash
# Run with SSH port forwarding
docker run -d \
  --name vivado-ssh \
  -p 2222:22 \
  vivado-jenkins-agent:2025.1

# Connect via SSH
ssh jenkins@localhost -p 2222
# Password: jenkins
```

### Run with Explicit Locale Settings

```bash
docker run -it \
  -e LANG=en_US.UTF-8 \
  -e LC_ALL=en_US.UTF-8 \
  -e LANGUAGE=en_US:en \
  vivado-jenkins-agent:2025.1
```

---

## Verifying Vivado Operation

```bash
# Check locale
root@container:/# locale
LANG=en_US.UTF-8
LANGUAGE=en_US:en
LC_ALL=en_US.UTF-8

# Check Vivado version
root@container:/# vivado -version

# Run in batch mode
root@container:/# vivado -mode batch

# Check environment
root@container:/# echo $XILINX_VIVADO
/opt/Xilinx/2025.1/Vivado

# Check PATH
root@container:/# which vivado
/opt/Xilinx/2025.1/Vivado/bin/vivado
```

---

## Usage in Jenkins Pipeline

### Example Jenkinsfile

```groovy
pipeline {
    agent {
        docker {
            image 'vivado-jenkins-agent:2025.1'
            args '-v /opt/Xilinx:/opt/Xilinx:ro'
        }
    }

    environment {
        XILINX_VIVADO = '/opt/Xilinx/2025.1/Vivado'
        LANG = 'en_US.UTF-8'
        LC_ALL = 'en_US.UTF-8'
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Synthesis') {
            steps {
                sh '''
                    source /opt/Xilinx/2025.1/Vivado/settings64.sh
                    vivado -mode batch -source synthesize.tcl
                '''
            }
        }

        stage('Implementation') {
            steps {
                sh '''
                    source /opt/Xilinx/2025.1/Vivado/settings64.sh
                    vivado -mode batch -source implement.tcl
                '''
            }
        }

        stage('Generate Bitstream') {
            steps {
                sh '''
                    source /opt/Xilinx/2025.1/Vivado/settings64.sh
                    vivado -mode batch -source generate_bitstream.tcl
                '''
            }
        }
    }

    post {
        success {
            archiveArtifacts '*.bit'
            archiveArtifacts '*.rpt'
        }
    }
}
```

### Usage with Kubernetes (Jenkins Kubernetes Plugin)

```yaml
apiVersion: v1
kind: Pod
spec:
  containers:
  - name: vivado
    image: vivado-jenkins-agent:2025.1
    command:
    - /usr/local/bin/jenkins-agent-setup.sh
    env:
    - name: JENKINS_URL
      value: "http://jenkins:8080"
    - name: JENKINS_SECRET
      valueFrom:
        secretKeyRef:
          name: jenkins-agent
          key: secret
    - name: LANG
      value: "en_US.UTF-8"
    - name: LC_ALL
      value: "en_US.UTF-8"
    - name: LANGUAGE
      value: "en_US:en"
    resources:
      requests:
        memory: "8Gi"
        cpu: "4"
      limits:
        memory: "16Gi"
        cpu: "8"
    volumeMounts:
    - name: license
      mountPath: /opt/Xilinx/.Xilinx
  volumes:
  - name: license
    hostPath:
      path: /opt/xilinx_licenses
```

---

## Troubleshooting

### Locale Error: "cannot change locale (en_US.UTF-8)"

**Cause**: Required locale missing in the image.

**Solution**: Ensure `locales` package and locale generation are added to Dockerfile, or specify variables at runtime:

```bash
docker run -e LANG=en_US.UTF-8 -e LC_ALL=en_US.UTF-8 ...
```

### Error: "libtinfo.so.5: cannot open shared object file"

**Cause**: Missing symbolic links to libraries.

**Solution**: Check for links:

```bash
docker run --rm vivado-jenkins-agent:2025.1 ls -la /lib/x86_64-linux-gnu/libtinfo.so*
```

### Error: Vivado installer not found during build

**Cause**: Incorrect directory structure.

**Solution**:

```bash
ls -la vivado_installer/xsetup
# Should exist and be executable
chmod +x vivado_installer/xsetup
```

### Vivado won't start (X server required)

**Cause**: Attempting to run in GUI mode.

**Solution**: Use batch mode:

```bash
vivado -mode batch -source script.tcl
# or
vivado -mode tcl -source script.tcl
```

### Insufficient memory during synthesis

**Solution**: Increase memory limits for the container:

```bash
docker run --memory="16g" --memory-swap="16g" \
  vivado-jenkins-agent:2025.1
```

### Jenkins agent fails to connect

**Solution**: Check environment variables and network connectivity:

```bash
# Check inside container
docker run --rm vivado-jenkins-agent:2025.1 \
  curl -v http://jenkins-master:8080/
```

### TAR archive is too large

**Solution**: Use maximum compression and clean Docker cache before export:

```bash
docker system prune -a
docker save vivado-jenkins-agent:2025.1 | gzip -9 > image.tar.gz
```

### Verify no mounts remain in final image

```bash
# Check that no mounts exist in the final image
docker run --rm vivado-jenkins-agent:2025.1 mount | grep /tmp
# Should be empty
```

---

## Quick Reference Commands

| Action | Command |
|--------|---------|
| Build image | `docker build -t vivado-jenkins-agent:2025.1 .` |
| Build with BuildKit | `DOCKER_BUILDKIT=1 docker build -t vivado-jenkins-agent:2025.1 .` |
| Export to TAR.GZ | `docker save vivado-jenkins-agent:2025.1 \| gzip > vivado-jenkins-agent-2025.1.tar.gz` |
| Load from TAR.GZ | `gunzip -c vivado-jenkins-agent-2025.1.tar.gz \| docker load` |
| Run Jenkins agent | `docker run -d -e JENKINS_URL=... -e JENKINS_SECRET=... vivado-jenkins-agent:2025.1` |
| Interactive run | `docker run -it vivado-jenkins-agent:2025.1` |
| Interactive run with locale | `docker run -it -e LANG=en_US.UTF-8 -e LC_ALL=en_US.UTF-8 vivado-jenkins-agent:2025.1` |
| View logs | `docker logs <container-id>` |
| Stop container | `docker stop <container-id>` |
| Remove container | `docker rm <container-id>` |
| Remove image | `docker rmi vivado-jenkins-agent:2025.1` |
| Check locale | `docker run --rm vivado-jenkins-agent:2025.1 locale` |
| Check Vivado | `docker run --rm vivado-jenkins-agent:2025.1 vivado -version` |
