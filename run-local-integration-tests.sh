#!/bin/bash

# Run all Integration Tests

# Exit on any errors
set -euo pipefail

github_token="$1"
fastly_token="$2"

if [ $# -gt 0 ]; then
  echo "Running act to test integration tests with:"
  echo "Github Personal Access Token: $github_token"
  echo "Fastly Token: $fastly_token"
  act -j sdktest --secret GITHUB_TOKEN="github_token" --secret fastly_token="$fastly_token"
else
  echo "USAGE: ./_test_integration_tests.sh [Github Personal Access Token to clone https://github.com/fastly/compute-sdk-ci-github-action] [Optional, Only if you want to test C@E Environment: Fastly Token for deploying to C@E services]"
fi

