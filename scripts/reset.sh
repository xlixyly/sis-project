#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
ENV_FILE="${PROJECT_ROOT}/.env"

if [[ -f "${ENV_FILE}" ]]; then
    set -a
    # shellcheck disable=SC1090
    source "${ENV_FILE}"
    set +a
fi

VOL_NAME="${DB_VOLUME:-sis_pgdata}"

"${SCRIPT_DIR}/stop.sh"

if docker volume inspect "${VOL_NAME}" >/dev/null 2>&1; then
    docker volume rm "${VOL_NAME}" >/dev/null
fi

echo "Reset complete: containers, network, and volume '${VOL_NAME}' removed."
