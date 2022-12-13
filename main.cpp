#include <iostream>
#include <vector>

#include "source/tiny.hpp"
#include "lib/json.hpp"

int main()
{
    tiny::TinyEngine engine({256, 240}, "Game");
    
    engine.Run("asset/tiled/testMap.json");

    tiny::int2d a{10, 10};
    tiny::int2d b{5, 5};

    tiny::int2d c = a - b;

    std::cout << c.x << ' ' << c.y << '\n';

    return 0;
}