#include <vector>

#include <atomContainer/hashmap.hpp>
#include <atomGraphics/backend/vulkan/vulkan_utils.h>
#include <atomGraphics/backend/vulkan/agpu_vulkan_exts.h>

class VulkanDeviceContext
{
public:
    VulkanDeviceContext(AGPUInstanceDescriptor const* desc)
    {
        const VulkanInstanceDescriptor* exts_desc = (const VulkanInstanceDescriptor*)desc->chained;
        // default
        device_extensions.insert(device_extensions.end(),
                                 std::begin(AGPU_wanted_device_exts),
                                 std::end(AGPU_wanted_device_exts));
        instance_extensions.insert(instance_extensions.end(),
                                   std::begin(AGPU_wanted_instance_exts),
                                   std::end(AGPU_wanted_instance_exts));
        // from desc
        if (desc->enable_debug_layer) { instance_layers.push_back(validation_layer_name); }
        if (desc->enable_set_name) {
            instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        // Merge All Parameters into one blackboard
        if (exts_desc != ATOM_NULLPTR) // Extensions
        {
            if (exts_desc->backend != AGPU_BACKEND_VULKAN) {
                atom_assert(exts_desc->backend == AGPU_BACKEND_VULKAN
                            && "Chained Instance Descriptor must have a vulkan backend!");
                exts_desc = ATOM_NULLPTR;
            } else {
                messenger_info_ptr = exts_desc->pDebugUtilsMessenger;
                report_info_ptr    = exts_desc->pDebugReportMessenger;
                // Merge Instance Extension Names
                if (exts_desc->ppInstanceExtensions != NULL && exts_desc->mInstanceExtensionCount != 0) {
                    instance_extensions.insert(instance_extensions.end(),
                                               exts_desc->ppInstanceExtensions,
                                               exts_desc->ppInstanceExtensions + exts_desc->mInstanceExtensionCount);
                }
                // Merge Instance Layer Names
                if (exts_desc->ppInstanceLayers != NULL && exts_desc->mInstanceLayerCount != 0) {
                    instance_layers.insert(instance_layers.end(),
                                           exts_desc->ppInstanceLayers,
                                           exts_desc->ppInstanceLayers + exts_desc->mInstanceLayerCount);
                }
                // Merge Device Extension Names
                if (exts_desc->ppDeviceExtensions != NULL && exts_desc->mDeviceExtensionCount != 0) {
                    device_extensions.insert(device_extensions.end(),
                                             exts_desc->ppDeviceExtensions,
                                             exts_desc->ppDeviceExtensions + exts_desc->mDeviceExtensionCount);
                }
            }
        }
    }

    const VkDebugUtilsMessengerCreateInfoEXT* messenger_info_ptr = ATOM_NULLPTR;
    const VkDebugReportCallbackCreateInfoEXT* report_info_ptr    = ATOM_NULLPTR;
    std::vector<const char*>                  instance_extensions;
    std::vector<const char*>                  instance_layers;
    std::vector<const char*>                  device_extensions;
    std::vector<const char*>                  device_layers;
};

struct AGPUCachedRenderPass {
    VkRenderPass pass;
    size_t       timestamp;
};

struct AGPUCachedFramebuffer {
    VkFramebuffer framebuffer;
    size_t        timestamp;
};

struct VulkanRenderPassTable //
{
    struct rpdesc_hash {
        size_t operator()(const VulkanRenderPassDescriptor& a) const
        {
            return atom_hash(&a, sizeof(VulkanRenderPassDescriptor), AGPU_NAME_HASH_SEED);
        }
    };

    struct rpdesc_eq {
        inline bool operator()(const VulkanRenderPassDescriptor& a, const VulkanRenderPassDescriptor& b) const
        {
            if (a.mColorAttachmentCount != b.mColorAttachmentCount) return false;
            return std::memcmp(&a, &b, sizeof(VulkanRenderPassDescriptor)) == 0;
        }
    };

    struct fbdesc_hash {
        size_t operator()(const VulkanFramebufferDescriptor& a) const
        {
            return atom_hash(&a, sizeof(VulkanFramebufferDescriptor), AGPU_NAME_HASH_SEED);
        }
    };

    struct fbdesc_eq {
        inline bool operator()(const VulkanFramebufferDescriptor& a, const VulkanFramebufferDescriptor& b) const
        {
            if (a.pRenderPass != b.pRenderPass) return false;
            if (a.mAttachmentCount != b.mAttachmentCount) return false;
            return std::memcmp(&a, &b, sizeof(VulkanRenderPassDescriptor)) == 0;
        }
    };

    atom::flat_hash_map<VulkanRenderPassDescriptor, AGPUCachedRenderPass, rpdesc_hash, rpdesc_eq>   cached_renderpasses;
    atom::flat_hash_map<VulkanFramebufferDescriptor, AGPUCachedFramebuffer, fbdesc_hash, fbdesc_eq> cached_framebuffers;
};

VkFramebuffer vulkan_frame_buffer_table_try_bind(struct VulkanRenderPassTable* table, const VulkanFramebufferDescriptor* desc)
{
    const auto& iter = table->cached_framebuffers.find(*desc);
    if (iter != table->cached_framebuffers.end()) { return iter->second.framebuffer; }
    return VK_NULL_HANDLE;
}

void vulkan_frame_buffer_table_add(struct VulkanRenderPassTable*             table,
                                   const struct VulkanFramebufferDescriptor* desc,
                                   VkFramebuffer                             framebuffer)
{
    const auto& iter = table->cached_framebuffers.find(*desc);
    if (iter != table->cached_framebuffers.end()) { ATOM_warn(u8"Vulkan Framebuffer with this desc already exists!"); }
    // TODO: Add timestamp
    AGPUCachedFramebuffer new_fb      = {framebuffer, 0};
    table->cached_framebuffers[*desc] = new_fb;
}

VkRenderPass vulkan_render_pass_table_try_find(struct VulkanRenderPassTable*            table,
                                               const struct VulkanRenderPassDescriptor* desc)
{
    const auto& iter = table->cached_renderpasses.find(*desc);
    if (iter != table->cached_renderpasses.end()) { return iter->second.pass; }
    return VK_NULL_HANDLE;
}

void vulkan_render_pass_table_add(struct VulkanRenderPassTable*            table,
                                  const struct VulkanRenderPassDescriptor* desc,
                                  VkRenderPass                             pass)
{
    const auto& iter = table->cached_renderpasses.find(*desc);
    if (iter != table->cached_renderpasses.end()) { ATOM_warn(u8"Vulkan Pass with this desc already exists!"); }
    // TODO: Add timestamp
    AGPUCachedRenderPass new_pass     = {pass, 0};
    table->cached_renderpasses[*desc] = new_pass;
}

struct VulkanExtensionTable : public atom::parallel_flat_hash_map<std::string, bool> //
{
    static void ConstructForAllAdapters(struct VulkanInstance* I, const VulkanDeviceContext& blackboard)
    {
        // enum physical devices & store informations.
        auto       wanted_device_extensions       = blackboard.device_extensions.data();
        const auto wanted_device_extensions_count = (uint32_t)blackboard.device_extensions.size();
        // construct extensions table
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++) {
            auto& Adapter            = I->pVulkanAdapters[i];
            Adapter.pExtensionsTable = atom_new<VulkanExtensionTable>();
            auto& Table              = *Adapter.pExtensionsTable;
            for (uint32_t j = 0; j < wanted_device_extensions_count; j++) { Table[wanted_device_extensions[j]] = false; }
            for (uint32_t j = 0; j < Adapter.mExtensionsCount; j++) {
                const auto extension_name = Adapter.pExtensionNames[j];
                Table[extension_name]     = true;
            }
            // Cache
            {
                Adapter.buffer_device_address = Table[VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME];
                Adapter.descriptor_buffer     = Table[VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME];
                Adapter.descriptor_indexing   = Table[VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME];

                Adapter.debug_marker         = Table[VK_EXT_DEBUG_MARKER_EXTENSION_NAME];
                Adapter.dedicated_allocation = Table[VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME];
                Adapter.memory_req2          = Table[VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME];
                Adapter.external_memory      = Table[VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME];
#ifdef _WIN32
                Adapter.external_memory_win32  = Table[VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME];
                Adapter.external_memory       &= Adapter.external_memory_win32;
#endif
                Adapter.draw_indirect_count     = Table[VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME];
                Adapter.amd_draw_indirect_count = Table[VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME];
                Adapter.amd_gcn_shader          = Table[VK_AMD_GCN_SHADER_EXTENSION_NAME];
                Adapter.sampler_ycbcr           = Table[VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME];
            }
        }
    }

    static void ConstructForInstance(struct VulkanInstance* I, const VulkanDeviceContext& blackboard)
    {
        // enum physical devices & store informations.
        auto       wanted_instance_extensions       = blackboard.instance_extensions.data();
        const auto wanted_instance_extensions_count = (uint32_t)blackboard.instance_extensions.size();
        // construct extensions table
        I->pExtensionsTable                         = atom_new<VulkanExtensionTable>();
        auto& Table                                 = *I->pExtensionsTable;
        for (uint32_t j = 0; j < wanted_instance_extensions_count; j++) {
            const auto key = wanted_instance_extensions[j];
            Table.insert_or_assign(key, false);
        }
        for (uint32_t j = 0; j < I->mExtensionsCount; j++) {
            const auto key = I->pExtensionNames[j];
            Table.insert_or_assign(key, true);
        }
        // Cache
        {
            I->device_group_creation = Table[VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME]; // Linked GPU
            I->debug_utils           = Table[VK_EXT_DEBUG_UTILS_EXTENSION_NAME];
            I->debug_report          = !I->debug_utils && Table[VK_EXT_DEBUG_REPORT_EXTENSION_NAME];
        }
    }
};

struct VulkanLayerTable : public atom::parallel_flat_hash_map<std::string, bool> //
{
    static void ConstructForAllAdapters(struct VulkanInstance* I, const VulkanDeviceContext& blackboard)
    {
        // enum physical devices & store informations.
        auto       wanted_device_layers       = blackboard.device_layers.data();
        const auto wanted_device_layers_count = (uint32_t)blackboard.device_layers.size();
        // construct layers table
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++) {
            auto& Adapter        = I->pVulkanAdapters[i];
            Adapter.pLayersTable = atom_new<VulkanLayerTable>();
            auto& Table          = *Adapter.pLayersTable;
            for (uint32_t j = 0; j < wanted_device_layers_count; j++) { Table[wanted_device_layers[j]] = false; }
            for (uint32_t j = 0; j < Adapter.mLayersCount; j++) { Table[Adapter.pLayerNames[j]] = true; }
        }
    }

    static void ConstructForInstance(struct VulkanInstance* I, const VulkanDeviceContext& blackboard)
    {
        // enum physical devices & store informations.
        auto       wanted_instance_layers       = blackboard.instance_layers.data();
        const auto wanted_instance_layers_count = (uint32_t)blackboard.instance_layers.size();
        // construct layers table
        I->pLayersTable                         = atom_new<VulkanLayerTable>();
        auto& Table                             = *I->pLayersTable;
        for (uint32_t j = 0; j < wanted_instance_layers_count; j++) { Table[wanted_instance_layers[j]] = false; }
        for (uint32_t j = 0; j < I->mLayersCount; j++) { Table[I->pLayerNames[j]] = true; }
    }
};

AGPUInstanceIter agpu_create_instance_vulkan(AGPUInstanceDescriptor const* desc)
{
    // Merge All Parameters into one blackboard
    const VulkanDeviceContext blackboard(desc);

    // Memory Alloc
    VulkanInstance* I = (VulkanInstance*)atom_calloc(1, sizeof(VulkanInstance));
    ::memset(I, 0, sizeof(VulkanInstance));

    // Initialize Environment
    vulkan_initialize_environment(&I->super);

    // Create VkInstance.
    ATOM_DECLARE_ZERO(VkApplicationInfo, appInfo)
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "AGPU";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_1;

    // Select Instance Layers & Layer Extensions
    vulkan_select_instance_layers(I, blackboard.instance_layers.data(), (uint32_t)blackboard.instance_layers.size());
    // Select Instance Extensions
    vulkan_select_instance_extensions(I,
                                      blackboard.instance_extensions.data(),
                                      (uint32_t)blackboard.instance_extensions.size());

    ATOM_DECLARE_ZERO(VkInstanceCreateInfo, createInfo)
#if defined(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) && defined(_MACOS)
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    // Instance Layers
    createInfo.enabledLayerCount       = I->mLayersCount;
    createInfo.ppEnabledLayerNames     = I->pLayerNames;
    // Instance Extensions
    createInfo.enabledExtensionCount   = I->mExtensionsCount;
    createInfo.ppEnabledExtensionNames = I->pExtensionNames;

    // List Validation Features
    ATOM_DECLARE_ZERO(VkValidationFeaturesEXT, validationFeaturesExt)
    validationFeaturesExt.sType                              = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    VkValidationFeatureEnableEXT enabledValidationFeatures[] = {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
    };
    if (desc->enable_gpu_based_validation) {
        if (!desc->enable_debug_layer) {
            ATOM_warn(u8"Vulkan GpuBasedValidation enabled while ValidationLayer is closed, there'll be no effect.");
        }
#if VK_HEADER_VERSION >= 108
        validationFeaturesExt.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validationFeaturesExt.enabledValidationFeatureCount =
            sizeof(enabledValidationFeatures) / sizeof(VkValidationFeatureEnableEXT);
        validationFeaturesExt.pEnabledValidationFeatures = enabledValidationFeatures;
        createInfo.pNext                                 = &validationFeaturesExt;
#else
        ATOM_warn("Vulkan GpuBasedValidation enabled but VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT is not supported!\n");
#endif
    }

    auto instRes = (int32_t)vkCreateInstance(&createInfo, GLOBAL_VkAllocationCallbacks, &I->pVkInstance);
    if (instRes != VK_SUCCESS) {
        ATOM_fatal(u8"Vulkan: failed to create instance with code %d", instRes);
        atom_assert(0 && "Vulkan: failed to create instance!");
    }
    VulkanLayerTable::ConstructForInstance(I, blackboard);
    VulkanExtensionTable::ConstructForInstance(I, blackboard);

#if defined(NX64)
    loadExtensionsNX(result->pVkInstance);
#else
    // Load Vulkan instance functions
    volkLoadInstance(I->pVkInstance);
#endif

    // enum physical devices & store informations.
    const char* const* wanted_device_extensions       = blackboard.device_extensions.data();
    const auto         wanted_device_extensions_count = (uint32_t)blackboard.device_extensions.size();
    const char* const* wanted_device_layers           = blackboard.device_layers.data();
    const auto         wanted_device_layers_count     = (uint32_t)blackboard.device_layers.size();
    vulkan_query_all_adapters(I,
                              wanted_device_layers,
                              wanted_device_layers_count,
                              wanted_device_extensions,
                              wanted_device_extensions_count);
    // sort by GPU type
    std::stable_sort(I->pVulkanAdapters,
                     I->pVulkanAdapters + I->mPhysicalDeviceCount,
                     [](const VulkanAdapter& a, const VulkanAdapter& b) {
                         const uint32_t orders[] = {4, 1, 0, 2, 3};
                         return orders[a.mPhysicalDeviceProps.properties.deviceType]
                                < orders[b.mPhysicalDeviceProps.properties.deviceType];
                     });
    // construct extensions table
    VulkanLayerTable::ConstructForAllAdapters(I, blackboard);
    VulkanExtensionTable::ConstructForAllAdapters(I, blackboard);

    // Open validation layer.
    if (desc->enable_debug_layer) {
        vulkan_enable_validation_layer(I, blackboard.messenger_info_ptr, blackboard.report_info_ptr);
    }

    return &(I->super);
}

void agpu_free_instance_vulkan(AGPUInstanceIter instance)
{
    VulkanInstance* to_destroy = (VulkanInstance*)instance;
    vulkan_deinitialize_environment(&to_destroy->super);
    if (to_destroy->pVkDebugUtilsMessenger) {
        atom_assert(vkDestroyDebugUtilsMessengerEXT && "Load vkDestroyDebugUtilsMessengerEXT failed!");
        vkDestroyDebugUtilsMessengerEXT(to_destroy->pVkInstance,
                                        to_destroy->pVkDebugUtilsMessenger,
                                        GLOBAL_VkAllocationCallbacks);
    }

    vkDestroyInstance(to_destroy->pVkInstance, GLOBAL_VkAllocationCallbacks);
    for (uint32_t i = 0; i < to_destroy->mPhysicalDeviceCount; i++) {
        auto& Adapter = to_destroy->pVulkanAdapters[i];
        atom_free(Adapter.pQueueFamilyProperties);
        // free extensions cache
        atom_delete(Adapter.pExtensionsTable);
        atom_free(Adapter.pExtensionNames);
        atom_free(Adapter.pExtensionProperties);

        // free layers cache
        atom_delete(Adapter.pLayersTable);
        atom_free(Adapter.pLayerNames);
        atom_free(Adapter.pLayerProperties);
    }
    // free extensions cache
    atom_delete(to_destroy->pExtensionsTable);
    atom_free(to_destroy->pExtensionNames);
    atom_free(to_destroy->pExtensionProperties);
    // free layers cache
    atom_delete(to_destroy->pLayersTable);
    atom_free(to_destroy->pLayerNames);
    atom_free(to_destroy->pLayerProperties);

    atom_free(to_destroy->pVulkanAdapters);
    atom_free(to_destroy);
}

const float queuePriorities[] = {
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
};

AGPUDeviceIter agpu_create_device_vulkan(AGPUAdapterIter adapter, const AGPUDeviceDescriptor* desc)
{
    VulkanInstance* I = (VulkanInstance*)adapter->instance;
    VulkanDevice*   D = (VulkanDevice*)atom_calloc(1, sizeof(VulkanDevice));
    VulkanAdapter*  A = (VulkanAdapter*)adapter;

    *const_cast<AGPUAdapterIter*>(&D->super.adapter) = adapter;

    // Prepare Create Queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.resize(desc->queue_group_count);
    for (uint32_t i = 0; i < desc->queue_group_count; i++) {
        VkDeviceQueueCreateInfo&  info       = queueCreateInfos[i];
        AGPUQueueGroupDescriptor& descriptor = desc->queue_groups[i];
        info.sType                           = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueCount                      = descriptor.queue_count;
        info.queueFamilyIndex                = (uint32_t)A->mQueueFamilyIndices[descriptor.queue_type];
        info.pQueuePriorities                = queuePriorities;

        atom_assert(info.queueCount <= A->pQueueFamilyProperties[info.queueFamilyIndex].queueCount
                    && "allocated too many queues!");
    }
    // Create Device
    ATOM_DECLARE_ZERO(VkDeviceCreateInfo, createInfo)
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                   = &A->mPhysicalDeviceFeatures;
    createInfo.queueCreateInfoCount    = (uint32_t)queueCreateInfos.size();
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = NULL;
    createInfo.enabledExtensionCount   = A->mExtensionsCount;
    createInfo.ppEnabledExtensionNames = A->pExtensionNames;
    createInfo.enabledLayerCount       = A->mLayersCount;
    createInfo.ppEnabledLayerNames     = A->pLayerNames;

    VkResult result = vkCreateDevice(A->pPhysicalDevice, &createInfo, GLOBAL_VkAllocationCallbacks, &D->pVkDevice);
    if (result != VK_SUCCESS) { atom_assert(0 && "failed to create logical device!"); }

    // Single Device Only.
    volkLoadDeviceTable(&D->mVkDeviceTable, D->pVkDevice);
    atom_assert(D->mVkDeviceTable.vkCreateSwapchainKHR && "failed to load swapchain proc!");

    // Create Pipeline Cache
    D->pPipelineCache = ATOM_NULLPTR;
    if (!desc->disable_pipeline_cache) { vulkan_create_pipeline_cache(D); }

    // Create VMA Allocator
    vulkan_create_vma_allocator(I, A, D);
    // Create Descriptor Heap
    D->pDescriptorPool = vulkan_create_desciptor_pool(D);
    // Create pass table
    D->pPassTable      = atom_new<VulkanRenderPassTable>();
    return &D->super;
}

void agpu_free_device_vulkan(AGPUDeviceIter device)
{
    VulkanDevice*   D = (VulkanDevice*)device;
    VulkanAdapter*  A = (VulkanAdapter*)device->adapter;
    VulkanInstance* I = (VulkanInstance*)device->adapter->instance;

    for (auto& iter : D->pPassTable->cached_renderpasses) {
        D->mVkDeviceTable.vkDestroyRenderPass(D->pVkDevice, iter.second.pass, GLOBAL_VkAllocationCallbacks);
    }
    for (auto& iter : D->pPassTable->cached_framebuffers) {
        D->mVkDeviceTable.vkDestroyFramebuffer(D->pVkDevice, iter.second.framebuffer, GLOBAL_VkAllocationCallbacks);
    }
    atom_delete(D->pPassTable);

    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if (D->pExternalMemoryVmaPools[i]) { vmaDestroyPool(D->pVmaAllocator, D->pExternalMemoryVmaPools[i]); }
        if (D->pExternalMemoryVmaPoolNexts[i]) { atom_free(D->pExternalMemoryVmaPoolNexts[i]); }
    }
    vulkan_free_vma_allocator(I, A, D);
    vulkan_free_descriptor_pool(D->pDescriptorPool);
    vulkan_free_pipeline_cache(I, A, D);
    vkDestroyDevice(D->pVkDevice, GLOBAL_VkAllocationCallbacks);
    atom_free(D);
}
