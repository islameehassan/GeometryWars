#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

class Entity;

#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include "Entity.h"

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;

class EntityManager
{
    EntityVec m_entitiesVec;
    EntityMap m_entitiesMap;
    EntityVec m_toAdd;

    size_t m_totalEntitites;
public:
    EntityManager();

    void update();

    std::shared_ptr<Entity> add(const std::string& tag);
    EntityVec& getEntities();
    EntityVec& getEntitiesByTag(const std::string& tag);
};

#endif