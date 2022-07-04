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

    class Component
    {
        private:
        vec2f m_position;
        vec2f m_scale;
        bool dirty;

        public:
        Component() = default;
        Component(vec2f position);

        void setPosition(vec2f newPosition);
        vec2f getPosition();
    };
}