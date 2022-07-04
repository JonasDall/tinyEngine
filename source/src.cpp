#include <iostream>
#include "src.hpp"
#include "physics.hpp"

namespace te
{
    //Constructor
    Component::Component(vec2f position) : m_position{position}{}

    void Component::setPosition(vec2f newPosition)
    {
        m_position = newPosition;
        return;
    }

    vec2f Component::getPosition()
    {
        return m_position;
    }
}