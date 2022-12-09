#include <iostream>
#include <vector>

#include "source/tiny.hpp"
#include "lib/json.hpp"

int main()
{
    tiny::TinyEngine engine({256, 240}, "Game", "asset/tiled/testMap.json");
    
    engine.Run();

    engine.LoadLevel("asset/tiled/testMap.json", "test");


    // engine.GetLevel()->DisplayNames();

    return 0;
}