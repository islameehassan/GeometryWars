#ifndef ENTITY_H 
#define ENTITY_H

#include "Components.hpp"
#include <memory>

class Entity
{
    std::string m_tag;
    bool m_active;
    size_t m_id;

    Entity(const std::string& tag, size_t id);
public:
    std::shared_ptr<cTransform>                     Transform;
    std::shared_ptr<cShape>                         Shape;
    std::shared_ptr<cInput>                         Input;
    std::shared_ptr<cCollision>                     Collision;
    std::shared_ptr<cScore>                         Score;
    std::shared_ptr<cLifespan>                      Lifespan;
    std::shared_ptr<cGravity>                       Gravity;
    std::vector<std::shared_ptr<cSpecialAbility>>   SpecialAbilities;

    void destroy();

    const std::string& tag() const;
    bool isActive() const;
    size_t id() const;


    friend class EntityManager;
};

#endif