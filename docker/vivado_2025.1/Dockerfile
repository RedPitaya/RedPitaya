# Dockerfile
FROM ubuntu:24.04 AS builder

# Minimal dependencies for Vivado headless installation
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    # Compatibility libraries
    libncurses6 \
    libncursesw6 \
    libtinfo6 \
    # Core system libraries
    libstdc++6 \
    libc6-i386 \
    # Installer utilities
    python3 \
    python3-pip \
    net-tools \
    # Jenkins agent prerequisites
    openjdk-17-jdk \
    openssh-server \
    git \
    curl \
    # Locale support
    locales \
    # Symlinks for backward compatibility
    && ln -s /lib/x86_64-linux-gnu/libtinfo.so.6 /lib/x86_64-linux-gnu/libtinfo.so.5 \
    && ln -s /lib/x86_64-linux-gnu/libncurses.so.6 /lib/x86_64-linux-gnu/libncurses.so.5 \
    && ln -s /lib/x86_64-linux-gnu/libncursesw.so.6 /lib/x86_64-linux-gnu/libncursesw.so.5 \
    # Generate locale
    && locale-gen en_US.UTF-8 \
    && update-locale LANG=en_US.UTF-8 \
    # Clean apt cache
    && rm -rf /var/lib/apt/lists/*

# Set locale environment variables
ENV LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8

# Install Vivado in batch mode using mounted files with write permissions
RUN --mount=type=bind,source=vivado_installer,target=/tmp/vivado_installer,rw \
    --mount=type=bind,source=install_config.txt,target=/tmp/install_config.txt \
    cd /tmp/vivado_installer && \
    chmod +x xsetup && \
    ./xsetup --agree 3rdPartyEULA,XilinxEULA \
             --batch Install \
             --config /tmp/install_config.txt

# ---- Final stage ----
FROM ubuntu:24.04

# Copy Vivado installation from builder
COPY --from=builder /opt/Xilinx /opt/Xilinx

# Install Java runtime in final stage (more reliable)
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    # Core compatibility libraries
    libncurses6 \
    libtinfo6 \
    libstdc++6 \
    libc6-i386 \
    # Jenkins agent requirements
    openjdk-17-jre-headless \
    openssh-server \
    git \
    curl \
    # Locale support
    locales \
    # Symlinks for Vivado compatibility
    && ln -s /lib/x86_64-linux-gnu/libtinfo.so.6 /lib/x86_64-linux-gnu/libtinfo.so.5 \
    && ln -s /lib/x86_64-linux-gnu/libncurses.so.6 /lib/x86_64-linux-gnu/libncurses.so.5 \
    && ln -s /lib/x86_64-linux-gnu/libncursesw.so.6 /lib/x86_64-linux-gnu/libncursesw.so.5 \
    # Generate locale
    && locale-gen en_US.UTF-8 \
    && update-locale LANG=en_US.UTF-8 \
    # Clean apt cache
    && rm -rf /var/lib/apt/lists/*

# Set locale environment variables
ENV LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8

# Create jenkins user
RUN useradd -m -s /bin/bash jenkins && \
    echo "jenkins:jenkins" | chpasswd && \
    mkdir -p /home/jenkins/agent /home/jenkins/.ssh && \
    chown -R jenkins:jenkins /home/jenkins

# Configure SSH for Jenkins
RUN ssh-keygen -A && \
    mkdir -p /var/run/sshd

# Download Jenkins agent jar
RUN curl --create-dirs -fsSLo /usr/share/jenkins/agent.jar \
    http://jenkins.redpitaya.com:8080/jnlpJars/agent.jar && \
    chmod 755 /usr/share/jenkins && \
    chown -R jenkins:jenkins /usr/share/jenkins

# Set Vivado environment variables
ENV XILINX_VIVADO=/opt/Xilinx/2025.1/Vivado
ENV PATH="${XILINX_VIVADO}/bin:${PATH}"

# Copy agent startup script
COPY jenkins-agent-setup.sh /usr/local/bin/jenkins-agent-setup.sh
RUN chmod +x /usr/local/bin/jenkins-agent-setup.sh

# Verify Java installation
RUN java -version

EXPOSE 22

ENTRYPOINT ["/usr/local/bin/jenkins-agent-setup.sh"]
