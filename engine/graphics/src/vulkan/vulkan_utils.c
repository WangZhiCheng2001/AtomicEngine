#include <stdio.h>
#include <string.h>

#include <../Source/SPIRV-Reflect/spirv_reflect.h>

#include <atomGraphics/backend/vulkan/vulkan_utils.h>
#include <atomGraphics/backend/vulkan/agpu_vulkan.h>
#include <atomGraphics/common/flags.h>

#ifdef AGPU_THREAD_SAFETY
#include <threads.h>
#endif

bool vulkan_initialize_environment(struct AGPUInstance* Inst)
{
    Inst->runtime_table = agpu_create_runtime_table();
    // VOLK
    VkResult volkInit   = volkInitialize();
    if (volkInit != VK_SUCCESS) {
        atom_assert((volkInit == VK_SUCCESS) && "Volk Initialize Failed!");
        return false;
    }

    return true;
}

void vulkan_deinitialize_environment(struct AGPUInstance* Inst) {}

typedef struct VulkanMessageToSkip {
    const char* what;
    uint64_t    hash;
} VulkanMessageToSkip;

VulkanMessageToSkip kSkippedMessages[] = {
    {"UNASSIGNED-BestPractices-vkCreateDevice-deprecated-extension"},
};

ATOM_FORCEINLINE bool vulkan_try_ignore_message(const char* MessageIter, bool Scan)
{
    if (!MessageIter) return false;
    if (Scan) {
        for (uint32_t i = 0; i < sizeof(kSkippedMessages) / sizeof(VulkanMessageToSkip); ++i) {
            if (strstr(kSkippedMessages[i].what, MessageIter) != ATOM_NULLPTR) return true;
        }
    } else {
        const uint64_t msg_hash = atom_hash(MessageIter, strlen(MessageIter), AGPU_NAME_HASH_SEED);
        for (uint32_t i = 0; i < sizeof(kSkippedMessages) / sizeof(VulkanMessageToSkip); ++i) {
            const uint64_t hash = kSkippedMessages[i].hash;
            if (msg_hash != hash) continue;
            if (strcmp(kSkippedMessages[i].what, MessageIter) == 0) return true;
        }
    }
    return false;
}

ATOM_FORCEINLINE void vulkan_initialize_message_to_skip()
{
    for (uint32_t i = 0; i < sizeof(kSkippedMessages) / sizeof(VulkanMessageToSkip); ++i) {
        const char* what         = kSkippedMessages[i].what;
        kSkippedMessages[i].hash = atom_hash(what, strlen(what), AGPU_NAME_HASH_SEED);
    }
}

// Instance APIs
void vulkan_enable_validation_layer(VulkanInstance*                           I,
                                    const VkDebugUtilsMessengerCreateInfoEXT* messenger_info_ptr,
                                    const VkDebugReportCallbackCreateInfoEXT* report_info_ptr)
{
    vulkan_initialize_message_to_skip();
    if (I->debug_utils) {
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pfnUserCallback = vulkan_debug_utils_callback,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                           | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .flags     = 0,
            .pUserData = NULL};
        const VkDebugUtilsMessengerCreateInfoEXT* messengerInfoPtr =
            (messenger_info_ptr != ATOM_NULLPTR) ? messenger_info_ptr : &messengerInfo;

        atom_assert(vkCreateDebugUtilsMessengerEXT && "Load vkCreateDebugUtilsMessengerEXT failed!");
        VkResult res = vkCreateDebugUtilsMessengerEXT(I->pVkInstance,
                                                      messengerInfoPtr,
                                                      GLOBAL_VkAllocationCallbacks,
                                                      &(I->pVkDebugUtilsMessenger));
        if (VK_SUCCESS != res) { atom_assert(0 && "vkCreateDebugUtilsMessengerEXT failed - disabling Vulkan debug callbacks"); }
    } else if (I->debug_report) {
        VkDebugReportCallbackCreateInfoEXT reportInfo = {
            .sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
            .pNext       = NULL,
            .pfnCallback = vulkan_debug_report_callback,
            .flags       = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT
                     | VK_DEBUG_REPORT_DEBUG_BIT_EXT /* | VK_DEBUG_REPORT_INFORMATION_BIT_EXT*/
        };
        const VkDebugReportCallbackCreateInfoEXT* reportInfoPtr =
            (report_info_ptr != ATOM_NULLPTR) ? report_info_ptr : &reportInfo;
        VkResult res =
            vkCreateDebugReportCallbackEXT(I->pVkInstance, reportInfoPtr, GLOBAL_VkAllocationCallbacks, &(I->pVkDebugReport));
        atom_assert(vkCreateDebugUtilsMessengerEXT && "Load vkCreateDebugReportCallbackEXT failed!");
        if (VK_SUCCESS != res) { atom_assert(0 && "vkCreateDebugReportCallbackEXT failed - disabling Vulkan debug callbacks"); }
    }
}

void vulkan_query_all_adapters(VulkanInstance*    I,
                               const char* const* device_layers,
                               uint32_t           device_layers_count,
                               const char* const* device_extensions,
                               uint32_t           device_extension_count)
{
    atom_assert((I->mPhysicalDeviceCount == 0) && "vulkan_query_all_adapters should only be called once!");

    vkEnumeratePhysicalDevices(I->pVkInstance, &I->mPhysicalDeviceCount, ATOM_NULLPTR);
    if (I->mPhysicalDeviceCount != 0) {
        I->pVulkanAdapters = (VulkanAdapter*)atom_calloc(I->mPhysicalDeviceCount, sizeof(VulkanAdapter));
        ATOM_DECLARE_ZERO_VLA(VkPhysicalDevice, pysicalDevices, I->mPhysicalDeviceCount)
        vkEnumeratePhysicalDevices(I->pVkInstance, &I->mPhysicalDeviceCount, pysicalDevices);
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++) {
            // Alloc & Zero Adapter
            VulkanAdapter* VkAdapter = &I->pVulkanAdapters[i];
            for (uint32_t q = 0; q < AGPU_QUEUE_TYPE_COUNT; q++) { VkAdapter->mQueueFamilyIndices[q] = -1; }
            VkAdapter->pPhysicalDevice            = pysicalDevices[i];
            // Query Physical Device Properties
            VkAdapter->mPhysicalDeviceProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
            // Append pNexts
            {
                void** ppNext                        = &VkAdapter->mPhysicalDeviceProps.pNext;
                VkAdapter->mSubgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
                VkAdapter->mSubgroupProperties.pNext = NULL;
                *ppNext                              = &VkAdapter->mSubgroupProperties;
                ppNext                               = &VkAdapter->mSubgroupProperties.pNext;
#if VK_KHR_depth_stencil_resolve
                VkAdapter->mPhysicalDeviceDepthStencilResolveProps.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES_KHR;
                *ppNext = &VkAdapter->mPhysicalDeviceDepthStencilResolveProps;
                ppNext  = &VkAdapter->mPhysicalDeviceDepthStencilResolveProps.pNext;
#endif
#if VK_EXT_extended_dynamic_state3
                VkAdapter->mPhysicalDeviceExtendedDynamicState3Properties.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceExtendedDynamicState3Properties;
                ppNext  = &VkAdapter->mPhysicalDeviceExtendedDynamicState3Properties.pNext;
#endif
#if VK_EXT_shader_object
                VkAdapter->mPhysicalDeviceShaderObjectProperties.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceShaderObjectProperties;
                ppNext  = &VkAdapter->mPhysicalDeviceShaderObjectProperties.pNext;
#endif

#if VK_EXT_descriptor_buffer
                VkAdapter->mPhysicalDeviceDescriptorBufferProperties.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceDescriptorBufferProperties;
                ppNext  = &VkAdapter->mPhysicalDeviceDescriptorBufferProperties.pNext;
#endif
            }
            vkGetPhysicalDeviceProperties2KHR(pysicalDevices[i], &VkAdapter->mPhysicalDeviceProps);
            // Query Physical Device Features
            VkAdapter->mPhysicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            // Append pNexts
            {
                void** ppNext = &VkAdapter->mPhysicalDeviceFeatures.pNext;
#if VK_KHR_buffer_device_address
                VkAdapter->mPhysicalDeviceBufferDeviceAddressFeatures.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceBufferDeviceAddressFeatures;
                ppNext  = &VkAdapter->mPhysicalDeviceBufferDeviceAddressFeatures.pNext;
#endif
#if VK_EXT_descriptor_buffer
                VkAdapter->mPhysicalDeviceDescriptorBufferFeatures.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceDescriptorBufferFeatures;
                ppNext  = &VkAdapter->mPhysicalDeviceDescriptorBufferFeatures.pNext;
#endif

#if VK_KHR_dynamic_rendering
                VkAdapter->mPhysicalDeviceDynamicRenderingFeatures.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
                *ppNext = &VkAdapter->mPhysicalDeviceDynamicRenderingFeatures;
                ppNext  = &VkAdapter->mPhysicalDeviceDynamicRenderingFeatures.pNext;
#endif
#if VK_EXT_extended_dynamic_state
                VkAdapter->mPhysicalDeviceExtendedDynamicStateFeatures.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceExtendedDynamicStateFeatures;
                ppNext  = &VkAdapter->mPhysicalDeviceExtendedDynamicStateFeatures.pNext;
#endif
#if VK_EXT_extended_dynamic_state2
                VkAdapter->mPhysicalDeviceExtendedDynamicState2Features.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceExtendedDynamicState2Features;
                ppNext  = &VkAdapter->mPhysicalDeviceExtendedDynamicState2Features.pNext;
#endif
#if VK_EXT_extended_dynamic_state3
                VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceExtendedDynamicState3Features;
                ppNext  = &VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.pNext;
#endif
#if VK_EXT_shader_object
                VkAdapter->mPhysicalDeviceShaderObjectFeatures.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
                *ppNext = &VkAdapter->mPhysicalDeviceShaderObjectFeatures;
                ppNext  = &VkAdapter->mPhysicalDeviceShaderObjectFeatures.pNext;
#endif
            }
            vkGetPhysicalDeviceFeatures2KHR(pysicalDevices[i], &VkAdapter->mPhysicalDeviceFeatures);
            // Enumerate Format Supports
            vulkan_enumerate_format_supports(VkAdapter);
            // Query Physical Device Layers Properties
            vulkan_select_physical_device_layers(VkAdapter, device_layers, device_layers_count);
            // Query Physical Device Extension Properties
            vulkan_select_physical_device_extensions(VkAdapter, device_extensions, device_extension_count);
            // Select Queue Indices
            vulkan_select_queue_indices(VkAdapter);
            // Record Adapter Detail
            vulkan_record_adapter_detail(VkAdapter);
        }
    } else {
        atom_assert(0 && "Vulkan: 0 physical device avalable!");
    }
}

// Device APIs
void vulkan_create_pipeline_cache(VulkanDevice* D)
{
    atom_assert((D->pPipelineCache == VK_NULL_HANDLE) && "vulkan_create_pipeline_cache should be called only once!");

    // TODO: serde
    VkPipelineCacheCreateInfo info = {.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
                                      .pNext           = NULL,
                                      .initialDataSize = 0,
                                      .pInitialData    = NULL};
    D->mVkDeviceTable.vkCreatePipelineCache(D->pVkDevice, &info, GLOBAL_VkAllocationCallbacks, &D->pPipelineCache);
}

void vulkan_free_pipeline_cache(VulkanInstance* I, VulkanAdapter* A, VulkanDevice* D)
{
    if (D->pPipelineCache != VK_NULL_HANDLE) {
        D->mVkDeviceTable.vkDestroyPipelineCache(D->pVkDevice, D->pPipelineCache, GLOBAL_VkAllocationCallbacks);
    }
}

// Shader Reflection
static const eAGPUResourceType RTLut[] = {
    AGPU_RESOURCE_TYPE_SAMPLER,                // SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER
    AGPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, // SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    AGPU_RESOURCE_TYPE_TEXTURE,                // SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    AGPU_RESOURCE_TYPE_RW_TEXTURE,             // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE
    AGPU_RESOURCE_TYPE_TEXEL_BUFFER,           // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    AGPU_RESOURCE_TYPE_RW_TEXEL_BUFFER,        // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    AGPU_RESOURCE_TYPE_UNIFORM_BUFFER,         // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    AGPU_RESOURCE_TYPE_RW_BUFFER,              // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER
    AGPU_RESOURCE_TYPE_UNIFORM_BUFFER,         // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    AGPU_RESOURCE_TYPE_RW_BUFFER,              // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
    AGPU_RESOURCE_TYPE_INPUT_ATTACHMENT,       // SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
    AGPU_RESOURCE_TYPE_RAY_TRACING             // SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
};
static eAGPUTextureDimension DIMLut[SpvDimSubpassData + 1] = {
    AGPU_TEX_DIMENSION_1D,                     // SpvDim1D
    AGPU_TEX_DIMENSION_2D,                     // SpvDim2D
    AGPU_TEX_DIMENSION_3D,                     // SpvDim3D
    AGPU_TEX_DIMENSION_CUBE,                   // SpvDimCube
    AGPU_TEX_DIMENSION_UNDEFINED,              // SpvDimRect
    AGPU_TEX_DIMENSION_UNDEFINED,              // SpvDimBuffer
    AGPU_TEX_DIMENSION_UNDEFINED               // SpvDimSubpassData
};
static eAGPUTextureDimension ArrDIMLut[SpvDimSubpassData + 1] = {
    AGPU_TEX_DIMENSION_1D_ARRAY,               // SpvDim1D
    AGPU_TEX_DIMENSION_2D_ARRAY,               // SpvDim2D
    AGPU_TEX_DIMENSION_UNDEFINED,              // SpvDim3D
    AGPU_TEX_DIMENSION_CUBE_ARRAY,             // SpvDimCube
    AGPU_TEX_DIMENSION_UNDEFINED,              // SpvDimRect
    AGPU_TEX_DIMENSION_UNDEFINED,              // SpvDimBuffer
    AGPU_TEX_DIMENSION_UNDEFINED               // SpvDimSubpassData
};
const char8_t* push_constants_name = "push_constants";

void vulkan_initialize_shader_reflection(AGPUDeviceIter                            device,
                                         VulkanShaderLibrary*                      S,
                                         const struct AGPUShaderLibraryDescriptor* desc)
{
    S->pReflect             = (SpvReflectShaderModule*)atom_calloc(1, sizeof(SpvReflectShaderModule));
    SpvReflectResult spvRes = spvReflectCreateShaderModule(desc->code_size, desc->code, S->pReflect);
    (void)spvRes;
    atom_assert(spvRes == SPV_REFLECT_RESULT_SUCCESS && "Failed to Reflect Shader!");
    uint32_t entry_count       = S->pReflect->entry_point_count;
    S->super.entrys_count      = entry_count;
    S->super.entry_reflections = atom_calloc(entry_count, sizeof(AGPUShaderReflection));
    for (uint32_t i = 0; i < entry_count; i++) {
        // Initialize Common Reflection Data
        AGPUShaderReflection*       reflection = &S->super.entry_reflections[i];
        // ATTENTION: We have only one entry point now
        const SpvReflectEntryPoint* entry      = spvReflectGetEntryPoint(S->pReflect, S->pReflect->entry_points[i].name);
        reflection->entry_name                 = (const char8_t*)entry->name;
        reflection->stage                      = (eAGPUShaderStage)entry->shader_stage;
        if (reflection->stage == AGPU_SHADER_STAGE_COMPUTE) {
            reflection->thread_group_sizes[0] = entry->local_size.x;
            reflection->thread_group_sizes[1] = entry->local_size.y;
            reflection->thread_group_sizes[2] = entry->local_size.z;
        }
        const bool bGLSL = S->pReflect->source_language & SpvSourceLanguageGLSL;
        (void)bGLSL;
        const bool bHLSL = S->pReflect->source_language & SpvSourceLanguageHLSL;
        uint32_t   icount;
        spvReflectEnumerateInputVariables(S->pReflect, &icount, NULL);
        if (icount > 0) {
            ATOM_DECLARE_ZERO_VLA(SpvReflectInterfaceVariable*, input_vars, icount)
            spvReflectEnumerateInputVariables(S->pReflect, &icount, input_vars);
            if ((entry->shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)) {
                reflection->vertex_inputs_count = icount;
                reflection->vertex_inputs       = atom_calloc(icount, sizeof(AGPUVertexInput));
                // Handle Vertex Inputs
                for (uint32_t i = 0; i < icount; i++) {
                    // We use semantic for HLSL sources because DXC is a piece of shit.
                    reflection->vertex_inputs[i].name   = bHLSL ? input_vars[i]->semantic : input_vars[i]->name;
                    reflection->vertex_inputs[i].format = vulkan_vk_format_to_agpu((VkFormat)input_vars[i]->format);
                }
            }
        }
        // Handle Descriptor Sets
        uint32_t scount;
        uint32_t ccount;
        spvReflectEnumeratePushConstantBlocks(S->pReflect, &ccount, NULL);
        spvReflectEnumerateDescriptorSets(S->pReflect, &scount, NULL);
        if (scount > 0 || ccount > 0) {
            ATOM_DECLARE_ZERO_VLA(SpvReflectDescriptorSet*, descriptros_sets, scount + 1)
            ATOM_DECLARE_ZERO_VLA(SpvReflectBlockVariable*, root_sets, ccount + 1)
            spvReflectEnumerateDescriptorSets(S->pReflect, &scount, descriptros_sets);
            spvReflectEnumeratePushConstantBlocks(S->pReflect, &ccount, root_sets);
            uint32_t bcount = 0;
            for (uint32_t i = 0; i < scount; i++) { bcount += descriptros_sets[i]->binding_count; }
            bcount                             += ccount;
            reflection->shader_resources_count  = bcount;
            reflection->shader_resources        = atom_calloc(bcount, sizeof(AGPUShaderResource));
            // Fill Shader Resources
            uint32_t i_res                      = 0;
            for (uint32_t i_set = 0; i_set < scount; i_set++) {
                SpvReflectDescriptorSet* current_set = descriptros_sets[i_set];
                for (uint32_t i_binding = 0; i_binding < current_set->binding_count; i_binding++, i_res++) {
                    SpvReflectDescriptorBinding* current_binding = current_set->bindings[i_binding];
                    AGPUShaderResource*          current_res     = &reflection->shader_resources[i_res];
                    current_res->set                             = current_binding->set;
                    current_res->binding                         = current_binding->binding;
                    current_res->stages                          = S->pReflect->shader_stage;
                    current_res->type                            = RTLut[current_binding->descriptor_type];
                    current_res->name                            = current_binding->name;
                    current_res->name_hash = agpu_name_hash(current_binding->name, strlen(current_binding->name));
                    current_res->size      = current_binding->count;
                    // Solve Dimension
                    if ((current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE)
                        || (current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE)) {
                        if (current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_ARRAY)
                            current_res->dim = ArrDIMLut[current_binding->image.dim];
                        else
                            current_res->dim = DIMLut[current_binding->image.dim];
                        if (current_binding->image.ms) {
                            current_res->dim =
                                current_res->dim & AGPU_TEX_DIMENSION_2D ? AGPU_TEX_DIMENSION_2DMS : current_res->dim;
                            current_res->dim = current_res->dim & AGPU_TEX_DIMENSION_2D_ARRAY ? AGPU_TEX_DIMENSION_2DMS_ARRAY
                                                                                              : current_res->dim;
                        }
                    }
                }
            }
            // Fill Push Constants
            for (uint32_t i = 0; i < ccount; i++) {
                AGPUShaderResource* current_res = &reflection->shader_resources[i_res + i];
                current_res->set                = 0;
                current_res->type               = AGPU_RESOURCE_TYPE_PUSH_CONSTANT;
                current_res->binding            = 0;
                current_res->name               = push_constants_name;
                current_res->name_hash          = agpu_name_hash(current_res->name, strlen(current_res->name));
                current_res->stages             = S->pReflect->shader_stage;
                current_res->size               = root_sets[i]->size;
                current_res->offset             = root_sets[i]->offset;
            }
        }
    }
}

void vulkan_free_shader_reflection(VulkanShaderLibrary* S)
{
    spvReflectDestroyShaderModule(S->pReflect);
    if (S->super.entry_reflections) {
        for (uint32_t i = 0; i < S->super.entrys_count; i++) {
            AGPUShaderReflection* reflection = S->super.entry_reflections + i;
            if (reflection->vertex_inputs) atom_free(reflection->vertex_inputs);
            if (reflection->shader_resources) atom_free(reflection->shader_resources);
        }
    }
    atom_free(S->super.entry_reflections);
    atom_free(S->pReflect);
}

// VMA
void vulkan_create_vma_allocator(VulkanInstance* I, VulkanAdapter* A, VulkanDevice* D)
{
    VmaVulkanFunctions vulkanFunctions = {
        .vkAllocateMemory                    = D->mVkDeviceTable.vkAllocateMemory,
        .vkBindBufferMemory                  = D->mVkDeviceTable.vkBindBufferMemory,
        .vkBindImageMemory                   = D->mVkDeviceTable.vkBindImageMemory,
        .vkCreateBuffer                      = D->mVkDeviceTable.vkCreateBuffer,
        .vkCreateImage                       = D->mVkDeviceTable.vkCreateImage,
        .vkDestroyBuffer                     = D->mVkDeviceTable.vkDestroyBuffer,
        .vkDestroyImage                      = D->mVkDeviceTable.vkDestroyImage,
        .vkFreeMemory                        = D->mVkDeviceTable.vkFreeMemory,
        .vkGetBufferMemoryRequirements       = D->mVkDeviceTable.vkGetBufferMemoryRequirements,
        .vkGetBufferMemoryRequirements2KHR   = D->mVkDeviceTable.vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements        = D->mVkDeviceTable.vkGetImageMemoryRequirements,
        .vkGetImageMemoryRequirements2KHR    = D->mVkDeviceTable.vkGetImageMemoryRequirements2KHR,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties,
        .vkMapMemory                         = D->mVkDeviceTable.vkMapMemory,
        .vkUnmapMemory                       = D->mVkDeviceTable.vkUnmapMemory,
        .vkFlushMappedMemoryRanges           = D->mVkDeviceTable.vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges      = D->mVkDeviceTable.vkInvalidateMappedMemoryRanges,
        .vkCmdCopyBuffer                     = D->mVkDeviceTable.vkCmdCopyBuffer};
    VmaAllocatorCreateInfo vmaInfo = {.device               = D->pVkDevice,
                                      .physicalDevice       = A->pPhysicalDevice,
                                      .instance             = I->pVkInstance,
                                      .pVulkanFunctions     = &vulkanFunctions,
                                      .pAllocationCallbacks = GLOBAL_VkAllocationCallbacks};
    if (A->dedicated_allocation) { vmaInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT; }
    if (vmaCreateAllocator(&vmaInfo, &D->pVmaAllocator) != VK_SUCCESS) { atom_assert(0 && "Failed to create VMA Allocator"); }
}

void vulkan_free_vma_allocator(VulkanInstance* I, VulkanAdapter* A, VulkanDevice* D) { vmaDestroyAllocator(D->pVmaAllocator); }

// API Objects Helpers
struct VulkanDescriptorPool* vulkan_create_desciptor_pool(VulkanDevice* D)
{
    VulkanDescriptorPool* Pool = (VulkanDescriptorPool*)atom_calloc(1, sizeof(VulkanDescriptorPool));
#ifdef AGPU_THREAD_SAFETY
    Pool->pMutex = (mtx_t*)atom_calloc(1, sizeof(mtx_t));
    mtx_init(Pool->pMutex, mtx_plain);
#endif
    VkDescriptorPoolCreateFlags flags          = (VkDescriptorPoolCreateFlags)0;
    // TODO: It is possible to avoid using that flag by updating descriptor sets instead of deleting them.
    flags                                     |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    Pool->Device                               = D;
    Pool->mFlags                               = flags;
    VkDescriptorPoolCreateInfo poolCreateInfo  = {.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                                  .pNext         = NULL,
                                                  .poolSizeCount = AGPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE,
                                                  .pPoolSizes    = gDescriptorPoolSizes,
                                                  .flags         = Pool->mFlags,
                                                  .maxSets       = 8192};
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateDescriptorPool(D->pVkDevice,
                                                            &poolCreateInfo,
                                                            GLOBAL_VkAllocationCallbacks,
                                                            &Pool->pVkDescPool));
    return Pool;
}

void vulkan_consume_descriptor_sets(struct VulkanDescriptorPool* pPool,
                                    const VkDescriptorSetLayout* pLayouts,
                                    VkDescriptorSet*             pSets,
                                    uint32_t                     numDescriptorSets)
{
#ifdef AGPU_THREAD_SAFETY
    mtx_lock(pPool->pMutex);
#endif
    {
        VulkanDevice*               D          = (VulkanDevice*)pPool->Device;
        VkDescriptorSetAllocateInfo alloc_info = {.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                  .pNext              = NULL,
                                                  .descriptorPool     = pPool->pVkDescPool,
                                                  .descriptorSetCount = numDescriptorSets,
                                                  .pSetLayouts        = pLayouts};
        VkResult                    vk_res     = D->mVkDeviceTable.vkAllocateDescriptorSets(D->pVkDevice, &alloc_info, pSets);
        if (vk_res != VK_SUCCESS) {
            atom_assert(0 && "Descriptor Set used out, vk descriptor pool expansion not implemented!");
        }
    }
#ifdef AGPU_THREAD_SAFETY
    mtx_unlock(pPool->pMutex);
#endif
}

void vulkan_fetch_descriptor_sets(struct VulkanDescriptorPool* pPool, VkDescriptorSet* pSets, uint32_t numDescriptorSets)
{
#ifdef AGPU_THREAD_SAFETY
    mtx_lock(pPool->pMutex);
#endif
    {
        // TODO: It is possible to avoid using that flag by updating descriptor sets instead of deleting them.
        // The application can keep track of recycled descriptor sets and re-use one of them when a new one is requested.
        // Reference:
        // https://arm-software.github.io/vulkan_best_practice_for_mobile_developers/samples/performance/descriptor_management/descriptor_management_tutorial.html
        VulkanDevice* D = (VulkanDevice*)pPool->Device;
        D->mVkDeviceTable.vkFreeDescriptorSets(D->pVkDevice, pPool->pVkDescPool, numDescriptorSets, pSets);
    }
#ifdef AGPU_THREAD_SAFETY
    mtx_unlock(pPool->pMutex);
#endif
}

void vulkan_free_descriptor_pool(struct VulkanDescriptorPool* DescPool)
{
    VulkanDevice* D = DescPool->Device;
    D->mVkDeviceTable.vkDestroyDescriptorPool(D->pVkDevice, DescPool->pVkDescPool, GLOBAL_VkAllocationCallbacks);
#ifdef AGPU_THREAD_SAFETY
    if (DescPool->pMutex) {
        mtx_destroy(DescPool->pMutex);
        atom_free(DescPool->pMutex);
    }
#endif
    atom_free(DescPool);
}

VkDescriptorSetLayout vulkan_create_descriptor_set_layout(VulkanDevice*                       D,
                                                          const VkDescriptorSetLayoutBinding* bindings,
                                                          uint32_t                            bindings_count)
{
    VkDescriptorSetLayout           out_layout  = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo layout_info = {.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                   .pNext        = NULL,
                                                   .bindingCount = bindings_count,
                                                   .pBindings    = bindings,
                                                   .flags        = 0};
    CHECK_VKRESULT(
        D->mVkDeviceTable.vkCreateDescriptorSetLayout(D->pVkDevice, &layout_info, GLOBAL_VkAllocationCallbacks, &out_layout));
    return out_layout;
}

void vulkan_free_descriptor_set_layout(VulkanDevice* D, VkDescriptorSetLayout layout)
{
    D->mVkDeviceTable.vkDestroyDescriptorSetLayout(D->pVkDevice, layout, GLOBAL_VkAllocationCallbacks);
}

// Select Helpers
void vulkan_query_host_visible_vram_info(VulkanAdapter* VkAdapter)
{
    AGPUAdapterDetail* adapter_detail         = &VkAdapter->adapter_detail;
    adapter_detail->support_host_visible_vram = false;
#ifdef VK_EXT_memory_budget
#if VK_EXT_memory_budget
    if (vkGetPhysicalDeviceMemoryProperties2KHR) {
        ATOM_DECLARE_ZERO(VkPhysicalDeviceMemoryBudgetPropertiesEXT, budget)
        budget.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
        ATOM_DECLARE_ZERO(VkPhysicalDeviceMemoryProperties2, mem_prop2)
        mem_prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        mem_prop2.pNext = &budget;
        vkGetPhysicalDeviceMemoryProperties2KHR(VkAdapter->pPhysicalDevice, &mem_prop2);
        VkPhysicalDeviceMemoryProperties mem_prop = mem_prop2.memoryProperties;
        for (uint32_t j = 0; j < mem_prop.memoryTypeCount; j++) {
            const uint32_t heap_index = mem_prop.memoryTypes[j].heapIndex;
            if (mem_prop.memoryHeaps[heap_index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                const bool isDeviceLocal = mem_prop.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                const bool isHostVisible = mem_prop.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                if (isDeviceLocal && isHostVisible) {
                    adapter_detail->support_host_visible_vram = true;
                    adapter_detail->host_visible_vram_budget =
                        budget.heapBudget[heap_index] ? budget.heapBudget[heap_index] : mem_prop.memoryHeaps[heap_index].size;
                    break;
                }
            }
        }
    } else
#endif
#endif
    {
        ATOM_DECLARE_ZERO(VkPhysicalDeviceMemoryProperties, mem_prop)
        vkGetPhysicalDeviceMemoryProperties(VkAdapter->pPhysicalDevice, &mem_prop);
        for (uint32_t j = 0; j < mem_prop.memoryTypeCount; j++) {
            const uint32_t heap_index = mem_prop.memoryTypes[j].heapIndex;
            if (mem_prop.memoryHeaps[heap_index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                const bool isDeviceLocal = mem_prop.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                const bool isHostVisible = mem_prop.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                if (isDeviceLocal && isHostVisible) {
                    adapter_detail->support_host_visible_vram = true;
                    adapter_detail->host_visible_vram_budget  = mem_prop.memoryHeaps[heap_index].size;
                    break;
                }
            }
        }
    }
}

void vulkan_query_dynamic_pipeline_states(VulkanAdapter* VkAdapter, uint32_t* pCount, VkDynamicState* pStates)
{
    VkDynamicState base_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_SCISSOR,
                                    VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                                    VK_DYNAMIC_STATE_DEPTH_BOUNDS,
                                    VK_DYNAMIC_STATE_STENCIL_REFERENCE};

    uint32_t base_states_count  = sizeof(base_states) / sizeof(VkDynamicState);
    uint32_t total_states_count = base_states_count;
    if (pStates) { memcpy(pStates, base_states, sizeof(base_states)); }
    if (VkAdapter->adapter_detail.support_shading_rate) {
        if (pStates) pStates[total_states_count] = VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR;
        total_states_count += 1;
    }
    if (pCount) { *pCount = total_states_count; }
}

void vulkan_record_adapter_detail(VulkanAdapter* VkAdapter)
{
    AGPUAdapterDetail*          adapter_detail = &VkAdapter->adapter_detail;
    VkPhysicalDeviceProperties* prop           = &VkAdapter->mPhysicalDeviceProps.properties;
    adapter_detail->is_cpu                     = prop->deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
    adapter_detail->is_virtual                 = prop->deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
    adapter_detail->is_uma                     = prop->deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    atom_assert(prop->deviceType != VK_PHYSICAL_DEVICE_TYPE_OTHER && "VK_PHYSICAL_DEVICE_TYPE_OTHER not supported!");

    // vendor info
    adapter_detail->vendor_preset.device_id = prop->deviceID;
    adapter_detail->vendor_preset.vendor_id = prop->vendorID;
    if (adapter_detail->vendor_preset.vendor_id == 0x10DE) // NVIDIA
    {
        const uint32_t vraw = prop->driverVersion;
        const uint32_t v0   = (vraw >> 22) & 0x3ff;
        const uint32_t v1   = (vraw >> 14) & 0x0ff;
        const uint32_t v2   = (vraw >> 6) & 0x0ff;
        const uint32_t v3   = (vraw) & 0x03f;
        adapter_detail->vendor_preset.driver_version =
            vulkan_combine_version(vulkan_combine_version(vulkan_combine_version(v0, v1), v2), v3);
    } else if (adapter_detail->vendor_preset.vendor_id == 0x8086) // Intel
    {
        const uint32_t vraw                          = prop->driverVersion;
        const uint32_t v0                            = (vraw >> 14);
        const uint32_t v1                            = (vraw) & 0x3fff;
        adapter_detail->vendor_preset.driver_version = vulkan_combine_version(v0, v1);
    } else {
        adapter_detail->vendor_preset.driver_version = prop->driverVersion;
    }
    const char* device_name = prop->deviceName;
    memcpy(adapter_detail->vendor_preset.gpu_name, device_name, strlen(device_name));

    // some features
    adapter_detail->uniform_buffer_alignment            = (uint32_t)prop->limits.minUniformBufferOffsetAlignment;
    adapter_detail->upload_buffer_texture_alignment     = (uint32_t)prop->limits.optimalBufferCopyOffsetAlignment;
    adapter_detail->upload_buffer_texture_row_alignment = (uint32_t)prop->limits.optimalBufferCopyRowPitchAlignment;
    adapter_detail->max_vertex_input_bindings           = prop->limits.maxVertexInputBindings;
    adapter_detail->multidraw_indirect                  = prop->limits.maxDrawIndirectCount > 1;
    adapter_detail->wave_lane_count                     = VkAdapter->mSubgroupProperties.subgroupSize;
    adapter_detail->support_geom_shader                 = VkAdapter->mPhysicalDeviceFeatures.features.geometryShader;
    adapter_detail->support_tessellation                = VkAdapter->mPhysicalDeviceFeatures.features.tessellationShader;
#if VK_EXT_extended_dynamic_state
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicStateFeatures.extendedDynamicState ? AGPU_DYNAMIC_STATE_Tier1 : 0;
#endif
#if VK_EXT_extended_dynamic_state2
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState2Features.extendedDynamicState2 ? AGPU_DYNAMIC_STATE_RasterDiscard : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState2Features.extendedDynamicState2 ? AGPU_DYNAMIC_STATE_DepthBias : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState2Features.extendedDynamicState2 ? AGPU_DYNAMIC_STATE_PrimitiveRestart : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState2Features.extendedDynamicState2LogicOp ? AGPU_DYNAMIC_STATE_LogicOp : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState2Features.extendedDynamicState2PatchControlPoints
            ? AGPU_DYNAMIC_STATE_PatchControlPoints
            : 0;
#endif
#if VK_EXT_extended_dynamic_state3
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3TessellationDomainOrigin
            ? AGPU_DYNAMIC_STATE_TessellationDomainOrigin
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3DepthClampEnable
            ? AGPU_DYNAMIC_STATE_DepthClampEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3PolygonMode
            ? AGPU_DYNAMIC_STATE_PolygonMode
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3RasterizationSamples
            ? AGPU_DYNAMIC_STATE_SampleCount
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3SampleMask ? AGPU_DYNAMIC_STATE_SampleMask
                                                                                                : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3AlphaToCoverageEnable
            ? AGPU_DYNAMIC_STATE_AlphaToCoverageEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3AlphaToOneEnable
            ? AGPU_DYNAMIC_STATE_AlphaToOneEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3LogicOpEnable
            ? AGPU_DYNAMIC_STATE_LogicOpEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ColorBlendEnable
            ? AGPU_DYNAMIC_STATE_ColorBlendEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ColorBlendEquation
            ? AGPU_DYNAMIC_STATE_ColorBlendEquation
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ColorWriteMask
            ? AGPU_DYNAMIC_STATE_ColorWriteMask
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3RasterizationStream
            ? AGPU_DYNAMIC_STATE_RasterStream
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ConservativeRasterizationMode
            ? AGPU_DYNAMIC_STATE_ConservativeRasterMode
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ExtraPrimitiveOverestimationSize
            ? AGPU_DYNAMIC_STATE_ExtraPrimitiveOverestimationSize
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3DepthClipEnable
            ? AGPU_DYNAMIC_STATE_DepthClipEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3SampleLocationsEnable
            ? AGPU_DYNAMIC_STATE_SampleLocationsEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ColorBlendAdvanced
            ? AGPU_DYNAMIC_STATE_ColorBlendAdvanced
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ProvokingVertexMode
            ? AGPU_DYNAMIC_STATE_ProvokingVertexMode
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3LineRasterizationMode
            ? AGPU_DYNAMIC_STATE_LineRasterizationMode
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3LineStippleEnable
            ? AGPU_DYNAMIC_STATE_LineStippleEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3DepthClipNegativeOneToOne
            ? AGPU_DYNAMIC_STATE_DepthClipNegativeOneToOne
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ViewportWScalingEnable
            ? AGPU_DYNAMIC_STATE_ViewportWScalingEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ViewportSwizzle
            ? AGPU_DYNAMIC_STATE_ViewportSwizzle
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3CoverageToColorEnable
            ? AGPU_DYNAMIC_STATE_CoverageToColorEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3CoverageToColorLocation
            ? AGPU_DYNAMIC_STATE_CoverageToColorLocation
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3CoverageModulationMode
            ? AGPU_DYNAMIC_STATE_CoverageModulationMode
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3CoverageModulationTableEnable
            ? AGPU_DYNAMIC_STATE_CoverageModulationTableEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3CoverageModulationTable
            ? AGPU_DYNAMIC_STATE_CoverageModulationTable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3CoverageModulationMode
            ? AGPU_DYNAMIC_STATE_CoverageReductionMode
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3RepresentativeFragmentTestEnable
            ? AGPU_DYNAMIC_STATE_RepresentativeFragmentTestEnable
            : 0;
    adapter_detail->dynamic_state_features |=
        VkAdapter->mPhysicalDeviceExtendedDynamicState3Features.extendedDynamicState3ShadingRateImageEnable
            ? AGPU_DYNAMIC_STATE_ShadingRateImageEnable
            : 0;
#endif
    // memory features
    vulkan_query_host_visible_vram_info(VkAdapter);
}

void vulkan_select_queue_indices(VulkanAdapter* VkAdapter)
{
    // Query Queue Information.
    vkGetPhysicalDeviceQueueFamilyProperties(VkAdapter->pPhysicalDevice, &VkAdapter->mQueueFamiliesCount, ATOM_NULLPTR);
    VkAdapter->pQueueFamilyProperties =
        (VkQueueFamilyProperties*)atom_calloc(VkAdapter->mQueueFamiliesCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(VkAdapter->pPhysicalDevice,
                                             &VkAdapter->mQueueFamiliesCount,
                                             VkAdapter->pQueueFamilyProperties);

    for (uint32_t j = 0; j < VkAdapter->mQueueFamiliesCount; j++) {
        const VkQueueFamilyProperties* prop = &VkAdapter->pQueueFamilyProperties[j];
        if ((VkAdapter->mQueueFamilyIndices[AGPU_QUEUE_TYPE_GRAPHICS] == -1) && (prop->queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            VkAdapter->mQueueFamilyIndices[AGPU_QUEUE_TYPE_GRAPHICS] = j;
        } else if ((VkAdapter->mQueueFamilyIndices[AGPU_QUEUE_TYPE_COMPUTE] == -1)
                   && (prop->queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            VkAdapter->mQueueFamilyIndices[AGPU_QUEUE_TYPE_COMPUTE] = j;
        } else if ((VkAdapter->mQueueFamilyIndices[AGPU_QUEUE_TYPE_TRANSFER] == -1)
                   && (prop->queueFlags & VK_QUEUE_TRANSFER_BIT)) {
            VkAdapter->mQueueFamilyIndices[AGPU_QUEUE_TYPE_TRANSFER] = j;
        } else if ((VkAdapter->mQueueFamilyIndices[AGPU_QUEUE_TYPE_TILE_MAPPING] == -1)
                   && (prop->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
            VkAdapter->mQueueFamilyIndices[AGPU_QUEUE_TYPE_TILE_MAPPING] = j;
        }
    }
}

void vulkan_enumerate_format_supports(VulkanAdapter* VkAdapter)
{
    AGPUAdapterDetail* adapter_detail = (AGPUAdapterDetail*)&VkAdapter->adapter_detail;
    for (uint32_t i = 0; i < AGPU_FORMAT_COUNT; ++i) {
        VkFormatProperties formatSupport;
        adapter_detail->format_supports[i].shader_read         = 0;
        adapter_detail->format_supports[i].shader_write        = 0;
        adapter_detail->format_supports[i].render_target_write = 0;
        VkFormat   fmt                                         = (VkFormat)vulkan_agpu_format_to_vk((eAGPUFormat)i);
        const bool bPVRTC = (fmt >= VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG && fmt <= VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG);
        if (fmt == VK_FORMAT_UNDEFINED) continue;
        if (bPVRTC) continue; // only iOS supports it and we dont use vulkan on iOS devices

        vkGetPhysicalDeviceFormatProperties(VkAdapter->pPhysicalDevice, fmt, &formatSupport);
        adapter_detail->format_supports[i].shader_read =
            (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
        adapter_detail->format_supports[i].shader_write =
            (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;
        adapter_detail->format_supports[i].render_target_write =
            (formatSupport.optimalTilingFeatures
             & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
            != 0;
    }
    return;
}

void vulkan_select_instance_layers(struct VulkanInstance* vkInstance,
                                   const char* const*     instance_layers,
                                   uint32_t               instance_layers_count)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, NULL);
    if (count != 0) {
        vkInstance->pLayerNames      = atom_calloc(instance_layers_count, sizeof(const char*));
        vkInstance->pLayerProperties = atom_calloc(instance_layers_count, sizeof(VkLayerProperties));

        ATOM_DECLARE_ZERO_VLA(VkLayerProperties, layer_props, count)
        vkEnumerateInstanceLayerProperties(&count, layer_props);
        uint32_t filled_exts = 0;
        for (uint32_t j = 0; j < instance_layers_count; j++) {
            for (uint32_t i = 0; i < count; i++) {
                if (strcmp(layer_props[i].layerName, instance_layers[j]) == 0) {
                    VkLayerProperties* pLayer                 = &layer_props[i];
                    vkInstance->pLayerProperties[filled_exts] = *pLayer;
                    vkInstance->pLayerNames[filled_exts]      = vkInstance->pLayerProperties[filled_exts].layerName;
                    filled_exts++;
                    break;
                }
            }
        }
        vkInstance->mLayersCount = filled_exts;
    }
    return;
}

void vulkan_select_instance_extensions(struct VulkanInstance* VkInstance,
                                       const char* const*     instance_extensions,
                                       uint32_t               instance_extension_count)
{
    const char* layer_name = NULL; // Query Vulkan implementation or by implicitly enabled layers
    uint32_t    count      = 0;
    vkEnumerateInstanceExtensionProperties(layer_name, &count, NULL);
    if (count > 0) {
        VkInstance->pExtensionProperties = atom_calloc(instance_extension_count, sizeof(VkExtensionProperties));
        VkInstance->pExtensionNames      = atom_calloc(instance_extension_count, sizeof(const char*));

        ATOM_DECLARE_ZERO_VLA(VkExtensionProperties, ext_props, count)
        vkEnumerateInstanceExtensionProperties(layer_name, &count, ext_props);
        uint32_t filled_exts = 0;
        for (uint32_t j = 0; j < instance_extension_count; j++) {
            for (uint32_t i = 0; i < count; i++) {
                VkExtensionProperties ext_prop = ext_props[i];
                if (strcmp(ext_prop.extensionName, instance_extensions[j]) == 0) {
                    VkInstance->pExtensionProperties[filled_exts] = ext_prop;
                    VkInstance->pExtensionNames[filled_exts]      = VkInstance->pExtensionProperties[filled_exts].extensionName;
                    filled_exts++;
                    break;
                }
            }
        }
        VkInstance->mExtensionsCount = filled_exts;
    }
    return;
}

void vulkan_select_physical_device_layers(struct VulkanAdapter* VkAdapter,
                                          const char* const*    device_layers,
                                          uint32_t              device_layers_count)
{
    uint32_t count;
    vkEnumerateDeviceLayerProperties(VkAdapter->pPhysicalDevice, &count, NULL);
    if (count != 0) {
        VkAdapter->pLayerNames      = atom_calloc(device_layers_count, sizeof(const char*));
        VkAdapter->pLayerProperties = atom_calloc(device_layers_count, sizeof(VkLayerProperties));

        ATOM_DECLARE_ZERO_VLA(VkLayerProperties, layer_props, count)
        vkEnumerateDeviceLayerProperties(VkAdapter->pPhysicalDevice, &count, layer_props);
        uint32_t filled_exts = 0;
        for (uint32_t j = 0; j < device_layers_count; j++) {
            for (uint32_t i = 0; i < count; i++) {
                if (strcmp(layer_props[i].layerName, device_layers[j]) == 0) {
                    VkAdapter->pLayerProperties[filled_exts] = layer_props[i];
                    VkAdapter->pLayerNames[filled_exts]      = VkAdapter->pLayerProperties[filled_exts].layerName;
                    filled_exts++;
                    break;
                }
            }
        }
        VkAdapter->mLayersCount = filled_exts;
    }
    return;
}

void vulkan_select_physical_device_extensions(struct VulkanAdapter* VkAdapter,
                                              const char* const*    device_extensions,
                                              uint32_t              device_extension_count)
{
    const char* layer_name = NULL; // Query Vulkan implementation or by implicitly enabled layers
    uint32_t    count      = 0;
    vkEnumerateDeviceExtensionProperties(VkAdapter->pPhysicalDevice, layer_name, &count, NULL);
    if (count > 0) {
        VkAdapter->pExtensionProperties = atom_calloc(device_extension_count, sizeof(VkExtensionProperties));
        VkAdapter->pExtensionNames      = atom_calloc(device_extension_count, sizeof(const char*));

        ATOM_DECLARE_ZERO_VLA(VkExtensionProperties, ext_props, count)
        vkEnumerateDeviceExtensionProperties(VkAdapter->pPhysicalDevice, layer_name, &count, ext_props);
        uint32_t filled_exts = 0;
        for (uint32_t j = 0; j < device_extension_count; j++) {
            for (uint32_t i = 0; i < count; i++) {
                if (strcmp(ext_props[i].extensionName, device_extensions[j]) == 0) {
                    VkAdapter->pExtensionProperties[filled_exts] = ext_props[i];
                    const char* enabledName                      = VkAdapter->pExtensionProperties[filled_exts].extensionName;
                    VkAdapter->pExtensionNames[filled_exts]      = enabledName;
                    filled_exts++;
                    continue;
                }
            }
        }
        VkAdapter->mExtensionsCount = filled_exts;
    }
    return;
}

// Debug Callback
static ATOM_FORCEINLINE void vulkan_debug_utils_set_object_name(VkDevice     pDevice,
                                                                uint64_t     handle,
                                                                VkObjectType type,
                                                                const char*  pName)
{
    VkDebugUtilsObjectNameInfoEXT nameInfo = {.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                                              .objectType   = type,
                                              .objectHandle = handle,
                                              .pObjectName  = pName};
    vkSetDebugUtilsObjectNameEXT(pDevice, &nameInfo);
}

static ATOM_FORCEINLINE void vulkan_debug_report_set_object_name(VkDevice                   pDevice,
                                                                 uint64_t                   handle,
                                                                 VkDebugReportObjectTypeEXT type,
                                                                 const char*                pName)
{
    VkDebugMarkerObjectNameInfoEXT nameInfo = {.sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
                                               .objectType  = type,
                                               .object      = (uint64_t)handle,
                                               .pObjectName = pName};
    vkDebugMarkerSetObjectNameEXT(pDevice, &nameInfo);
}

void vulkan_optional_set_object_name(struct VulkanDevice* device, uint64_t handle, VkObjectType type, const char* name)
{
    VulkanInstance* I = (VulkanInstance*)device->super.adapter->instance;
    if (I->super.enable_set_name && name) {
        if (I->debug_report) {
            VkDebugReportObjectTypeEXT exttype = vulkan_object_type_to_debug_report_type(type);
            vulkan_debug_report_set_object_name(device->pVkDevice, handle, exttype, name);
        }
        if (I->debug_utils) { vulkan_debug_utils_set_object_name(device->pVkDevice, handle, type, name); }
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           void*                                       pUserData)
{
    if (vulkan_try_ignore_message(pCallbackData->pMessageIdName, false)) return VK_FALSE;

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            ATOM_trace("Vulkan validation layer: %s\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            ATOM_info("Vulkan validation layer: %s\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            ATOM_warn("Vulkan validation layer: %s\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            ATOM_error("Vulkan validation layer: %s\n", pCallbackData->pMessage);
            break;
        default: return VK_TRUE;
    }
    return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_report_callback(VkDebugReportFlagsEXT      flags,
                                                            VkDebugReportObjectTypeEXT objectType,
                                                            uint64_t                   object,
                                                            size_t                     location,
                                                            int32_t                    messageCode,
                                                            const char*                pLayerPrefix,
                                                            const char*                pMessage,
                                                            void*                      pUserData)
{
    if (vulkan_try_ignore_message(pMessage, true)) return VK_FALSE;

    switch (flags) {
        case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:         ATOM_info("Vulkan validation layer: %s\n", pMessage);
        case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT: ATOM_warn("Vulkan validation layer: %s\n", pMessage); break;
        case VK_DEBUG_REPORT_WARNING_BIT_EXT:             ATOM_warn("Vulkan validation layer: %s\n", pMessage); break;
        case VK_DEBUG_REPORT_DEBUG_BIT_EXT:               ATOM_debug("Vulkan validation layer: %s\n", pMessage); break;
        case VK_DEBUG_REPORT_ERROR_BIT_EXT:               ATOM_error("Vulkan validation layer: %s\n", pMessage); break;
        default:                                          return VK_TRUE;
    }
    return VK_FALSE;
}
