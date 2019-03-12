#pragma once

#include <mutex>
#include <vector>

#include "dxvk_include.h"

#ifdef __WINE__
using SoHandle = void*;
#else
using SoHandle = HMODULE;
#endif

namespace vr {
  class IVRCompositor;
  class IVRSystem;
}

namespace dxvk {

  class DxvkInstance;

  /**
   * \brief OpenVR instance
   * 
   * Loads Initializes OpenVR to provide
   * access to Vulkan extension queries.
   */
  class VrInstance {
    
  public:
    
    VrInstance();
    ~VrInstance();

    /**
     * \brief Query instance extensions
     * \returns Instance extensions
     */
    DxvkNameSet getInstanceExtensions();

    /**
     * \brief Query device extensions
     * 
     * Retrieves the extensions required for a specific
     * physical device. The adapter index should remain
     * the same across multiple Vulkan instances.
     * \param [in] adapterId Adapter index
     */
    DxvkNameSet getDeviceExtensions(
            uint32_t      adapterId);
    
    /**
     * \brief Initializes instance extension set
     * 
     * Should be called before creating
     * the first Vulkan instance.
     */
    void initInstanceExtensions();

    /**
     * \brief Initializes device extension sets
     * 
     * Should be called after setting
     * up the Vulkan physical devices.
     * \param [in] instance DXVK instance
     */
    void initDeviceExtensions(
      const DxvkInstance* instance);

  private:

    std::mutex            m_mutex;
    vr::IVRCompositor*    m_compositor = nullptr;
    SoHandle              m_ovrApi     = nullptr;

    bool m_loadedOvrApi      = false;
    bool m_initializedOpenVr = false;
    bool m_initializedInsExt = false;
    bool m_initializedDevExt = false;

    DxvkNameSet              m_insExtensions;
    std::vector<DxvkNameSet> m_devExtensions;
    
    DxvkNameSet queryInstanceExtensions() const;

    DxvkNameSet queryDeviceExtensions(
            VkPhysicalDevice          adapter) const;

    DxvkNameSet parseExtensionList(
      const std::string&              str) const;
    
    vr::IVRCompositor* getCompositor();

    void shutdown();

    SoHandle loadLibrary();

    void freeLibrary();

    void* getSym(const char* sym);
    
  };

  extern VrInstance g_vrInstance;
  
}