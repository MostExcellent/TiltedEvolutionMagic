#include <ModCompat/ScriptExtenderPluginList.h>

#include <algorithm>
#include <filesystem>
#include <immersive_launcher/launcher.h>
#include <spdlog/spdlog.h>

bool ScriptExtenderPluginList::HasPlugin(const String& aPluginName) const
{
    // Return false if no plugins or empty string
    if (m_Plugins.empty() || aPluginName.empty())
        return false;
        
    return m_Plugins.find(aPluginName) != m_Plugins.end();
}

bool ScriptExtenderPluginList::HasPlugins(const Vector<String>& aPluginNames, bool abMatchExact) const
{
    // Return false if no SKSE plugins or none to check
    if (aPluginNames.empty() || m_Plugins.empty())
        return false;
    
    auto isPluginInList = [this](const String& pluginName) {
        return m_Plugins.find(pluginName) != m_Plugins.end();
    };
    
    // If abMatchExact is true, check if ALL plugin names are in m_Plugins
    if (abMatchExact)
    {
        return std::ranges::all_of(aPluginNames, isPluginInList);
    }
    
    // Otherwise, check if ANY plugin name is in m_Plugins
    return std::ranges::any_of(aPluginNames, isPluginInList);
}

void ScriptExtenderPluginList::DumpFoundPlugins() const
{
    for (const auto& plugin : m_Plugins)
    {
        spdlog::info("Found SKSE plugin: {}", plugin);
    }
}
void ScriptExtenderPluginList::Init()
{
    // Early out if already initialized
    if (m_bPluginsChecked)
    {
        return;
    }

    // std::filesystem::path gamePath = launcher::GetLaunchContext()->gamePath;
    std::filesystem::path pPluginsPath = launcher::GetLaunchContext()->gamePath / L"Data" / L"SKSE" / L"Plugins";
    
    if (!std::filesystem::exists(pPluginsPath) || !std::filesystem::is_directory(pPluginsPath))
    {
        spdlog::warn("No SKSE plugins directory found");
        m_bPluginsChecked = true;
        return;
    }

    // Iterate through all files in the plugins directory and add names with .dll extension to m_Plugins
    for (const auto& entry : std::filesystem::directory_iterator(pPluginsPath))
    {
        if (entry.is_regular_file())
        {
            const auto& path = entry.path();
            if (path.extension() == L".dll")
            {
                // Convert the filename to string and add to the set
                String pluginName(path.filename().string());
                m_Plugins.insert(pluginName);
            }
        }
    }
    spdlog::debug("Found {} SKSE plugins", m_Plugins.size());
    m_bPluginsChecked = true;
}

ScriptExtenderPluginList::ScriptExtenderPluginList()
{
    m_bPluginsChecked = false;
    m_Plugins = {};
}

ScriptExtenderPluginList* ScriptExtenderPluginList::s_Instance = nullptr;

ScriptExtenderPluginList* ScriptExtenderPluginList::Get()
{
    if (!s_Instance)
    {
        s_Instance = new ScriptExtenderPluginList();
        s_Instance->Init();
    }
    return s_Instance;
}

// TiltedPhoques::Initializer s_ScriptExtenderPluginListDebugInitializer(
//     []() {
//         ScriptExtenderPluginList::Get()->DumpFoundPlugins();
//     });
