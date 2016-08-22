#!/bin/sh

set -ev

docker build -t clang-lanai .
docker run -v $(pwd):/mnt -t clang-lanai clang --target=lanai -S /mnt/example.c
docker run -v $(pwd):/mnt -t clang-lanai clang --target=lanai -c /mnt/example.c
