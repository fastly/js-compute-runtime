#!/bin/bash

# Run all Integration Tests

# Exit on any errors
set -e

if [ $# -gt 0 ]
then
  echo "Running act to test integration tests with:"
  echo "Github Personal Access Token: $1"
  echo "Fastly Token: $2"
  act -j sdktest --secret GITHUB_TOKEN=$1 --secret fastly_token=$2
else
  echo "USAGE: ./_test_integration_tests.sh [Github Personal Access Token to clone https://github.com/fastly/compute-sdk-ci-github-action] [Optional, Only if you want to test C@E Environment: Fastly Token for deploying to C@E services]"
fi

