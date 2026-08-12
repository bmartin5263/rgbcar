#!/bin/sh -e

if [ $# -ne 1 ]; then
  echo "Usage: $0 <variables.yml>"
  exit 1
fi

VARIABLES_YML="$1"


yes | fly -t rgbcar set-pipeline -c cicd/pipeline.yml -p rgbcar -l "${VARIABLES_YML}"