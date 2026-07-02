FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

ARG GIT_COMMIT_HASH=unknown
ARG BUILD_DATE=unknown

LABEL maintainer="dev@redpitaya.com" \
    description="Qt 6.7.3 development environment for Windows MinGW64 cross-compilation with QML support and CQtDeployer" \
    git_commit_hash="${GIT_COMMIT_HASH}" \
    build_date="${BUILD_DATE}" \
    version="1.0.0"

# Install system dependencies and MinGW64
# Enable i386 architecture for wine32, update repos, and install dependencies
RUN dpkg --add-architecture i386 && \
    apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    git \
    python3 \
    python3-pip \
    pipx \
    pkg-config \
    mingw-w64 \
    mingw-w64-tools \
    g++-mingw-w64-x86-64 \
    binutils-mingw-w64-x86-64 \
    libglib2.0-0 \
    libglib2.0-dev \
    libdbus-1-3 \
    libdbus-1-dev \
    libpulse0 \
    libpulse-dev \
    libudev1 \
    libudev-dev \
    libxcb-cursor0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-sync1 \
    libxcb-xfixes0 \
    libxcb-xkb1 \
    libxkbcommon0 \
    libxkbcommon-x11-0 \
    libx11-6 \
    libx11-xcb1 \
    libxext6 \
    libxfixes3 \
    libxi6 \
    libxrender1 \
    libfontconfig1 \
    libfreetype6 \
    libpng16-16 \
    libjpeg-turbo8 \
    libgl1 \
    libglu1-mesa \
    libicu74 \
    libvulkan1 \
    libvulkan-dev \
    vulkan-tools \
    wine64 \
    wine64-tools \
    wine32:i386 \
    xvfb \
    zip \
    unzip \
    file \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*


# ============================================
# Install aqtinstall
# ============================================
RUN pipx install aqtinstall
ENV PATH="/root/.local/bin:${PATH}"

# ============================================
# Install Qt 6.7.3 for Windows MinGW64 with QML
# ============================================
RUN aqt install-qt windows desktop 6.7.3 win64_mingw -O /opt/qt -b https://master.qt.io/ \
    --archives qtbase qtdeclarative qtshadertools qtsvg icu \
    --modules \
    qtmultimedia \
    qtwebsockets \
    qt5compat \
    qtcharts \
    qtscxml \
    qtquick3d \
    qtquicktimeline

# ============================================
# Set Qt environment variables for cross-compilation
# ============================================
ENV QT_DIR=/opt/qt/6.7.3/mingw_64
ENV MINGW_DIR=/usr/x86_64-w64-mingw32
ENV PATH="${QT_DIR}/bin:${MINGW_DIR}/bin:${PATH}"
ENV CMAKE_PREFIX_PATH="${QT_DIR}"
ENV QML_IMPORT_PATH="${QT_DIR}/qml"
ENV QML2_IMPORT_PATH="${QT_DIR}/qml"

# ============================================
# Verify installation
# ============================================
RUN ${MINGW_DIR}/bin/qmake --version || echo "qmake not available for direct execution, use CMake toolchain"

# ============================================
# Install CQtDeployer for Linux (host tool)
# Note: CQtDeployer runs on host to package Windows binaries
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
    echo "Qt version: 6.7.3 (Windows MinGW64 cross-compilation)" >> /etc/build_info.txt && \
    echo "CQtDeployer version: 1.6.2441" >> /etc/build_info.txt && \
    echo "Target: Windows x86_64" >> /etc/build_info.txt && \
    echo "QML support: enabled" >> /etc/build_info.txt


RUN ln -sf /usr/bin/wine /usr/bin/wine64

# ============================================
# Verify installation
# ============================================
RUN cqtdeployer v && \
    cat /etc/build_info.txt && \
    ls -la ${QT_DIR}/bin/

# ============================================
# Set working directory
# ============================================
WORKDIR /workspace
CMD ["/bin/bash"]