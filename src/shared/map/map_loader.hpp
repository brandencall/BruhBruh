// shared/map/map_loader.hpp
#pragma once
#include "map_types.hpp"
#include <fstream>
#include <sstream>

/*
Map file format:

# map01.txt
# WALL x y w h
# SPAWN idx x y

WALL  0    0    800  16     # top border
WALL  0    584  800  16     # bottom border
WALL  0    0    16   600    # left border
WALL  784  0    16   600    # right border
WALL  200  100  16   200    # interior wall
WALL  300  300  200  16     # interior wall

SPAWN 0  100  100
SPAWN 1  700  500
*/

inline MapData LoadMap(const std::string &path) {
    MapData map;
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue; // skip comments

        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "WALL") {
            float x, y, w, h;
            ss >> x >> y >> w >> h;
            map.walls.push_back({{x, y}, {x + w, y + h}});
        } else if (type == "SPAWN") {
            int idx;
            float x, y;
            ss >> idx >> x >> y;
            if (idx < MAX_PLAYERS)
                map.spawnPoints[idx] = {x, y};
        }
    }
    return map;
}
