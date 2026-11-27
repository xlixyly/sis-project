#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENV_FILE="${PROJECT_ROOT}/.env"

if [[ ! -f "${ENV_FILE}" ]]; then
    echo "Error: .env file not found at ${ENV_FILE}." >&2
    exit 1
fi

set -a
# shellcheck disable=SC1090
source "${ENV_FILE}"
set +a

: "${DOCKERHUB_USERNAME:?DOCKERHUB_USERNAME is required in .env}"
: "${APP_IMAGE_NAME:?APP_IMAGE_NAME is required in .env}"

TAG="${1:-latest}"
IMAGE_REPO="${DOCKERHUB_USERNAME}/${APP_IMAGE_NAME}"
TARGET_IMAGE="${IMAGE_REPO}:${TAG}"

if [[ ! -f "${PROJECT_ROOT}/Dockerfile" ]]; then
    echo "Error: Dockerfile not found at ${PROJECT_ROOT}/Dockerfile." >&2
    exit 1
fi

docker build -t "${TARGET_IMAGE}" "${PROJECT_ROOT}"
if [[ "${TAG}" != "latest" ]]; then
    docker tag "${TARGET_IMAGE}" "${IMAGE_REPO}:latest"
fi

docker push "${TARGET_IMAGE}"
if [[ "${TAG}" != "latest" ]]; then
    docker push "${IMAGE_REPO}:latest"
fi

echo "Pushed ${TARGET_IMAGE}"
