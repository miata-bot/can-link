#!/usr/bin/env bash

set -e

COMPONENT_SRCS=$(ls components/spect-*/*.{c,h})
COMPONENT_HEADERS=$(ls components/spect-*/include/*.h)
MAIN_SRC=$(ls main/*.c)
MAIN_SRC=$(ls include/*.h)

cloc $COMPONENT_SRCS $COMPONENT_HEADERS $MAIN_SRC_HEADERS
