#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENV_FILE="${PROJECT_ROOT}/.env"

if [[ ! -f "${ENV_FILE}" ]]; then
    echo "Error: .env file not found at ${ENV_FILE}. Copy .env.example to .env first." >&2
    exit 1
fi

set -a
# shellcheck disable=SC1090
source "${ENV_FILE}"
set +a

: "${DB_PORT:?DB_PORT is required in .env}"
: "${POSTGRES_DB:?POSTGRES_DB is required in .env}"
: "${POSTGRES_USER:?POSTGRES_USER is required in .env}"
: "${POSTGRES_PASSWORD:?POSTGRES_PASSWORD is required in .env}"
: "${DOCKERHUB_USERNAME:?DOCKERHUB_USERNAME is required in .env}"
: "${APP_IMAGE_NAME:?APP_IMAGE_NAME is required in .env}"

NET_NAME="${DOCKER_NETWORK:-sis_net}"
VOL_NAME="${DB_VOLUME:-sis_pgdata}"
DB_CONTAINER="${DB_CONTAINER_NAME:-sis_postgres}"
APP_CONTAINER="${APP_CONTAINER_NAME:-sis_app}"
POSTGRES_IMAGE="${POSTGRES_IMAGE:-postgres:16-alpine}"
LOCAL_IMAGE="${DOCKERHUB_USERNAME}/${APP_IMAGE_NAME}:latest"
SCHEMA_FILE="${PROJECT_ROOT}/sql/init.sql"

cleanup_on_error() {
    local exit_code=$?
    if [[ "${exit_code}" -ne 0 ]]; then
        echo "Error: start.sh failed with exit code ${exit_code}." >&2
    fi
}
trap cleanup_on_error EXIT

if ! docker network inspect "${NET_NAME}" >/dev/null 2>&1; then
    docker network create "${NET_NAME}" >/dev/null
fi

if ! docker volume inspect "${VOL_NAME}" >/dev/null 2>&1; then
    docker volume create "${VOL_NAME}" >/dev/null
fi

if docker ps -a --format '{{.Names}}' | grep -qx "${DB_CONTAINER}"; then
    if ! docker ps --format '{{.Names}}' | grep -qx "${DB_CONTAINER}"; then
        docker start "${DB_CONTAINER}" >/dev/null
    fi
else
    docker run -d \
        --name "${DB_CONTAINER}" \
        --network "${NET_NAME}" \
        -e "POSTGRES_DB=${POSTGRES_DB}" \
        -e "POSTGRES_USER=${POSTGRES_USER}" \
        -e "POSTGRES_PASSWORD=${POSTGRES_PASSWORD}" \
        -p "${DB_PORT}:5432" \
        -v "${VOL_NAME}:/var/lib/postgresql/data" \
        "${POSTGRES_IMAGE}" >/dev/null
fi

echo "Waiting for PostgreSQL database '${POSTGRES_DB}' to become ready..."
retries=30
until docker exec "${DB_CONTAINER}" pg_isready -U "${POSTGRES_USER}" -d "${POSTGRES_DB}" >/dev/null 2>&1 \
    && docker exec -e "PGPASSWORD=${POSTGRES_PASSWORD}" "${DB_CONTAINER}" \
        psql -h localhost -U "${POSTGRES_USER}" -d "${POSTGRES_DB}" -c "SELECT 1;" >/dev/null 2>&1; do
    retries=$((retries - 1))
    if [[ "${retries}" -le 0 ]]; then
        echo "Error: PostgreSQL did not become ready in time." >&2
        exit 1
    fi
    sleep 1
done

if [[ -f "${SCHEMA_FILE}" ]]; then
    docker exec -i -e "PGPASSWORD=${POSTGRES_PASSWORD}" "${DB_CONTAINER}" \
        psql -h localhost -U "${POSTGRES_USER}" -d "${POSTGRES_DB}" -v ON_ERROR_STOP=1 < "${SCHEMA_FILE}" >/dev/null
fi

if [[ -f "${PROJECT_ROOT}/Dockerfile" ]]; then
    docker build -t "${LOCAL_IMAGE}" "${PROJECT_ROOT}"
fi

if docker image inspect "${LOCAL_IMAGE}" >/dev/null 2>&1; then
    if docker ps -a --format '{{.Names}}' | grep -qx "${APP_CONTAINER}"; then
        docker rm -f "${APP_CONTAINER}" >/dev/null
    fi
    docker run -d \
        --name "${APP_CONTAINER}" \
        --network "${NET_NAME}" \
        -e "DB_HOST=${DB_CONTAINER}" \
        -e "DB_PORT=5432" \
        -e "DB_NAME=${POSTGRES_DB}" \
        -e "DB_USER=${POSTGRES_USER}" \
        -e "DB_PASSWORD=${POSTGRES_PASSWORD}" \
        "${LOCAL_IMAGE}" tail -f /dev/null >/dev/null
fi

echo "SIS infrastructure is up and ready."
