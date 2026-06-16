#!/bin/bash

echo "Cleaning networking..."

sudo ip link delete mini0 2>/dev/null

echo "Done."