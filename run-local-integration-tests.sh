#!/bin/bash

# Run all Integration Tests

# Exit on any errors
set -euo pipefail

if [ "$#" -lt 1 ]; then
  cat <<EOF
USAGE: $0 github-token [fastly-token]

  github-token    Github Personal Access Token to clone https://github.com/fastly/compute-sdk-ci-github-action
  fastly-token    Optional, Only if you want to test C@E Environment: Fastly Token for deploying to C@E services"
EOF
  exit 1
fi

github_token="$1"
fastly_token="${2:-}"

echo "Running act to test integration tests with:"
echo "Github Personal Access Token: $github_token"
echo "Fastly Token: $fastly_token"
act -j sdktest --secret GITHUB_TOKEN="$github_token" --secret fastly_token="$fastly_token"
