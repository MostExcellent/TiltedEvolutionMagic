#include <TiltedCore/Stl.hpp>
#include <TiltedCore/Allocator.hpp>
#include <TiltedCore/Buffer.hpp>
#include <TiltedCore/Serialization.hpp>

#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "StringCache.h"
#include "Messages/StringCacheUpdate.h"

#include <catch2/catch.hpp>

#include <Messages/ClientMessageFactory.h>
#include <Messages/ServerMessageFactory.h>
#include <Structs/Vector2_NetQuantize.h>
#include <Structs/NetActionEvent.h>
#include <Structs/GameId.h>
#include <Structs/ReferenceUpdate.h>

#include <TiltedCore/Math.hpp>
#include <TiltedCore/Platform.hpp>

using namespace TiltedPhoques;

TEST_CASE("Encoding factory", "[encoding.factory]")
{
    Buffer buff(1000);

    {
        AuthenticationRequest request;
        request.Token = "TesSt";

        Buffer::Writer writer(&buff);
        request.Serialize(writer);

        Buffer::Reader reader(&buff);

        const ClientMessageFactory factory;
        auto pMessage = factory.Extract(reader);

        REQUIRE(pMessage);
        REQUIRE(pMessage->GetOpcode() == request.GetOpcode());

        auto pRequest = CastUnique<AuthenticationRequest>(std::move(pMessage));
        REQUIRE(pRequest->Token == request.Token);
    }

    {
        PartyAcceptInviteRequest request;
        request.InviterId = 123456;

        Buffer::Writer writer(&buff);
        request.Serialize(writer);

        Buffer::Reader reader(&buff);

        const ClientMessageFactory factory;
        auto pMessage = factory.Extract(reader);

        REQUIRE(pMessage);
        REQUIRE(pMessage->GetOpcode() == request.GetOpcode());

        auto pRequest = CastUnique<PartyAcceptInviteRequest>(std::move(pMessage));
        REQUIRE(pRequest->InviterId == request.InviterId);
    }
}

TEST_CASE("Static structures", "[encoding.static]")
{
    GIVEN("GameId")
    {
        GameId sendObjects, recvObjects;
        sendObjects.ModId = 1456987;
        sendObjects.BaseId = 0x789654;

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendObjects.Serialize(writer);

            Buffer::Reader reader(&buff);
            recvObjects.Deserialize(reader);

            REQUIRE(sendObjects == recvObjects);
        }
    }

    GIVEN("Vector3_NetQuantize")
    {
        Vector3_NetQuantize sendObjects, recvObjects;
        sendObjects.x = 142.56f;
        sendObjects.y = 45687.7f;
        sendObjects.z = -142.56f;

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendObjects.Serialize(writer);

            Buffer::Reader reader(&buff);
            recvObjects.Deserialize(reader);

            REQUIRE(sendObjects == recvObjects);
        }
    }

    GIVEN("Vector2_NetQuantize")
    {
        Vector2_NetQuantize sendObjects, recvObjects;
        sendObjects.x = 1000.89f;
        sendObjects.y = -485632.75f;

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendObjects.Serialize(writer);

            Buffer::Reader reader(&buff);
            recvObjects.Deserialize(reader);

            REQUIRE(sendObjects == recvObjects);
        }
    }

    GIVEN("Rotator2_NetQuantize")
    {
        Rotator2_NetQuantize sendObjects, recvObjects;
        sendObjects.x = 1.89f;
        sendObjects.y = TiltedPhoques::Pi * 2.0f;

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendObjects.Serialize(writer);

            Buffer::Reader reader(&buff);
            recvObjects.Deserialize(reader);

            REQUIRE(sendObjects == recvObjects);
        }
    }

    GIVEN("Rotator2_NetQuantize needing wrap")
    {
        // This test is a bit dangerous as floating errors can lead to sendObjects != recvObjects but the difference is minuscule so we don't care abut such cases
        Rotator2_NetQuantize sendObjects, recvObjects;
        sendObjects.x = -1.87f;
        sendObjects.y = static_cast<float>(TiltedPhoques::Pi) * 18.0f + 3.6f;

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendObjects.Serialize(writer);

            Buffer::Reader reader(&buff);
            recvObjects.Deserialize(reader);

            REQUIRE(sendObjects == recvObjects);
        }
    }
}

TEST_CASE("Differential structures", "[encoding.differential]")
{
    GIVEN("Full NetActionEvent")
    {
        NetActionEvent sendAction, recvAction;

        sendAction.ActionId = GameId(1, 42);
        sendAction.State1 = 6547;
        sendAction.Tick = 48;
        sendAction.ActorId = 12345678;
        sendAction.EventName = "test";
        sendAction.IdleId = GameId(2, 87964);
        sendAction.State2 = 8963;
        sendAction.TargetEventName = "toast";
        sendAction.TargetId = 963741;
        sendAction.Type = 4;

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendAction.GenerateDifferential(recvAction, writer);

            Buffer::Reader reader(&buff);
            recvAction.ApplyDifferential(reader);

            REQUIRE(sendAction == recvAction);
        }

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendAction.EventName = "Plot twist !";

            sendAction.GenerateDifferential(recvAction, writer);

            Buffer::Reader reader(&buff);
            recvAction.ApplyDifferential(reader);

            REQUIRE(sendAction == recvAction);
        }
    }

    GIVEN("A single cached event name")
    {
        NetActionEvent sendAction, recvAction;

        TP_UNUSED(StringCache::Get().Add("test"))

        sendAction.ActionId = GameId(1, 42);
        sendAction.State1 = 6547;
        sendAction.Tick = 48;
        sendAction.ActorId = 12345678;
        sendAction.EventName = "test";
        sendAction.IdleId = GameId(2, 87964);
        sendAction.State2 = 8963;
        sendAction.TargetEventName = "toast";
        sendAction.TargetId = 963741;
        sendAction.Type = 4;

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendAction.GenerateDifferential(recvAction, writer);

            Buffer::Reader reader(&buff);
            recvAction.ApplyDifferential(reader);

            REQUIRE(sendAction == recvAction);
        }

        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);

            sendAction.EventName = "Plot twist !";

            sendAction.GenerateDifferential(recvAction, writer);

            Buffer::Reader reader(&buff);
            recvAction.ApplyDifferential(reader);

            REQUIRE(sendAction == recvAction);
        }
    }

    GIVEN("Full Mods")
    {
        Mods sendMods, recvMods;

        Buffer buff(1000);
        Buffer::Writer writer(&buff);

        sendMods.ModList.push_back({"Hello", 42});
        sendMods.ModList.push_back({"Hi", 14});
        sendMods.ModList.push_back({"Test", 8});
        sendMods.ModList.push_back({"Toast", 49});

        sendMods.Serialize(writer);

        Buffer::Reader reader(&buff);
        recvMods.Deserialize(reader);

        REQUIRE(sendMods == recvMods);
    }

    GIVEN("AnimationVariables")
    {
        AnimationVariables vars, recvVars;

        vars.Booleans.resize(76);
        String testString("\xDE\xAD\xBE\xEF"
            "\xDE\xAD\xBE\xEF\x76\xB");
        vars.String_to_VectorBool(testString, vars.Booleans);

        vars.Floats.push_back(1.f);
        vars.Floats.push_back(7.f);
        vars.Floats.push_back(12.f);
        vars.Floats.push_back(0.f);
        vars.Floats.push_back(145.f);
        vars.Floats.push_back(100.f);
        vars.Floats.push_back(-1.f);

        vars.Integers.push_back(0);
        vars.Integers.push_back(12000);
        vars.Integers.push_back(06);
        vars.Integers.push_back(7778);
        vars.Integers.push_back(41104539);

        Buffer buff(1000);
        {
            Buffer::Writer writer(&buff);

            vars.GenerateDiff(recvVars, writer);

            Buffer::Reader reader(&buff);
            recvVars.ApplyDiff(reader);

            REQUIRE(vars.Booleans == recvVars.Booleans);
            REQUIRE(vars.Floats == recvVars.Floats);
            REQUIRE(vars.Integers == recvVars.Integers);
        }

        vars.Booleans.resize(33);
        vars.Booleans[16] = false;
        vars.Booleans[17] = false;
        vars.Booleans[18] = false;
        vars.Booleans[19] = false;
        vars.Floats[3] = 42.f;
        vars.Integers[0] = 18;
        vars.Integers[3] = 0;

        {
            Buffer::Writer writer(&buff);

            vars.GenerateDiff(recvVars, writer);

            Buffer::Reader reader(&buff);
            recvVars.ApplyDiff(reader);

            REQUIRE(vars.Booleans == recvVars.Booleans);
            REQUIRE(vars.Floats == recvVars.Floats);
            REQUIRE(vars.Integers == recvVars.Integers);
        }
    }
}

TEST_CASE("Packets", "[encoding.packets]")
{
    SECTION("AuthenticationRequest")
    {
        Buffer buff(1000);

        AuthenticationRequest sendMessage, recvMessage;
        sendMessage.Token = "TesSt";
        sendMessage.UserMods.ModList.push_back({"Hello", 42});
        sendMessage.UserMods.ModList.push_back({"Hi", 14});
        sendMessage.UserMods.ModList.push_back({"Test", 8});
        sendMessage.UserMods.ModList.push_back({"Toast", 49});

        Buffer::Writer writer(&buff);
        sendMessage.Serialize(writer);

        Buffer::Reader reader(&buff);

        uint64_t trash;
        reader.ReadBits(trash, 8); // pop opcode

        recvMessage.DeserializeRaw(reader);

        REQUIRE(sendMessage == recvMessage);
    }

    SECTION("AuthenticationResponse")
    {
        Buffer buff(1000);

        AuthenticationResponse sendMessage, recvMessage;
        sendMessage.Type = AuthenticationResponse::ResponseType::kAccepted;
        sendMessage.UserMods.ModList.push_back({"Hello", 42});
        sendMessage.UserMods.ModList.push_back({"Hi", 14});
        sendMessage.UserMods.ModList.push_back({"Test", 8});
        sendMessage.UserMods.ModList.push_back({"Toast", 49});

        Buffer::Writer writer(&buff);
        sendMessage.Serialize(writer);

        Buffer::Reader reader(&buff);

        uint64_t trash;
        reader.ReadBits(trash, 8); // pop opcode

        recvMessage.DeserializeRaw(reader);

        REQUIRE(sendMessage == recvMessage);
    }

    SECTION("CancelAssignmentRequest")
    {
        Buffer buff(1000);

        CancelAssignmentRequest sendMessage, recvMessage;
        sendMessage.Cookie = 14523698;
        Buffer::Writer writer(&buff);
        sendMessage.Serialize(writer);

        Buffer::Reader reader(&buff);

        uint64_t trash;
        reader.ReadBits(trash, 8); // pop opcode

        recvMessage.DeserializeRaw(reader);

        REQUIRE(sendMessage == recvMessage);
    }

    SECTION("AssignCharacterRequest")
    {
        Buffer buff(1000);

        NetActionEvent sendAction;
        sendAction.ActionId = GameId(1, 42);
        sendAction.State1 = 6547;
        sendAction.Tick = 48;
        sendAction.ActorId = 12345678;
        sendAction.EventName = "test";
        sendAction.IdleId = GameId(2, 87964);
        sendAction.State2 = 8963;
        sendAction.TargetEventName = "toast";
        sendAction.TargetId = 963741;
        sendAction.Type = 4;

        AssignCharacterRequest sendMessage, recvMessage;
        sendMessage.Cookie = 14523698;
        sendMessage.AppearanceBuffer = "toto";
        sendMessage.CellId.BaseId = 45;
        sendMessage.FormId.ModId = 48;
        sendMessage.ReferenceId.BaseId = 456799;
        sendMessage.ReferenceId.ModId = 4079;
        sendMessage.LatestAction = sendAction;
        sendMessage.Position.x = -452.4f;
        sendMessage.Position.y = 452.4f;
        sendMessage.Position.z = 125452.4f;
        sendMessage.Rotation.x = -1.87f;
        sendMessage.Rotation.y = 45.35f;

        Buffer::Writer writer(&buff);
        sendMessage.Serialize(writer);

        Buffer::Reader reader(&buff);

        uint64_t trash;
        reader.ReadBits(trash, 8); // pop opcode

        recvMessage.DeserializeRaw(reader);

        REQUIRE(sendMessage == recvMessage);
    }

    GIVEN("ClientReferencesMoveRequest")
    {
        ClientReferencesMoveRequest sendMessage, recvMessage;
        auto& update = sendMessage.Updates[1];
        auto& move = update.UpdatedMovement;

        AnimationVariables vars;
        vars.Booleans.resize(76);
        String testString("\xDE\xAD\xBE\xEF\x76\xB");
        vars.String_to_VectorBool(testString, vars.Booleans);

        vars.Floats.push_back(1.f);
        vars.Floats.push_back(7.f);
        vars.Floats.push_back(12.f);
        vars.Floats.push_back(0.f);
        vars.Floats.push_back(145.f);
        vars.Floats.push_back(100.f);
        vars.Floats.push_back(-1.f);

        vars.Integers.push_back(0);
        vars.Integers.push_back(12000);
        vars.Integers.push_back(06);
        vars.Integers.push_back(7778);
        vars.Integers.push_back(41104539);

        move.Variables = vars;

        Buffer buff(1000);
        Buffer::Writer writer(&buff);
        sendMessage.Serialize(writer);

        Buffer::Reader reader(&buff);

        uint64_t trash;
        reader.ReadBits(trash, 8); // pop opcode

        recvMessage.DeserializeRaw(reader);

        REQUIRE(recvMessage.Updates[1].UpdatedMovement == sendMessage.Updates[1].UpdatedMovement);
    }
}

TEST_CASE("StringCache", "[encoding.string_cache]")
{
    SECTION("Messages")
    {
        StringCacheUpdate update;
        update.Values.push_back("Hello");
        update.Values.push_back("Bye");

        Buffer buff(1000);
        Buffer::Writer writer(&buff);
        update.Serialize(writer);

        Buffer::Reader reader(&buff);

        uint64_t trash;
        reader.ReadBits(trash, 8); // pop opcode

        StringCacheUpdate recvUpdate;
        recvUpdate.DeserializeRaw(reader);

        REQUIRE(update == recvUpdate);
    }
}

TEST_CASE("ReferenceUpdate serialization", "[encoding.reference_update]")
{
    GIVEN("Multiple NetActionEvents with differential encoding")
    {
        ReferenceUpdate sendUpdate, recvUpdate;
        
        // Set up movement data
        sendUpdate.UpdatedMovement.Position = {100.0f, 200.0f, 300.0f};
        sendUpdate.UpdatedMovement.Rotation.x = 1.0f;
        sendUpdate.UpdatedMovement.Rotation.y = 2.0f;
        sendUpdate.UpdatedMovement.Direction = 0.5f;
        sendUpdate.UpdatedMovement.CellId = GameId(1, 1000);
        sendUpdate.UpdatedMovement.WorldSpaceId = GameId(1, 60);
        
        // Create multiple action events with incremental changes
        NetActionEvent event1;
        event1.Tick = 100;
        event1.ActorId = 12345;
        event1.TargetId = 0;
        event1.ActionId = GameId(1, 42);
        event1.State1 = 1000;
        event1.State2 = 2000;
        event1.Type = 1;
        event1.EventName = "AttackStart";
        event1.SequenceIndex = 0;
        
        NetActionEvent event2;
        event2.Tick = 150;
        event2.ActorId = 12345;
        event2.TargetId = 54321;
        event2.ActionId = GameId(1, 42);
        event2.State1 = 1001;
        event2.State2 = 2000;
        event2.Type = 1;
        event2.EventName = "AttackHit";
        event2.SequenceIndex = 1;
        
        NetActionEvent event3;
        event3.Tick = 200;
        event3.ActorId = 12345;
        event3.TargetId = 54321;
        event3.ActionId = GameId(2, 100);
        event3.State1 = 2000;
        event3.State2 = 3000;
        event3.Type = 2;
        event3.EventName = "AttackEnd";
        event3.SequenceIndex = 2;
        event3.IdleId = GameId(3, 500);
        
        sendUpdate.ActionEvents.push_back(event1);
        sendUpdate.ActionEvents.push_back(event2);
        sendUpdate.ActionEvents.push_back(event3);
        
        // Serialize
        Buffer buff(1000);
        Buffer::Writer writer(&buff);
        sendUpdate.Serialize(writer);
        
        // Deserialize
        Buffer::Reader reader(&buff);
        recvUpdate.Deserialize(reader);

        // Check
        REQUIRE(sendUpdate.UpdatedMovement.Position == recvUpdate.UpdatedMovement.Position);
        REQUIRE(sendUpdate.UpdatedMovement.Rotation == recvUpdate.UpdatedMovement.Rotation);
        REQUIRE(sendUpdate.UpdatedMovement.Direction == recvUpdate.UpdatedMovement.Direction);
        REQUIRE(sendUpdate.UpdatedMovement.CellId == recvUpdate.UpdatedMovement.CellId);
        REQUIRE(sendUpdate.UpdatedMovement.WorldSpaceId == recvUpdate.UpdatedMovement.WorldSpaceId);

        
        REQUIRE(sendUpdate.ActionEvents.size() == recvUpdate.ActionEvents.size());
        REQUIRE(recvUpdate.ActionEvents.size() == 3);
        
        for (size_t i = 0; i < sendUpdate.ActionEvents.size(); ++i)
        {
            const auto& sent = sendUpdate.ActionEvents[i];
            const auto& recv = recvUpdate.ActionEvents[i];
            
            REQUIRE(sent.Tick == recv.Tick);
            REQUIRE(sent.ActorId == recv.ActorId);
            REQUIRE(sent.TargetId == recv.TargetId);
            REQUIRE(sent.ActionId == recv.ActionId);
            REQUIRE(sent.State1 == recv.State1);
            REQUIRE(sent.State2 == recv.State2);
            REQUIRE(sent.Type == recv.Type);
            REQUIRE(sent.EventName == recv.EventName);
            REQUIRE(sent.SequenceIndex == recv.SequenceIndex);
            REQUIRE(sent.IdleId == recv.IdleId);
        }
        
        // Verify the complete equality
        REQUIRE(sendUpdate == recvUpdate);
    }
    
    GIVEN("Empty ActionEvents list")
    {
        ReferenceUpdate sendUpdate, recvUpdate;
        
        sendUpdate.UpdatedMovement.Position = {10.0f, 20.0f, 30.0f};
        sendUpdate.UpdatedMovement.CellId = GameId(1, 2000);
        
        Buffer buff(1000);
        Buffer::Writer writer(&buff);
        sendUpdate.Serialize(writer);
        
        Buffer::Reader reader(&buff);
        recvUpdate.Deserialize(reader);
        
        REQUIRE(sendUpdate.ActionEvents.empty());
        REQUIRE(recvUpdate.ActionEvents.empty());
        REQUIRE(sendUpdate == recvUpdate);
    }
    
    GIVEN("Multiple ReferenceUpdate packets in sequence")
    {
        // This tests that each ReferenceUpdate packet independently handles
        // its differential encoding correctly
        std::vector<ReferenceUpdate> sendUpdates(3);
        std::vector<ReferenceUpdate> recvUpdates(3);
        
        // First update with 2 events
        sendUpdates[0].UpdatedMovement.Position = {100.0f, 100.0f, 100.0f};
        sendUpdates[0].UpdatedMovement.CellId = GameId(1, 1000);
        
        NetActionEvent event1a;
        event1a.Tick = 100;
        event1a.ActorId = 11111;
        event1a.ActionId = GameId(1, 10);
        event1a.EventName = "Move";
        
        NetActionEvent event1b;
        event1b.Tick = 120;  // Differential: +20
        event1b.ActorId = 11111;  // Same actor
        event1b.ActionId = GameId(1, 11);  // Different action
        event1b.EventName = "Jump";
        
        sendUpdates[0].ActionEvents.push_back(event1a);
        sendUpdates[0].ActionEvents.push_back(event1b);
        
        // Second update with 3 events, different actor
        sendUpdates[1].UpdatedMovement.Position = {200.0f, 200.0f, 200.0f};
        sendUpdates[1].UpdatedMovement.CellId = GameId(1, 2000);
        
        NetActionEvent event2a;
        event2a.Tick = 200;
        event2a.ActorId = 22222;  // Different actor
        event2a.ActionId = GameId(2, 20);
        event2a.EventName = "Attack";
        
        NetActionEvent event2b;
        event2b.Tick = 250;  // Differential: +50
        event2b.ActorId = 22222;
        event2b.ActionId = GameId(2, 20);  // Same action
        event2b.EventName = "AttackHit";
        
        NetActionEvent event2c;
        event2c.Tick = 300;  // Differential: +50
        event2c.ActorId = 22222;
        event2c.ActionId = GameId(2, 21);  // Different action
        event2c.EventName = "AttackEnd";
        
        sendUpdates[1].ActionEvents.push_back(event2a);
        sendUpdates[1].ActionEvents.push_back(event2b);
        sendUpdates[1].ActionEvents.push_back(event2c);
        
        // Third update with 1 event
        sendUpdates[2].UpdatedMovement.Position = {300.0f, 300.0f, 300.0f};
        sendUpdates[2].UpdatedMovement.CellId = GameId(1, 3000);
        
        NetActionEvent event3a;
        event3a.Tick = 400;
        event3a.ActorId = 33333;
        event3a.ActionId = GameId(3, 30);
        event3a.EventName = "Idle";
        event3a.IdleId = GameId(3, 1000);
        
        sendUpdates[2].ActionEvents.push_back(event3a);
        
        // Serialize and deserialize each update independently
        for (size_t i = 0; i < sendUpdates.size(); ++i)
        {
            Buffer buff(1000);
            Buffer::Writer writer(&buff);
            sendUpdates[i].Serialize(writer);
            
            Buffer::Reader reader(&buff);
            recvUpdates[i].Deserialize(reader);
            
            // Verify this specific update
            REQUIRE(sendUpdates[i] == recvUpdates[i]);
            REQUIRE(sendUpdates[i].ActionEvents.size() == recvUpdates[i].ActionEvents.size());
            
            // Verify each event in detail
            for (size_t j = 0; j < sendUpdates[i].ActionEvents.size(); ++j)
            {
                const auto& sent = sendUpdates[i].ActionEvents[j];
                const auto& recv = recvUpdates[i].ActionEvents[j];
                
                REQUIRE(sent.Tick == recv.Tick);
                REQUIRE(sent.ActorId == recv.ActorId);
                REQUIRE(sent.ActionId == recv.ActionId);
                REQUIRE(sent.EventName == recv.EventName);
                REQUIRE(sent.IdleId == recv.IdleId);
            }
        }
        
        // Verify that updates are independent (no cross-contamination)
        REQUIRE(recvUpdates[0].ActionEvents.size() == 2);
        REQUIRE(recvUpdates[1].ActionEvents.size() == 3);
        REQUIRE(recvUpdates[2].ActionEvents.size() == 1);
        
        // Verify specific values weren't affected by other updates
        REQUIRE(recvUpdates[0].ActionEvents[0].ActorId == 11111);
        REQUIRE(recvUpdates[1].ActionEvents[0].ActorId == 22222);
        REQUIRE(recvUpdates[2].ActionEvents[0].ActorId == 33333);
    }
}
