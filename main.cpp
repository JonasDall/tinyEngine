#include <iostream>
#include <vector>

#include "source/tiny.hpp"
#include "lib/json.hpp"

int main()
{
    tiny::TinyEngine engine({256, 240}, "Game");
    
    engine.Run("asset/tiled/testMap.json");

    return 0;
}