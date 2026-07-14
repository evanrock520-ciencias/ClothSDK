FROM python:3.12-slim

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    git \
    libomp-dev \
    libboost-dev \
    libimath-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

RUN pip install --no-cache-dir --upgrade pip setuptools wheel jinja2

COPY . .

ENV CMAKE_BUILD_PARALLEL_LEVEL=4

RUN python scripts/build.py --no-viewer

RUN pip install --no-cache-dir .

CMD ["python"]