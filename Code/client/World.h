#pragma once

#include <Services/RunnerService.h>
#include <Services/TransportService.h>
#include <Services/PartyService.h>
#include <Services/CharacterService.h>
#include <Services/OverlayService.h>
#include <Services/CharacterService.h>
#include <Services/MagicService.h>
#include <Services/DebugService.h>

#include <Systems/ModSystem.h>

#include <Structs/ServerSettings.h>
#include <shared_mutex>

struct World : entt::registry
{
    World();
    ~World();

    void Update() noexcept;

    RunnerService& GetRunner() noexcept;
    TransportService& GetTransport() noexcept;
    ModSystem& GetModSystem() noexcept;

    PartyService& GetPartyService() noexcept { return ctx().at<PartyService>(); }
    const PartyService& GetPartyService() const noexcept { return ctx().at<const PartyService>(); }
    CharacterService& GetCharacterService() noexcept { return ctx().at<CharacterService>(); }
    const CharacterService& GetCharacterService() const noexcept { return ctx().at<const CharacterService>(); }
    OverlayService& GetOverlayService() noexcept { return ctx().at<OverlayService>(); }
    const OverlayService& GetOverlayService() const noexcept { return ctx().at<const OverlayService>(); }
    DebugService& GetDebugService() noexcept { return ctx().at<DebugService>(); }
    const DebugService& GetDebugService() const noexcept { return ctx().at<const DebugService>(); }
    MagicService& GetMagicService() noexcept { return ctx().at<MagicService>(); }
    const MagicService& GetMagicService() const noexcept { return ctx().at<const MagicService>(); }

    auto& GetDispatcher() noexcept { return m_dispatcher; }

    const ServerSettings& GetServerSettings() const noexcept { return m_serverSettings; }
    void SetServerSettings(ServerSettings aServerSettings) noexcept { m_serverSettings = aServerSettings; }

    [[nodiscard]] uint64_t GetTick() const noexcept;

    // Entity lookup methods
    [[nodiscard]] std::optional<entt::entity> GetEntityByServerId(uint32_t aServerId) const noexcept;
    [[nodiscard]] std::optional<entt::entity> GetEntityByFormId(uint32_t aFormId) const noexcept;
    
    // Entity registration methods
    void RegisterEntityServerId(entt::entity aEntity, uint32_t aServerId) noexcept;
    void RegisterEntityFormId(entt::entity aEntity, uint32_t aFormId) noexcept;
    
    // Entity unregistration methods
    void UnregisterEntityServerId(uint32_t aServerId) noexcept;
    void UnregisterEntityFormId(uint32_t aFormId) noexcept;
    void UnregisterEntity(entt::entity aEntity) noexcept;

    static void Create() noexcept;
    [[nodiscard]] static World& Get() noexcept;

private:
    entt::dispatcher m_dispatcher;
    RunnerService m_runner;
    TransportService m_transport;
    ModSystem m_modSystem;
    ServerSettings m_serverSettings{};

    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    
    // Entity lookup maps
    TiltedPhoques::Map<uint32_t, entt::entity> m_serverIdMap;   // ServerId -> Entity
    TiltedPhoques::Map<uint32_t, entt::entity> m_formIdMap;     // FormId -> Entity
    TiltedPhoques::Map<entt::entity, std::pair<uint32_t, uint32_t>> m_entityMap; // Entity -> (FormId, ServerId)
    
    // Thread safety
    mutable std::shared_mutex m_lookupMutex;
};
