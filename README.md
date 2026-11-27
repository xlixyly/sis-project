# Student Information System (SIS)

A command-line Student Information System implemented in modern C++ with `libpqxx`, backed by PostgreSQL, orchestrated via Bash scripts, and containerized with a multi-stage `Dockerfile`.

## Architecture & Tech Stack

* **Application Layer:** C++ CLI (`sis`) using `libpqxx` with parameterized queries (`exec_params`), input validation, and structured logging (`[INFO]` / `[ERROR]`).
* **Database Layer:** PostgreSQL (`sis_db`) initialized from `sql/init.sql` (`students` table and `idx_students_department` index) with persistent storage on the `sis_pgdata` Docker volume.
* **Networking & Orchestration:** Isolated Docker bridge network (`sis_net`) and helper scripts in `scripts/` (`start.sh`, `stop.sh`, `reset.sh`, `status.sh`, `push.sh`).
* **Container Registry & CI:** Multi-stage `Dockerfile` (Ubuntu 24.04 builder + runtime stages) published to Docker Hub (`hasanly/sis-project-app`) and validated with GitHub Actions (`shellcheck`, CMake/CTest, and Docker build).

## Repository Layout

* `include/sis.hpp` — Core data structures, validation helpers, repository interface, and CLI entry point.
* `src/sis.cpp` — Input validation, environment config (`DbConfig::from_env`), PostgreSQL CRUD operations, and CLI argument parser.
* `src/main.cpp` — Executable entry point wiring `DbConfig` and `StudentRepository` to `run_cli`.
* `tests/test_sis.cpp` — Unit tests (`ctest`) and optional database integration tests (`RUN_DB_TESTS=1`).
* `sql/init.sql` — Database schema and index initialization script.
* `scripts/` — Lifecycle management scripts (`start.sh`, `stop.sh`, `reset.sh`, `status.sh`, `push.sh`).
* `Dockerfile` — Multi-stage build and runtime image definition.
* `INSTALL.md` — Setup, environment configuration, and local/Docker build instructions.
* `MANUAL.md` — CLI usage guide, CRUD examples, operational scripts, and troubleshooting.

## Quick Start

1. Copy `.env.example` to `.env` and adjust credentials if needed: `cp .env.example .env`
2. Start PostgreSQL and build the Docker image: `sudo -g docker ./scripts/start.sh`
3. List students over `sis_net`:
   `set -a && source .env && set +a && sudo -g docker docker run --rm --network "${DOCKER_NETWORK:-sis_net}" -e "DB_HOST=${DB_CONTAINER_NAME:-sis_postgres}" -e "DB_PORT=5432" -e "DB_NAME=${POSTGRES_DB}" -e "DB_USER=${POSTGRES_USER}" -e "DB_PASSWORD=${POSTGRES_PASSWORD}" "${DOCKERHUB_USERNAME:-hasanly}/${APP_IMAGE_NAME:-sis-project-app}:latest" list`
