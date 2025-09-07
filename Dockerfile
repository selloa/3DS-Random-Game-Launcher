FROM ubuntu:20.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    wget \
    unzip \
    make \
    && rm -rf /var/lib/apt/lists/*

# Install devkitPro
RUN wget https://github.com/devkitPro/pacman/releases/download/devkitpro-pacman-1.0.2/devkitpro-pacman.amd64.deb && \
    dpkg -i devkitpro-pacman.amd64.deb && \
    rm devkitpro-pacman.amd64.deb

# Install 3DS development tools
RUN dkp-pacman -S --noconfirm 3ds-dev

# Set working directory
WORKDIR /workspace

# Copy source code
COPY . .

# Build command
CMD ["make"]
