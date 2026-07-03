GIT_HASH=$(git rev-parse --short HEAD)

DOCKER_BUILDKIT=1 docker build \
  --build-arg GIT_COMMIT_HASH=${GIT_HASH} \
  --build-arg BUILD_DATE=$(date -u +"%Y-%m-%dT%H:%M:%SZ") \
  -t rp-qt6120-x86-linux:${GIT_HASH} \
  .
