#include <TiltedOnlinePCH.h>

#include "World.h"

#include <Services/DiscoveryService.h>
#include <Services/InputService.h>
#include <Services/TransportService.h>
#include <Services/RunnerService.h>
#include <Services/ImguiService.h>
#include <Services/PapyrusService.h>
#include <Services/DiscordService.h>
#include <Services/ObjectService.h>
#include <Services/QuestService.h>
#include <Services/ActorValueService.h>
#include <Services/InventoryService.h>
#include <Services/MagicService.h>
#include <Services/CommandService.h>
#include <Services/CalendarService.h>
#include <Services/StringCacheService.h>
#include <Services/PlayerService.h>
#include <Services/CombatService.h>
#include <Services/WeatherService.h>
#include <Services/MapService.h>

#include <Events/PreUpdateEvent.h>
#include <Events/UpdateEvent.h>

#include <ModCompat/BehaviorVar.h>  

World::World()
    : m_runner(m_dispatcher)
    , m_transport(*this, m_dispatcher)
    , m_modSystem(m_dispatcher)
    , m_lastFrameTime{std::chrono::high_resolution_clock::now()}
{
    ctx().emplace<ImguiService>();
    ctx().emplace<DiscoveryService>(*this, m_dispatcher);
    ctx().emplace<OverlayService>(*this, m_transport, m_dispatcher);
    ctx().emplace<InputService>(ctx().at<OverlayService>());
    ctx().emplace<CharacterService>(*this, m_dispatcher, m_transport);
    ctx().emplace<DebugService>(m_dispatcher, *this, m_transport, ctx().at<ImguiService>());
    ctx().emplace<PapyrusService>(m_dispatcher);
    ctx().emplace<DiscordService>(m_dispatcher);
    ctx().emplace<ObjectService>(*this, m_dispatcher, m_transport);
    ctx().emplace<CalendarService>(*this, m_dispatcher, m_transport);
    ctx().emplace<QuestService>(*this, m_dispatcher);
    ctx().emplace<PartyService>(*this, m_dispatcher, m_transport);
    ctx().emplace<ActorValueService>(*this, m_dispatcher, m_transport);
    ctx().emplace<InventoryService>(*this, m_dispatcher, m_transport);
    ctx().emplace<MagicService>(*this, m_dispatcher, m_transport);
    ctx().emplace<CommandService>(*this, m_transport, m_dispatcher);
    ctx().emplace<PlayerService>(*this, m_dispatcher, m_transport);
    ctx().emplace<StringCacheService>(m_dispatcher);
    ctx().emplace<CombatService>(*this, m_transport, m_dispatcher);
    ctx().emplace<WeatherService>(*this, m_transport, m_dispatcher);
    ctx().emplace<MapService>(*this, m_dispatcher, m_transport);

    BehaviorVar::Get()->Init();
}

World::~World() = default;

void World::Update() noexcept
{
    const auto cNow = std::chrono::high_resolution_clock::now();
    const auto cDelta = cNow - m_lastFrameTime;
    m_lastFrameTime = cNow;

    const auto cDeltaSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(cDelta).count();

    m_dispatcher.trigger(PreUpdateEvent(cDeltaSeconds));

    // Force run this before so we get the tasks scheduled to run
    m_runner.OnUpdate(UpdateEvent(cDeltaSeconds));
    m_dispatcher.trigger(UpdateEvent(cDeltaSeconds));
}

RunnerService& World::GetRunner() noexcept
{
    return m_runner;
}

TransportService& World::GetTransport() noexcept
{
    return m_transport;
}

ModSystem& World::GetModSystem() noexcept
{
    return m_modSystem;
}

uint64_t World::GetTick() const noexcept
{
    return m_transport.GetClock().GetCurrentTick();
}

void World::Create() noexcept
{
    if (!entt::locator<World>::has_value())
    {
        entt::locator<World>::emplace();
    }
}

World& World::Get() noexcept
{
    return entt::locator<World>::value();
}

// Entity lookup methods
std::optional<entt::entity> World::GetEntityByServerId(uint32_t aServerId) const noexcept
{
    std::shared_lock<std::shared_mutex> lock(m_lookupMutex);
    
    const auto it = m_serverIdMap.find(aServerId);
    if (it != m_serverIdMap.end())
        return it->second;
    
    return std::nullopt;
}

std::optional<entt::entity> World::GetEntityByFormId(uint32_t aFormId) const noexcept
{
    std::shared_lock<std::shared_mutex> lock(m_lookupMutex);
    
    const auto it = m_formIdMap.find(aFormId);
    if (it != m_formIdMap.end())
        return it->second;
    
    return std::nullopt;
}

// Entity registration methods
void World::RegisterEntityServerId(entt::entity aEntity, uint32_t aServerId) noexcept
{
    std::unique_lock<std::shared_mutex> lock(m_lookupMutex);
    
    // Add to server ID map
    m_serverIdMap[aServerId] = aEntity;
    
    // Update entity mapping
    auto entityIt = m_entityMap.find(aEntity);
    if (entityIt != m_entityMap.end())
    {
        // Create a new pair with updated ServerId and existing FormId
        uint32_t formId = entityIt->second.first;
        m_entityMap.erase(entityIt);
        m_entityMap[aEntity] = std::make_pair(formId, aServerId);
    }
    else
    {
        // Create new entry with default FormId (will be updated later if needed)
        m_entityMap[aEntity] = std::make_pair(0, aServerId);
    }
    
    spdlog::debug("Registered entity {:X} with server ID {:X}", static_cast<uint32_t>(aEntity), aServerId);
}

void World::RegisterEntityFormId(entt::entity aEntity, uint32_t aFormId) noexcept
{
    std::unique_lock<std::shared_mutex> lock(m_lookupMutex);
    
    // Add to form ID map
    m_formIdMap[aFormId] = aEntity;
    
    // Update entity mapping
    auto entityIt = m_entityMap.find(aEntity);
    if (entityIt != m_entityMap.end())
    {
        // Create a new pair with existing ServerId and updated FormId
        uint32_t serverId = entityIt->second.second;
        m_entityMap.erase(entityIt);
        m_entityMap[aEntity] = std::make_pair(aFormId, serverId);
    }
    else
    {
        // Create new entry with default ServerId (will be updated later if needed)
        m_entityMap[aEntity] = std::make_pair(aFormId, 0);
    }
    
    spdlog::debug("Registered entity {:X} with form ID {:X}", static_cast<uint32_t>(aEntity), aFormId);
}

// Entity unregistration methods
void World::UnregisterEntityServerId(uint32_t aServerId) noexcept
{
    std::unique_lock<std::shared_mutex> lock(m_lookupMutex);
    
    // Find the entity with this server ID
    auto it = m_serverIdMap.find(aServerId);
    if (it != m_serverIdMap.end())
    {
        auto entity = it->second;
        
        // Remove from server ID map
        m_serverIdMap.erase(it);
        
        // Update entity mapping if it exists
        auto entityIt = m_entityMap.find(entity);
        if (entityIt != m_entityMap.end())
        {
            // If this entity had both ServerId and FormId, create a new entry with just FormId
            if (entityIt->second.first != 0)
            {
                uint32_t formId = entityIt->second.first;
                m_entityMap.erase(entityIt);
                m_entityMap[entity] = std::make_pair(formId, 0);
            }
            else
                // If it only had ServerId, remove the entity mapping entirely
                m_entityMap.erase(entityIt);
        }
        
        spdlog::debug("Unregistered server ID {:X} from entity {:X}", aServerId, static_cast<uint32_t>(entity));
    }
}

void World::UnregisterEntityFormId(uint32_t aFormId) noexcept
{
    std::unique_lock<std::shared_mutex> lock(m_lookupMutex);
    
    // Find the entity with this form ID
    auto it = m_formIdMap.find(aFormId);
    if (it != m_formIdMap.end())
    {
        auto entity = it->second;
        
        // Remove from form ID map
        m_formIdMap.erase(it);
        
        // Update entity mapping if it exists
        auto entityIt = m_entityMap.find(entity);
        if (entityIt != m_entityMap.end())
        {
            // If this entity had both ServerId and FormId, create a new entry with just ServerId
            if (entityIt->second.second != 0)
            {
                uint32_t serverId = entityIt->second.second;
                m_entityMap.erase(entityIt);
                m_entityMap[entity] = std::make_pair(0, serverId);
            }
            else
                // If it only had FormId, remove the entity mapping entirely
                m_entityMap.erase(entityIt);
        }
        
        spdlog::debug("Unregistered form ID {:X} from entity {:X}", aFormId, static_cast<uint32_t>(entity));
    }
}

void World::UnregisterEntity(entt::entity aEntity) noexcept
{
    std::unique_lock<std::shared_mutex> lock(m_lookupMutex);
    
    // Find this entity in the entity map
    auto entityIt = m_entityMap.find(aEntity);
    if (entityIt != m_entityMap.end())
    {
        const auto& [formId, serverId] = entityIt->second;
        
        // Remove from form ID map if it has a form ID
        if (formId != 0)
            m_formIdMap.erase(formId);
        
        // Remove from server ID map if it has a server ID
        if (serverId != 0)
            m_serverIdMap.erase(serverId);
        
        // Remove from entity map
        m_entityMap.erase(entityIt);
        
        spdlog::debug("Unregistered entity {:X} with form ID {:X} and server ID {:X}", 
                     static_cast<uint32_t>(aEntity), formId, serverId);
    }
}
