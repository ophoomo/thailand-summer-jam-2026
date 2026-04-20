
#include <stdexcept>

#include "renderer/vulkan/vulkan_instance.h"
#include "utils/logger.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
              const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
{
    LOG_CORE_WARN("[Validation] {}", callbackData->pMessage);
    return VK_FALSE;
}

// ============================================================
// Construction / destruction
// ============================================================

VulkanInstance::VulkanInstance(const std::string &title,
                               const std::vector<const char *> &extensions)
{
    if (this->m_enableValidation) {
        VulkanInstance::checkValidationLayerSupport();
    }

    std::vector<const char *> instanceExtensions = extensions;
    if (this->m_enableValidation) {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VulkanInstance::checkExtensionSupport(instanceExtensions);

    uint32_t version = 0;
    vkEnumerateInstanceVersion(&version);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "OxEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = version;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (this->m_enableValidation) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(VulkanInstance::m_validationLayers.size());
        createInfo.ppEnabledLayerNames = VulkanInstance::m_validationLayers.data();
        LOG_CORE_TRACE("[Vulkan] Enabled ValidationLayers");

        debugCreateInfo = VulkanInstance::populateDebugCreateInfo();
        createInfo.pNext = &debugCreateInfo;
    }

    LOG_CORE_TRACE("[Vulkan] Extensions Count {}", instanceExtensions.size());
    for (const auto &ext : instanceExtensions) {
        LOG_CORE_TRACE("[Vulkan] Extension: {}", ext);
    }

    if (vkCreateInstance(&createInfo, nullptr, &this->v_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance!");
    }

    if (this->m_enableValidation) {
        this->setupDebugMessenger(debugCreateInfo);
    }

    LOG_CORE_INFO("[Vulkan] Instance Created");
}

VulkanInstance::~VulkanInstance()
{
    if (this->v_debugMessenger != VK_NULL_HANDLE) {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(this->v_instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func) {
            func(this->v_instance, this->v_debugMessenger, nullptr);
        }
    }
    vkDestroyInstance(this->v_instance, nullptr);
    LOG_CORE_INFO("[Vulkan] Instance Destroyed");
}

// ============================================================
// Private Methods
// ============================================================

void VulkanInstance::checkValidationLayerSupport()
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> availableLayers(count);
    vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

    for (const char *layerName : VulkanInstance::m_validationLayers) {
        bool found = false;
        for (const auto &layer : availableLayers) {
            if (strcmp(layer.layerName, layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("Validation layer not supported!");
        }
    }
}

void VulkanInstance::checkExtensionSupport(const std::vector<const char *> &extensions)
{
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());

    for (const char *required : extensions) {
        bool found = false;
        for (const auto &ext : availableExtensions) {
            if (strcmp(ext.extensionName, required) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error(std::string("Required extension not supported: ") + required);
        }
    }
}

VkDebugUtilsMessengerCreateInfoEXT VulkanInstance::populateDebugCreateInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    return createInfo;
}

void VulkanInstance::setupDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT &info)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(this->v_instance, "vkCreateDebugUtilsMessengerEXT"));
    if (!func || func(this->v_instance, &info, nullptr, &this->v_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
    LOG_CORE_INFO("[Vulkan] Debug Messenger created");
}
