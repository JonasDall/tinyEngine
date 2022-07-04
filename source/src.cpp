#include <src.hpp>
#include <iostream>

namespace te
{
    class Component{
        private:
        vec2f m_location;
        vec2f m_scale;

        public:
        Component(vec2f location) : m_location{location}{}
    };
}