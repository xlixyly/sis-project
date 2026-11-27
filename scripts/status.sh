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
VOL_NAME="${DB_VOLUME:-sis_pgdata}"
DB_CONTAINER="${DB_CONTAINER_NAME:-sis_postgres}"
APP_CONTAINER="${APP_CONTAINER_NAME:-sis_app}"

echo "=== Docker Network (${NET_NAME}) ==="
docker network inspect "${NET_NAME}" --format '{{.Name}} ({{.Driver}})' 2>/dev/null || echo "Not found"

echo "=== Docker Volume (${VOL_NAME}) ==="
docker volume inspect "${VOL_NAME}" --format '{{.Name}} ({{.Mountpoint}})' 2>/dev/null || echo "Not found"

echo "=== Containers ==="
docker ps -a --filter "name=${DB_CONTAINER}" --filter "name=${APP_CONTAINER}" \
    --format "table {{.Names}}\t{{.Image}}\t{{.Status}}\t{{.Ports}}"
