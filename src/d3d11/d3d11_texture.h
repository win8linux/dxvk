#pragma once

#include "../dxvk/dxvk_device.h"

#include "../d3d10/d3d10_texture.h"

#include "d3d11_device_child.h"
#include "d3d11_interfaces.h"

namespace dxvk {
  
  class D3D11Device;
  
  /**
   * \brief Image memory mapping mode
   * 
   * Determines how exactly \c Map will
   * behave when mapping an image.
   */
  enum D3D11_COMMON_TEXTURE_MAP_MODE {
    D3D11_COMMON_TEXTURE_MAP_MODE_NONE,   ///< Not mapped
    D3D11_COMMON_TEXTURE_MAP_MODE_BUFFER, ///< Mapped through buffer
    D3D11_COMMON_TEXTURE_MAP_MODE_DIRECT, ///< Directly mapped to host mem
  };
  
  
  /**
   * \brief Common texture description
   * 
   * Contains all members that can be
   * defined for 1D, 2D and 3D textures.
   */
  struct D3D11_COMMON_TEXTURE_DESC {
    UINT             Width;
    UINT             Height;
    UINT             Depth;
    UINT             MipLevels;
    UINT             ArraySize;
    DXGI_FORMAT      Format;
    DXGI_SAMPLE_DESC SampleDesc;
    D3D11_USAGE      Usage;
    UINT             BindFlags;
    UINT             CPUAccessFlags;
    UINT             MiscFlags;
  };
  
  
  /**
   * \brief D3D11 common texture object
   * 
   * This class implements common texture methods and
   * aims to work around the issue that there are three
   * different interfaces for basically the same thing.
   */
  class D3D11CommonTexture {
    
  public:
    
    D3D11CommonTexture(
            D3D11Device*                pDevice,
      const D3D11_COMMON_TEXTURE_DESC*  pDesc,
            D3D11_RESOURCE_DIMENSION    Dimension);
    
    ~D3D11CommonTexture();
    
    /**
     * \brief Texture properties
     * 
     * The returned data can be used to fill in
     * \c D3D11_TEXTURE2D_DESC and similar structs.
     * \returns Pointer to texture description
     */
    const D3D11_COMMON_TEXTURE_DESC* Desc() const {
      return &m_desc;
    }
    
    /**
     * \brief Map mode
     * \returns Map mode
     */
    D3D11_COMMON_TEXTURE_MAP_MODE GetMapMode() const {
      return m_mapMode;
    }
    
    /**
     * \brief The DXVK image
     * \returns The DXVK image
     */
    Rc<DxvkImage> GetImage() const {
      return m_image;
    }
    
    /**
     * \brief The DXVK buffer
     * \returns The DXVK buffer
     */
    Rc<DxvkBuffer> GetMappedBuffer() const {
      return m_buffer;
    }
    
    /**
     * \brief Currently mapped subresource
     * \returns Mapped subresource
     */
    VkImageSubresource GetMappedSubresource() const {
      return m_mappedSubresource;
    }

    /**
     * \brief Current map type
     */
    D3D11_MAP GetMapType() const {
      return m_mapType;
    }
    
    /**
     * \brief Sets mapped subresource
     * \param [in] subresource THe subresource
     */
    void SetMappedSubresource(VkImageSubresource Subresource, D3D11_MAP MapType) {
      m_mappedSubresource = Subresource;
      m_mapType           = MapType;
    }
    
    /**
     * \brief Resets mapped subresource
     * Marks the texture as not mapped.
     */
    void ClearMappedSubresource() {
      m_mappedSubresource = VkImageSubresource { };
    }
    
    /**
     * \brief Computes subresource from the subresource index
     * 
     * Used by some functions that operate on only
     * one subresource, such as \c UpdateSubresource.
     * \param [in] Aspect The image aspect
     * \param [in] Subresource Subresource index
     * \returns The Vulkan image subresource
     */
    VkImageSubresource GetSubresourceFromIndex(
            VkImageAspectFlags    Aspect,
            UINT                  Subresource) const;
    
    /**
     * \brief Format mode
     * 
     * Determines whether the image is going to
     * be used as a color image or a depth image.
     * \returns Format mode
     */
    DXGI_VK_FORMAT_MODE GetFormatMode() const;
    
    /**
     * \brief Retrieves parent D3D11 device
     * \param [out] ppDevice The device
     */
    void GetDevice(ID3D11Device** ppDevice) const;
    
    /**
     * \brief Checks whether a view can be created for this textue
     * 
     * View formats are only compatible if they are either identical
     * or from the same family of typeless formats, where the resource
     * format must be typeless and the view format must be typed. This
     * will also check whether the required bind flags are supported.
     * \param [in] BindFlags Bind flags for the view
     * \param [in] Format The desired view format
     * \returns \c true if the format is compatible
     */
    bool CheckViewCompatibility(
            UINT                BindFlags,
            DXGI_FORMAT         Format) const;
    
    /**
     * \brief Normalizes and validates texture description
     * 
     * Fills in undefined values and validates the texture
     * parameters. Any error returned by this method should
     * be forwarded to the application.
     * \param [in,out] pDesc Texture description
     * \returns \c S_OK if the parameters are valid
     */
    static HRESULT NormalizeTextureProperties(
            D3D11_COMMON_TEXTURE_DESC* pDesc);
    
  private:
    
    Com<D3D11Device>              m_device;
    D3D11_COMMON_TEXTURE_DESC     m_desc;
    D3D11_COMMON_TEXTURE_MAP_MODE m_mapMode;
    
    Rc<DxvkImage>   m_image;
    Rc<DxvkBuffer>  m_buffer;
    
    VkImageSubresource m_mappedSubresource
      = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    D3D11_MAP m_mapType = D3D11_MAP_READ;
    
    Rc<DxvkBuffer> CreateMappedBuffer() const;
    
    BOOL CheckImageSupport(
      const DxvkImageCreateInfo*  pImageInfo,
            VkImageTiling         Tiling) const;
    
    BOOL CheckFormatFeatureSupport(
            VkFormat              Format,
            VkFormatFeatureFlags  Features) const;
    
    VkImageUsageFlags EnableMetaCopyUsage(
            VkFormat              Format,
            VkImageTiling         Tiling) const;
    
    VkImageUsageFlags EnableMetaPackUsage(
            VkFormat              Format,
            UINT                  CpuAccess) const;
    
    D3D11_COMMON_TEXTURE_MAP_MODE DetermineMapMode(
      const DxvkImageCreateInfo*  pImageInfo) const;
    
    static VkImageType GetImageTypeFromResourceDim(
            D3D11_RESOURCE_DIMENSION  Dimension);
    
    static VkImageLayout OptimizeLayout(
            VkImageUsageFlags         Usage);
    
  };
  
  
  /**
   * \brief Common texture interop class
   * 
   * Provides access to the underlying Vulkan
   * texture to external Vulkan libraries.
   */
  class D3D11VkInteropSurface : public IDXGIVkInteropSurface {
    
  public:
    
    D3D11VkInteropSurface(
            ID3D11DeviceChild*  pContainer,
            D3D11CommonTexture* pTexture);
    
    ~D3D11VkInteropSurface();
    
    ULONG STDMETHODCALLTYPE AddRef();
    
    ULONG STDMETHODCALLTYPE Release();
    
    HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID                  riid,
            void**                  ppvObject);
    
    HRESULT STDMETHODCALLTYPE GetDevice(
            IDXGIVkInteropDevice**  ppDevice);
    
    HRESULT STDMETHODCALLTYPE GetVulkanImageInfo(
            VkImage*              pHandle,
            VkImageLayout*        pLayout,
            VkImageCreateInfo*    pInfo);
    
  private:
    
    ID3D11DeviceChild*  m_container;
    D3D11CommonTexture* m_texture;
    
  };
  
  
  ///////////////////////////////////////////
  //      D 3 D 1 1 T E X T U R E 1 D
  class D3D11Texture1D : public D3D11DeviceChild<ID3D11Texture1D> {
    
  public:
    
    D3D11Texture1D(
            D3D11Device*                pDevice,
      const D3D11_COMMON_TEXTURE_DESC*  pDesc);
    
    ~D3D11Texture1D();
    
    HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID  riid,
            void**  ppvObject) final;
    
    void STDMETHODCALLTYPE GetDevice(
            ID3D11Device **ppDevice) final;
    
    void STDMETHODCALLTYPE GetType(
            D3D11_RESOURCE_DIMENSION *pResourceDimension) final;
    
    UINT STDMETHODCALLTYPE GetEvictionPriority() final;
    
    void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) final;
    
    void STDMETHODCALLTYPE GetDesc(
            D3D11_TEXTURE1D_DESC *pDesc) final;
    
    D3D11CommonTexture* GetCommonTexture() {
      return &m_texture;
    }

    D3D10Texture1D* GetD3D10Iface() {
      return &m_d3d10;
    }
    
  private:
    
    D3D11CommonTexture    m_texture;
    D3D11VkInteropSurface m_interop;
    D3D10Texture1D        m_d3d10;
    
  };
  
  
  ///////////////////////////////////////////
  //      D 3 D 1 1 T E X T U R E 2 D
  class D3D11Texture2D : public D3D11DeviceChild<ID3D11Texture2D> {
    
  public:
    
    D3D11Texture2D(
            D3D11Device*                pDevice,
      const D3D11_COMMON_TEXTURE_DESC*  pDesc);
    
    ~D3D11Texture2D();
    
    HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID  riid,
            void**  ppvObject) final;
    
    void STDMETHODCALLTYPE GetDevice(
            ID3D11Device **ppDevice) final;
    
    void STDMETHODCALLTYPE GetType(
            D3D11_RESOURCE_DIMENSION *pResourceDimension) final;
    
    UINT STDMETHODCALLTYPE GetEvictionPriority() final;
    
    void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) final;
    
    void STDMETHODCALLTYPE GetDesc(
            D3D11_TEXTURE2D_DESC *pDesc) final;
    
    D3D11CommonTexture* GetCommonTexture() {
      return &m_texture;
    }
    
    D3D10Texture2D* GetD3D10Iface() {
      return &m_d3d10;
    }

  private:
    
    D3D11CommonTexture    m_texture;
    D3D11VkInteropSurface m_interop;
    D3D10Texture2D        m_d3d10;
    
  };
  
  
  ///////////////////////////////////////////
  //      D 3 D 1 1 T E X T U R E 3 D
  class D3D11Texture3D : public D3D11DeviceChild<ID3D11Texture3D> {
    
  public:
    
    D3D11Texture3D(
            D3D11Device*                pDevice,
      const D3D11_COMMON_TEXTURE_DESC*  pDesc);
    
    ~D3D11Texture3D();
    
    HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID  riid,
            void**  ppvObject) final;
    
    void STDMETHODCALLTYPE GetDevice(
            ID3D11Device **ppDevice) final;
    
    void STDMETHODCALLTYPE GetType(
            D3D11_RESOURCE_DIMENSION *pResourceDimension) final;
    
    UINT STDMETHODCALLTYPE GetEvictionPriority() final;
    
    void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) final;
    
    void STDMETHODCALLTYPE GetDesc(
            D3D11_TEXTURE3D_DESC *pDesc) final;
    
    D3D11CommonTexture* GetCommonTexture() {
      return &m_texture;
    }
    
    D3D10Texture3D* GetD3D10Iface() {
      return &m_d3d10;
    }

  private:
    
    D3D11CommonTexture    m_texture;
    D3D11VkInteropSurface m_interop;
    D3D10Texture3D        m_d3d10;
    
  };
  
  
  /**
   * \brief Retrieves texture from resource pointer
   * 
   * \param [in] pResource The resource to query
   * \returns Pointer to texture info, or \c nullptr
   */
  D3D11CommonTexture* GetCommonTexture(
          ID3D11Resource*       pResource);
  
}
