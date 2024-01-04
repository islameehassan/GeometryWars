#include "EntityManager.h"

EntityManager::EntityManager()
{
    m_entitiesVec.reserve(100);
    m_toAdd.reserve(100);

    m_totalEntitites = 1;   // player
}

std::shared_ptr<Entity> EntityManager::add(const std::string& tag)
{
    auto e = std::shared_ptr<Entity>(new Entity(tag, m_totalEntitites++));

    m_toAdd.push_back(e);

    return e;
}

void EntityManager::update()
{
    // commit the new elements in m_toAdd to m_entititeVec
    for(auto& e: m_toAdd)
    {
        m_entitiesVec.push_back(e);
        m_entitiesMap[e->tag()].push_back(e);
    }
     
    m_toAdd.clear();


    // remove elements that are marked as inactive
    m_entitiesVec.erase(std::remove_if(m_entitiesVec.begin(), m_entitiesVec.end(), [](std::shared_ptr<Entity>& e){ return !e->isActive();}), 
                        m_entitiesVec.end());
    
    for(auto& [tag, entityVec]: m_entitiesMap)
    {
        entityVec.erase(std::remove_if(entityVec.begin(), entityVec.end(), [](std::shared_ptr<Entity>& e){ return !e->isActive();}), 
                        entityVec.end());  
  
    }
}

EntityVec& EntityManager::getEntities() 
{
    return m_entitiesVec;
}

EntityVec& EntityManager::getEntitiesByTag(const std::string& tag)
{
    return m_entitiesMap[tag];
}