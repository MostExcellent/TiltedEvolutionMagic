
#include <imgui.h>
#include <EquipManager.h>
#include <services/DebugService.h>

#include <Games/Skyrim/Forms/TESForm.h>
#include <Games/Skyrim/BSGraphics/BSGraphicsRenderer.h>
#include <Games/Skyrim/DefaultObjectManager.h>
#include <Games/Skyrim/Forms/TESAmmo.h>
#include <Games/Skyrim/Misc/InventoryEntry.h>
#include <Games/Skyrim/Misc/MiddleProcess.h>

#include <Games/ActorExtension.h>

#include <Actor.h>

#include <BSAnimationGraphManager.h>
#include <Havok/hkbStateMachine.h>
#include <Havok/BShkbAnimationGraph.h>
#include <Havok/BShkbHkxDB.h>
#include <Havok/hkbBehaviorGraph.h>

#include <structs/AnimationGraphDescriptorManager.h>
#include <inttypes.h>

#include <ModCompat/BehaviorVar.h>

uint64_t DisplayGraphDescriptorKey(BSAnimationGraphManager* pManager) noexcept
{
    auto hash = pManager->GetDescriptorKey();
    auto pDescriptor = AnimationGraphDescriptorManager::Get().GetDescriptor(hash);

    spdlog::info("Key: {}", hash);
    std::cout << "uint64_t key = " << hash << ";" << '\n';
    if (!pDescriptor)
        spdlog::error("Descriptor key not found");

    return hash;
}

void DebugService::DrawAnimDebugView()
{
    static uint32_t fetchFormId = 0;
    static Actor* pActor = nullptr;
    static Map<uint32_t, uint32_t> s_values;
    static Map<uint32_t, uint32_t> s_reusedValues;
    static Map<uint32_t, short> s_valueTypes; // 0 for bool, 1 for float, 2 for int
    static Vector<uint32_t> s_blacklist{};
    static SortedMap<uint32_t, String> s_varMap{};
    static Map<uint64_t, uint32_t> s_cachedKeys{};

    ImGui::Begin("Animation debugging");

    ImGui::InputScalar("Form ID", ImGuiDataType_U32, &fetchFormId, 0, 0, "%" PRIx32, ImGuiInputTextFlags_CharsHexadecimal);

    if (ImGui::Button("Look up"))
    {
        if (fetchFormId)
        {
            auto* pFetchForm = TESForm::GetById(fetchFormId);
            if (pFetchForm)
                pActor = Cast<Actor>(pFetchForm);
        }

        s_values.clear();
        s_reusedValues.clear();
        s_valueTypes.clear();
        s_blacklist.clear();
        s_varMap.clear();
    }

    if (!pActor)
    {
        ImGui::End();
        s_varMap.clear();
        s_values.clear();
        s_reusedValues.clear();
        s_valueTypes.clear();
        s_blacklist.clear();
        return;
    }

     if (ImGui::Button("Show cached hash"))
    {
        spdlog::info("{}", pActor->GetExtension()->GraphDescriptorHash);
        BehaviorVar::Get()->Debug();
    }

    if (ImGui::Button("Clear all"))
    {
        s_varMap.clear();
        s_values.clear();
        s_reusedValues.clear();
        s_valueTypes.clear();
        s_blacklist.clear();
    }

    BSAnimationGraphManager* pManager = nullptr;
    pActor->animationGraphHolder.GetBSAnimationGraph(&pManager);

    if (!pManager)
    {
        ImGui::End();
        s_varMap.clear();
        s_values.clear();
        s_reusedValues.clear();
        s_valueTypes.clear();
        s_blacklist.clear();
        return;
    }

    const auto pGraph = pManager->animationGraphs.Get(pManager->animationGraphIndex);

    if (!pGraph)
    {
        ImGui::End();
        s_varMap.clear();
        s_values.clear();
        s_reusedValues.clear();
        s_valueTypes.clear();
        s_blacklist.clear();
        return;
    }

    if (s_varMap.empty())
    {
        s_varMap = pManager->DumpAnimationVariables(true);

        auto hash = DisplayGraphDescriptorKey(pManager);

        if (s_cachedKeys.find(hash) != std::end(s_cachedKeys))
            spdlog::warn("Key was detected before! Form ID of last detected actor: {:X}", s_cachedKeys[hash]);
        s_cachedKeys[hash] = pActor->formID;
    }

    if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Variables"))
        {
            auto hash = pManager->GetDescriptorKey();
            auto pDescriptor = AnimationGraphDescriptorManager::Get().GetDescriptor(hash);
            if (pDescriptor)
            {
                const auto* pVariableSet = pGraph->behaviorGraph->animationVariables;

                Map<String, bool> bools{};
                Map<String, float> floats{};
                Map<String, int> ints{};

                for (size_t i = 0; i < pDescriptor->BooleanLookUpTable.size(); ++i)
                {
                    const auto idx = pDescriptor->BooleanLookUpTable[i];
                    bools[s_varMap[idx]] = pVariableSet->data[idx].b;
                }

                for (size_t i = 0; i < pDescriptor->FloatLookupTable.size(); ++i)
                {
                    const auto idx = pDescriptor->FloatLookupTable[i];
                    floats[s_varMap[idx]] = pVariableSet->data[idx].f;
                }

                for (size_t i = 0; i < pDescriptor->IntegerLookupTable.size(); ++i)
                {
                    const auto idx = pDescriptor->IntegerLookupTable[i];
                    ints[s_varMap[idx]] = pVariableSet->data[idx].i;
                }

                if (ImGui::Button("Dump variable values"))
                {
                    std::cout << "Bools: " << '\n';
                    for (auto& [name, value] : bools)
                    {
                        std::cout << "\t" << name << ": " << value << '\n';
                    }

                    std::cout << "Floats: " << '\n';
                    for (auto& [name, value] : floats)
                    {
                        std::cout << "\t" << name << ": " << value << '\n';
                    }

                    std::cout << "Integers: " << '\n';
                    for (auto& [name, value] : ints)
                    {
                        std::cout << "\t" << name << ": " << value << '\n';
                    }
                }

                if (ImGui::CollapsingHeader("Bools", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (auto pair : bools)
                    {
                        auto name = pair.first;
                        auto value = pair.second;
                        int iValue = int(value);
                        ImGui::InputInt(name.c_str(), &iValue, 0, 0, ImGuiInputTextFlags_ReadOnly);
                    }
                }

                if (ImGui::CollapsingHeader("Floats", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (auto pair : floats)
                    {
                        auto name = pair.first;
                        auto value = pair.second;
                        ImGui::InputFloat(name.c_str(), &value, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
                    }
                }

                if (ImGui::CollapsingHeader("Integers", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (auto pair : ints)
                    {
                        auto name = pair.first;
                        auto value = pair.second;
                        ImGui::InputInt(name.c_str(), &value, 0, 0, ImGuiInputTextFlags_ReadOnly);
                    }
                }
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Blacklist"))
        {
            static uint32_t s_selectedBlacklistVar = 0;
            static uint32_t s_selected = 0;

            ImGui::BeginChild("Blacklisted variables");

            int i = 0;
            for (auto blacklistedVar : s_blacklist)
            {
                if (blacklistedVar >= pGraph->behaviorGraph->animationVariables->size)
                {
                    ++i;
                    continue;
                }

                const auto varName = s_varMap[blacklistedVar];

                char name[256];
                sprintf_s(name, std::size(name), "k%s (%d)", varName.c_str(), blacklistedVar);

                if (ImGui::Selectable(name, s_selectedBlacklistVar == blacklistedVar))
                {
                    s_selectedBlacklistVar = blacklistedVar;
                }

                if (s_selectedBlacklistVar == blacklistedVar)
                {
                    s_selected = i;
                }

                ++i;
            }

            ImGui::EndChild();

            if (s_selected < s_blacklist.size())
            {
                if (ImGui::Button("Delete"))
                {
                    s_blacklist.erase(s_blacklist.begin() + s_selected);
                }
            }

            static uint32_t s_blacklistVar = 0;
            ImGui::InputInt("New blacklist var:", (int*)&s_blacklistVar, 0, 0);

            if (ImGui::Button("Add"))
            {
                s_blacklist.push_back(s_blacklistVar);
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Control"))
        {
            ImGui::InputInt("Animation graph count", (int*)&pManager->animationGraphs.size, 0, 0, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputInt("Animation graph index", (int*)&pManager->animationGraphIndex, 0, 0, ImGuiInputTextFlags_ReadOnly);

            char name[256];
            sprintf_s(name, std::size(name), "%s", pGraph->behaviorGraph->rootGenerator->name.data());
            ImGui::InputText("Graph root generator name", name, std::size(name), ImGuiInputTextFlags_ReadOnly);

            const auto pVariableSet = pGraph->behaviorGraph->animationVariables;

            auto pDescriptor = AnimationGraphDescriptorManager::Get().GetDescriptor(pManager->GetDescriptorKey());

            static bool toggleVariableRecord = false;
            if (ImGui::Button("Toggle variable recording"))
            {
                toggleVariableRecord = !toggleVariableRecord;
                spdlog::info("Toggle variable recording: {}", toggleVariableRecord);
            }

            if (pVariableSet && toggleVariableRecord)
            {
                for (auto i = 0u; i < pVariableSet->size; ++i)
                {
                    if (std::find(s_blacklist.begin(), s_blacklist.end(), i) != s_blacklist.end())
                        continue;

                    if (pDescriptor && pDescriptor->IsSynced(i))
                        continue;

                    auto iter = s_values.find(i);
                    if (iter == std::end(s_values))
                    {
                        s_values[i] = pVariableSet->data[i].raw;

                        const auto varName = s_varMap[i];

                        spdlog::info("Variable k{} ({}) initialized to f: {} i: {}", varName, i, pVariableSet->data[i].f, pVariableSet->data[i].i);
                    }
                    else if (iter->second != pVariableSet->data[i].raw)
                    {
                        const auto varName = s_varMap[i];

                        float asFloat = pVariableSet->data[i].f;
                        int asInt = pVariableSet->data[i].i;
                        spdlog::warn("Variable k{} ({}) changed to f: {} i: {}", varName, i, asFloat, asInt);

                        s_values[i] = pVariableSet->data[i].raw;
                        s_reusedValues[i] = pVariableSet->data[i].raw;

                        char varTypeChar = varName[0]; // for guessing type, to see if u can find type char (i, f, b)
                        if (varTypeChar == 'f')
                        {
                            s_valueTypes[i] = 1;
                        }
                        else if (varTypeChar == 'i' && varName[1] != 's')
                        {
                            s_valueTypes[i] = 2;
                        }
                        else if (varTypeChar != 'b' && s_valueTypes[i] != 1) // no char hint to go off of and not assuming float
                        {
                            if (asInt > 1000 || asInt < -1000) // arbitrary int threshold
                            {
                                s_valueTypes[i] = 1; // assume float
                            }
                            else
                            {
                                if (asInt > 1 || asInt < 0)
                                {
                                    s_valueTypes[i] = 2; // assume int
                                }
                            }
                        }
                    }
                }
            }

            if (ImGui::Button("Dump variables") && pVariableSet)
            {
                // kinda ugly to iterate 3 times but idc cuz its just for debugging and its a small collection
                //  BOOLS
                std::cout << "{" << '\n';
                for (auto& [key, value] : s_reusedValues)
                {
                    if (s_valueTypes[key] == 0)
                    {
                        const auto varName = s_varMap[key];
                        std::cout << "k" << varName << "," << '\n';
                    }
                }
                // FLOATS
                std::cout << "}," << '\n' << "{" << '\n';
                for (auto& [key, value] : s_reusedValues)
                {
                    if (s_valueTypes[key] == 1)
                    {
                        const auto varName = s_varMap[key];
                        std::cout << "k" << varName << "," << '\n';
                    }
                }
                // INTS
                std::cout << "}," << '\n' << "{" << '\n';
                for (auto& [key, value] : s_reusedValues)
                {
                    if (s_valueTypes[key] == 2)
                    {
                        const auto varName = s_varMap[key];
                        std::cout << "k" << varName << "," << '\n';
                    }
                }
                std::cout << "}" << '\n';
            }

            if (ImGui::Button("Reset recording"))
            {
                s_values.clear();
                s_reusedValues.clear();
                spdlog::info("Variable recording has been reset");
            }

            ImGui::EndTabItem();
        }
    }

    ImGui::End();
}
