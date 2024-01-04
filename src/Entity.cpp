#include "Entity.h"

Entity::Entity(const std::string& tag, size_t id): m_tag(tag), m_id(id)
{
    m_active = true;
}

void Entity::destroy()
{
    m_active = false;
}

const std::string& Entity::tag() const 
{
    return m_tag;
}

bool Entity::isActive() const 
{
    return m_active;
}

size_t Entity::id() const 
{
    return m_id;
}