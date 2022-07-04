#pragma once

namespace te
{
    template <typename T>
    struct vec2
    {
        T x;
        T y;
    };

    typedef vec2<int> vec2i;
    typedef vec2<float> vec2f;
}