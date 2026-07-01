FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

ARG GIT_COMMIT_HASH=unknown
ARG BUILD_DATE=unknown

LABEL maintainer="dev@redpitaya.com" \
    description="Qt 6.12.0 development environment with QML support and CQtDeployer" \
    git_commit_hash="${GIT_COMMIT_HASH}" \
    build_date="${BUILD_DATE}" \
    version="1.0.0"

# ============================================
# Install system dependencies
# ============================================
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    git \
    python3 \
    python3-pip \
    pipx \
    pkg-config \
    libssl-dev \
    libdbus-1-dev \
    libpulse-dev \
    libudev-dev \
    libxcb-cursor-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-shape0-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-xkb-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libx11-dev \
    libx11-xcb-dev \
    libxext-dev \
    libxfixes-dev \
    libxi-dev \
    libxrender-dev \
    libxcb-glx0-dev \
    libxcb-util-dev \
    libxcb-shm0-dev \
    libxcb-xinerama0-dev \
    libfontconfig1-dev \
    libfreetype-dev \
    libpng-dev \
    libjpeg-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libicu-dev \
    libwayland-dev \
    wayland-protocols \
    zip \
    file \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# ============================================
# Install aqtinstall
# ============================================
RUN pipx install aqtinstall
ENV PATH="/root/.local/bin:${PATH}"

# ============================================
# Install Qt 6.12.0
# ============================================
RUN aqt install-qt linux desktop 6.12.0 linux_gcc_64 -O /opt/qt -b https://master.qt.io/ \
    --archives qtbase qtdeclarative qtshadertools qtsvg icu qtwayland \
    --modules \
    qtmultimedia \
    qtwebsockets \
    qt5compat \
    qtcharts \
    qtscxml

# ============================================
# Set Qt environment variables
# ============================================
ENV QT_DIR=/opt/qt/6.12.0/gcc_64
ENV PATH="${QT_DIR}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${QT_DIR}/lib:${LD_LIBRARY_PATH}"
ENV QT_QPA_PLATFORM=offscreen
ENV CMAKE_PREFIX_PATH="${QT_DIR}"

# ============================================
# Verify installation
# ============================================
RUN qmake --version

# ============================================
# Install CQtDeployer v1.6.2365 from source
# ============================================
RUN wget -O /tmp/cqtdeployer.deb \
    https://github.com/QuasarApp/CQtDeployer/releases/download/v1.6.2441.67ee1ef/CQtDeployer_1.6.2441.67ee1ef_Linux_x86_64.deb && \
    dpkg -i /tmp/cqtdeployer.deb && \
    rm /tmp/cqtdeployer.deb

# ============================================
# Store build info
# ============================================
ENV GIT_COMMIT_HASH=${GIT_COMMIT_HASH}
RUN echo "Git commit hash: ${GIT_COMMIT_HASH}" > /etc/build_info.txt && \
    echo "Build date: ${BUILD_DATE}" >> /etc/build_info.txt && \
    echo "Qt version: 6.12.0" >> /etc/build_info.txt && \
    echo "CQtDeployer version: 1.6.2365" >> /etc/build_info.txt

# ============================================
# Verify installation
# ============================================
RUN qmake --version && \
    cqtdeployer v && \
    cat /etc/build_info.txt

# ============================================
# Set working directory
# ============================================
WORKDIR /workspace
CMD ["/bin/bash"]