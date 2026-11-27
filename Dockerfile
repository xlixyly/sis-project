FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN echo 'Acquire::ForceIPv4 "true";' > /etc/apt/apt.conf.d/99force-ipv4 \
    && sed -i 's|http://archive.ubuntu.com/ubuntu|http://mirrors.edge.kernel.org/ubuntu|g; s|http://security.ubuntu.com/ubuntu|http://mirrors.edge.kernel.org/ubuntu|g' /etc/apt/sources.list.d/ubuntu.sources \
    && apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    pkg-config \
    libpqxx-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j"$(nproc)" \
    && ctest --test-dir build --output-on-failure

FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
RUN echo 'Acquire::ForceIPv4 "true";' > /etc/apt/apt.conf.d/99force-ipv4 \
    && sed -i 's|http://archive.ubuntu.com/ubuntu|http://mirrors.edge.kernel.org/ubuntu|g; s|http://security.ubuntu.com/ubuntu|http://mirrors.edge.kernel.org/ubuntu|g' /etc/apt/sources.list.d/ubuntu.sources \
    && apt-get update && apt-get install -y --no-install-recommends \
    libpqxx-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/sis /usr/local/bin/sis
COPY --from=builder /app/build/sis_tests /usr/local/bin/sis_tests

ENTRYPOINT ["sis"]
CMD ["--help"]
