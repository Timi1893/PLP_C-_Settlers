#pragma once
#include <map>
#include <string>
#include "types.h"

struct Player {
    int id;                  
    std::string name;
    int points = 0;
    std::map<Resource, int> resources;
};

