# Build stage
FROM ubuntu:18.04

RUN echo ========== Install dependencies ========== \
  && apt-get update && apt-get install -y \
    clang \
    gdb \
    git \
    make \
    perl \
    unzip \
    wget \
    libasound2-dev \
    libavahi-compat-libdnssd-dev \
    libfaac-dev \
    libopus-dev \
    libsqlite3-dev \
    libssl-dev \
    openssh-server \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /build
