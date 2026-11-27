# Installation & Setup Guide

## Prerequisites
* Docker Engine / Docker Desktop (`sudo -g docker` in WSL2)
* Optional Native Toolchain: `g++`, `cmake`, `pkg-config`, `libpqxx-dev`, `shellcheck`

## 1. Clone & Configure
```bash
git clone [https://github.com/xlixyly/sis-project.git](https://github.com/xlixyly/sis-project.git)
cd sis-project && git checkout develop
cp .env.example .env
```

## 2. Build & Start Stack
```bash
sudo -g docker ./scripts/start.sh
sudo -g docker ./scripts/status.sh
```

## 3. Native Tests & Docker Hub Push
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
sudo -g docker ./scripts/push.sh v1.0
```
