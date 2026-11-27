#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENV_FILE="${PROJECT_ROOT}/.env"

if [[ -f "${ENV_FILE}" ]]; then
    set -a
    # shellcheck disable=SC1090
    source "${ENV_FILE}"
    set +a
fi

NET_NAME="${DOCKER_NETWORK:-sis_net}"
DB_CONTAINER="${DB_CONTAINER_NAME:-sis_postgres}"
APP_CONTAINER="${APP_CONTAINER_NAME:-sis_app}"

for container in "${APP_CONTAINER}" "${DB_CONTAINER}"; do
    if docker ps -a --format '{{.Names}}' | grep -qx "${container}"; then
        docker rm -f "${container}" >/dev/null
    fi
done

if docker network inspect "${NET_NAME}" >/dev/null 2>&1; then
    docker network rm "${NET_NAME}" >/dev/null
fi

echo "Stopped containers and removed network '${NET_NAME}'."
