FROM python:3.12-slim

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    git \
    libomp-dev \
    libboost-dev \
    libimath-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/alembic/alembic.git && \
    cd alembic && git checkout 1.8.11 && \
    mkdir build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DUSE_TESTS=OFF && \
    make -j$(nproc) && make install && \
    cd / && rm -rf alembic

RUN pip install --no-cache-dir \
    numpy matplotlib imageio tqdm \
    pybind11 mypy pytest

RUN git clone https://github.com/evanrock520-ciencias/Tissu.git && \
    cd Tissu && git checkout blender && \
    mkdir build && \
    cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DTISSU_BUILD_VIEWER=OFF \
        -DCMAKE_PREFIX_PATH=/usr/local && \
    cmake --build build -j$(nproc) && \
    pip install -e . && \
    rm -rf build