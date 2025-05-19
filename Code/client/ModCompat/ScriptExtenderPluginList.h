#pragma once

/**
 * @brief Manages Script Extender plugin detection
 * 
 * This class checks for the presence of SKSE plugins. Useful for 
 */
class ScriptExtenderPluginList
{
public:
    const Set<String>& GetPlugins() const { return m_Plugins; }

    bool HasPlugin(const String& aPluginName) const;
    bool HasPlugins(const Vector<String>& aPluginNames, bool abMatchExact = false) const;

    void DumpFoundPlugins() const;

private:
    void Init();
    
    bool m_bPluginsChecked;
    
    Set<String> m_Plugins;
    
public:
    static ScriptExtenderPluginList* Get();

private:
    ScriptExtenderPluginList();
    static ScriptExtenderPluginList* s_Instance;
};
