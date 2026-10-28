# Build from the repository root:
#   docker build -f cicd/Dockerfile -t <registry>/rgbcar-ci:latest .
FROM python:3.12-slim-bookworm

ARG PIO_VERSION=6.2.0

ENV PLATFORMIO_CORE_DIR=/opt/platformio
ENV PLATFORMIO_LIBDEPS_DIR=/opt/platformio-libdeps

RUN apt-get update \
  && apt-get install -y --no-install-recommends build-essential ca-certificates curl git \
  && rm -rf /var/lib/apt/lists/*

RUN pip install --no-cache-dir platformio==${PIO_VERSION}

# Pre-fetch the platforms, toolchains and libraries so CI never hits the PlatformIO registry.
WORKDIR /tmp/warmup
COPY platformio.ini .
RUN pio pkg install -e release \
  && rm -rf /tmp/warmup

WORKDIR /
