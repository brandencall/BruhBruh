#!/bin/bash
cd "$(dirname "$0")"
export SteamAppId=480
export SteamGameId=480

# Find and preload the Steam overlay
STEAM_ROOT="$HOME/.steam/steam"
OVERLAY_LIB="$STEAM_ROOT/ubuntu12_64/gameoverlayrenderer.so"
if [ -f "$OVERLAY_LIB" ]; then
    export LD_PRELOAD="$OVERLAY_LIB:$LD_PRELOAD"
fi

echo "starting game" > log.txt
./build/BruhBruh >> log.txt 2>&1
echo "finished" >> log.txt
