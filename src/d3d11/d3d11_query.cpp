#include "d3d11_device.h"
#include "d3d11_query.h"

namespace dxvk {
  
  D3D11Query::D3D11Query(
          D3D11Device*      device,
    const D3D11_QUERY_DESC& desc)
  : m_device(device), m_desc(desc),
    m_d3d10(this, device->GetD3D10Interface()) {
    switch (m_desc.Query) {
      case D3D11_QUERY_EVENT:
        m_event = new DxvkEvent();
        break;
        
      case D3D11_QUERY_OCCLUSION:
        m_query = new DxvkQuery(
          VK_QUERY_TYPE_OCCLUSION,
          VK_QUERY_CONTROL_PRECISE_BIT);
        break;
      
      case D3D11_QUERY_OCCLUSION_PREDICATE:
        m_query = new DxvkQuery(
          VK_QUERY_TYPE_OCCLUSION, 0);
        break;
        
      case D3D11_QUERY_TIMESTAMP:
        m_query = new DxvkQuery(
          VK_QUERY_TYPE_TIMESTAMP, 0);
        break;
      
      case D3D11_QUERY_TIMESTAMP_DISJOINT:
        break;
      
      case D3D11_QUERY_PIPELINE_STATISTICS:
        m_query = new DxvkQuery(
          VK_QUERY_TYPE_PIPELINE_STATISTICS, 0);
        break;
      
      case D3D11_QUERY_SO_STATISTICS:
      case D3D11_QUERY_SO_STATISTICS_STREAM0:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM0:
        // FIXME it is technically incorrect to map
        // SO_OVERFLOW_PREDICATE to the first stream,
        // but this is good enough for D3D10 behaviour
        m_query = new DxvkQuery(
          VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, 0, 0);
        break;
      
      case D3D11_QUERY_SO_STATISTICS_STREAM1:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM1:
        m_query = new DxvkQuery(
          VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, 0, 1);
        break;
      
      case D3D11_QUERY_SO_STATISTICS_STREAM2:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM2:
        m_query = new DxvkQuery(
          VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, 0, 2);
        break;
      
      case D3D11_QUERY_SO_STATISTICS_STREAM3:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM3:
        m_query = new DxvkQuery(
          VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, 0, 3);
        break;
      
      default:
        throw DxvkError(str::format("D3D11: Unhandled query type: ", desc.Query));
    }
  }
  
  
  D3D11Query::~D3D11Query() {
    
  }
  
    
  HRESULT STDMETHODCALLTYPE D3D11Query::QueryInterface(REFIID  riid, void** ppvObject) {
    if (ppvObject == nullptr)
      return E_POINTER;

    *ppvObject = nullptr;
    
    if (riid == __uuidof(IUnknown)
     || riid == __uuidof(ID3D11DeviceChild)
     || riid == __uuidof(ID3D11Asynchronous)
     || riid == __uuidof(ID3D11Query)) {
      *ppvObject = ref(this);
      return S_OK;
    }
    
    if (riid == __uuidof(IUnknown)
     || riid == __uuidof(ID3D10DeviceChild)
     || riid == __uuidof(ID3D10Asynchronous)
     || riid == __uuidof(ID3D10Query)) {
      *ppvObject = ref(&m_d3d10);
      return S_OK;
    }
    
    if (m_desc.Query == D3D11_QUERY_OCCLUSION_PREDICATE) {
      if (riid == __uuidof(ID3D11Predicate)) {
        *ppvObject = ref(this);
        return S_OK;
      }

      if (riid == __uuidof(ID3D10Predicate)) {
        *ppvObject = ref(&m_d3d10);
        return S_OK;
      }
    }
    
    Logger::warn("D3D11Query: Unknown interface query");
    Logger::warn(str::format(riid));
    return E_NOINTERFACE;
  }
  
  
  void STDMETHODCALLTYPE D3D11Query::GetDevice(ID3D11Device **ppDevice) {
    *ppDevice = ref(m_device);
  }
  
  
  UINT STDMETHODCALLTYPE D3D11Query::GetDataSize() {
    switch (m_desc.Query) {
      case D3D11_QUERY_EVENT:
        return sizeof(BOOL);
      
      case D3D11_QUERY_OCCLUSION:
        return sizeof(UINT64);
      
      case D3D11_QUERY_TIMESTAMP:
        return sizeof(UINT64);
      
      case D3D11_QUERY_TIMESTAMP_DISJOINT:
        return sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT);
      
      case D3D11_QUERY_PIPELINE_STATISTICS:
        return sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS);
      
      case D3D11_QUERY_OCCLUSION_PREDICATE:
        return sizeof(BOOL);
      
      case D3D11_QUERY_SO_STATISTICS:
      case D3D11_QUERY_SO_STATISTICS_STREAM0:
      case D3D11_QUERY_SO_STATISTICS_STREAM1:
      case D3D11_QUERY_SO_STATISTICS_STREAM2:
      case D3D11_QUERY_SO_STATISTICS_STREAM3:
        return sizeof(D3D11_QUERY_DATA_SO_STATISTICS);
      
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM0:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM1:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM2:
      case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM3:
        return sizeof(BOOL);
    }
    
    Logger::err("D3D11Query: Failed to query data size");
    return 0;
  }
  
    
  void STDMETHODCALLTYPE D3D11Query::GetDesc(D3D11_QUERY_DESC *pDesc) {
    *pDesc = m_desc;
  }
  
  
  uint32_t D3D11Query::Reset() {
    if (m_query != nullptr)
      return m_query->reset();
    
    if (m_event != nullptr)
      return m_event->reset();
    
    return 0;
  }
  
  
  bool D3D11Query::HasBeginEnabled() const {
    return m_desc.Query != D3D11_QUERY_EVENT
        && m_desc.Query != D3D11_QUERY_TIMESTAMP;
  }
  
  
  void D3D11Query::Begin(DxvkContext* ctx, uint32_t revision) {
    m_revision = revision;
    
    if (m_query != nullptr) {
      DxvkQueryRevision rev = { m_query, revision };
      ctx->beginQuery(rev);
    }
  }
  
  
  void D3D11Query::End(DxvkContext* ctx) {
    if (m_query != nullptr) {
      DxvkQueryRevision rev = { m_query, m_revision };
      ctx->endQuery(rev);
    }
  }
  
  
  void D3D11Query::Signal(DxvkContext* ctx, uint32_t revision) {
    switch (m_desc.Query) {
      case D3D11_QUERY_EVENT: {
        DxvkEventRevision rev = { m_event, revision };
        ctx->signalEvent(rev);
      } break;
      
      case D3D11_QUERY_TIMESTAMP: {
        DxvkQueryRevision rev = { m_query, revision };
        ctx->writeTimestamp(rev);
      } break;
      
      default:
        break;
    }
  }
  
  
  HRESULT STDMETHODCALLTYPE D3D11Query::GetData(
          void*                             pData,
          UINT                              GetDataFlags) {
    if (m_desc.Query == D3D11_QUERY_EVENT) {
      const bool signaled = m_event->getStatus() == DxvkEventStatus::Signaled;
      
      if (pData != nullptr)
        *static_cast<BOOL*>(pData) = signaled;
      
      return signaled ? S_OK : S_FALSE;
    } else {
      DxvkQueryData queryData = {};
      
      if (m_query != nullptr) {
        DxvkQueryStatus status = m_query->getData(queryData);

        if (status == DxvkQueryStatus::Created)
          return DXGI_ERROR_INVALID_CALL;
        
        if (status != DxvkQueryStatus::Available)
          return S_FALSE;
      }
      
      if (pData == nullptr)
        return S_OK;
      
      switch (m_desc.Query) {
        case D3D11_QUERY_OCCLUSION:
          *static_cast<UINT64*>(pData) = queryData.occlusion.samplesPassed;
          return S_OK;
        
        case D3D11_QUERY_OCCLUSION_PREDICATE:
          *static_cast<BOOL*>(pData) = queryData.occlusion.samplesPassed != 0;
          return S_OK;
        
        case D3D11_QUERY_TIMESTAMP:
          *static_cast<UINT64*>(pData) = queryData.timestamp.time;
          return S_OK;
        
        case D3D11_QUERY_TIMESTAMP_DISJOINT: {
          auto data = static_cast<D3D11_QUERY_DATA_TIMESTAMP_DISJOINT*>(pData);
          data->Frequency = GetTimestampQueryFrequency();
          data->Disjoint = FALSE;
        } return S_OK;
        
        case D3D11_QUERY_PIPELINE_STATISTICS: {
          auto data = static_cast<D3D11_QUERY_DATA_PIPELINE_STATISTICS*>(pData);
          data->IAVertices    = queryData.statistic.iaVertices;
          data->IAPrimitives  = queryData.statistic.iaPrimitives;
          data->VSInvocations = queryData.statistic.vsInvocations;
          data->GSInvocations = queryData.statistic.gsInvocations;
          data->GSPrimitives  = queryData.statistic.gsPrimitives;
          data->CInvocations  = queryData.statistic.clipInvocations;
          data->CPrimitives   = queryData.statistic.clipPrimitives;
          data->PSInvocations = queryData.statistic.fsInvocations;
          data->HSInvocations = queryData.statistic.tcsPatches;
          data->DSInvocations = queryData.statistic.tesInvocations;
          data->CSInvocations = queryData.statistic.csInvocations;
        } return S_OK;

        case D3D11_QUERY_SO_STATISTICS:
        case D3D11_QUERY_SO_STATISTICS_STREAM0:
        case D3D11_QUERY_SO_STATISTICS_STREAM1:
        case D3D11_QUERY_SO_STATISTICS_STREAM2:
        case D3D11_QUERY_SO_STATISTICS_STREAM3: {
          auto data = static_cast<D3D11_QUERY_DATA_SO_STATISTICS*>(pData);
          data->NumPrimitivesWritten    = queryData.xfbStream.primitivesWritten;
          data->PrimitivesStorageNeeded = queryData.xfbStream.primitivesNeeded;
        } return S_OK;
          
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM0:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM1:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM2:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM3: {
          auto data = static_cast<BOOL*>(pData);
          *data = queryData.xfbStream.primitivesNeeded
                > queryData.xfbStream.primitivesWritten;
        } return S_OK;

        default:
          Logger::err(str::format("D3D11: Unhandled query type in GetData: ", m_desc.Query));
          return E_INVALIDARG;
      }
    }
  }
  
  
  UINT64 D3D11Query::GetTimestampQueryFrequency() const {
    Rc<DxvkDevice>  device  = m_device->GetDXVKDevice();
    Rc<DxvkAdapter> adapter = device->adapter();

    VkPhysicalDeviceLimits limits = adapter->deviceProperties().limits;
    return uint64_t(1'000'000'000.0f / limits.timestampPeriod);
  }
  
}
