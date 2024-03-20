#include <iostream>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <vector>
#include <string_view>
#include <algorithm>
#include <array>
#include <mutex>
#include <optional>

static constexpr int INITIAL_WIDTH = 800;
static constexpr int INITIAL_HEIGHT = 600;

constexpr bool is_debug() {
#if NDEBUG
	return false;
#else
	return true;
#endif
}

void load_vulkan_function(VkInstance instance, const char *name, PFN_vkVoidFunction *function) {
	static std::mutex mutex;
	if (nullptr == *function) {
		// TODO(chris): check that locking actually requires core sync. I don't see
		// how it could work otherwise but we're assuming it's the case with the
		// second check below.
		if (mutex.try_lock()) {
			std::lock_guard lock(mutex, std::adopt_lock);
			if (nullptr == *function) {
				*function = vkGetInstanceProcAddr(instance, name);
			}
		}
	}
	return;
}

static VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT &createInfo, const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT &debugMessenger) {
	static VkResult(*createDebugUtilsMessengerExt)(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT*);
	// TODO(chris): I guess we should probably load this at the beginning and
	// check it was successful. Currently we'll just crash if loading fails.
	load_vulkan_function(instance, "vkCreateDebugUtilsMessengerEXT", (PFN_vkVoidFunction*) &createDebugUtilsMessengerExt);
	return (*createDebugUtilsMessengerExt)(instance, &createInfo, allocator, &debugMessenger);
}

template <size_t N>
static bool check_validation_layer_support(const std::array<const char*, N> &validationLayers) {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	bool ok = true;
	for (auto validationLayer : validationLayers) {
		bool found = false;
		for (auto layer : availableLayers) {
			if (std::string_view(validationLayer) == layer.layerName) {
				found = true;
			}
		}
		if (!found) {
			std::cout << "validation layer " << validationLayer << " not found" << std::endl;
			ok = false;
		}
	}
	return ok;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
	void *userData
) {
	std::cerr << "validation layer: " << callbackData->pMessage << std::endl;
	return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = vulkan_debug;
	createInfo.pUserData = nullptr;
	return createInfo;
}

static void setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT &debugMessenger) {
	if (!is_debug()) {
		return;
	}
	
	const VkDebugUtilsMessengerCreateInfoEXT createInfo = debug_messenger_create_info();
	if (VK_SUCCESS != create_debug_utils_messenger_ext(instance, createInfo, nullptr, debugMessenger)) {
		std::cerr << "failed to setupdebug messenger" << std::endl;
		return;
	}
}

static VkInstance create_instance(const bool enableValidationLayers) {
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	static const std::array<const char*, 1> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	if (enableValidationLayers && !check_validation_layer_support(validationLayers)) {
		std::cout << "required validation layer is missing" << std::endl;
		return VK_NULL_HANDLE;
	}
	uint32_t extensionCount = 0;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	std::vector<const char*> extended_extensions(extensions, extensions + extensionCount);
	extended_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	extensionCount = extended_extensions.size();
	createInfo.enabledExtensionCount = extensionCount;
	createInfo.ppEnabledExtensionNames = extended_extensions.data();
	if (enableValidationLayers) {
		std::cout << "enabling validation layers" << std::endl;
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	
		static const auto debugCreateInfo = debug_messenger_create_info();
		createInfo.pNext = &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}
	
	VkInstance instance = {};
	const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS) {
		std::cout << "vkCreateInstance failed" << std::endl;
		return VK_NULL_HANDLE;
	}
	return instance;
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
};

static QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	for (int i = 0; i < queueFamilyCount; i++) {
		const auto &queueFamily = queueFamilies[i];
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = std::optional(i);
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = std::make_optional(i);
		}
	}
	//auto queueFamily = std::find_if(queueFamilies.cbegin(), queueFamilies.cend(), [](const VkQueueFamilyProperties queueFamily) -> bool {
	//	std::cout << "queue family flags: " << queueFamily.queueFlags << std::endl;
	//	return queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
	//});
	//if (queueFamily != queueFamilies.cend()) {
	//	indices.graphicsFamily = queueFamily - queueFamilies.cbegin();
	//}
	return indices;
}

static bool is_device_suitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	
	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

static VkPhysicalDevice pick_physical_device(VkInstance instance) {
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (0 == deviceCount) {
		return VK_NULL_HANDLE;
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	auto device = std::find_if(devices.cbegin(), devices.cend(), [](const VkPhysicalDevice device) -> bool {
		return is_device_suitable(device);
	});
	if (device == devices.cend()) {
		std::cerr << "no suitable device available" << std::endl;
		return VK_NULL_HANDLE;
	}
	return *device;
}

struct LogicalDevice {
	const QueueFamilyIndices indices;
	VkDevice device;
};

static LogicalDevice create_logical_device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	const QueueFamilyIndices indices = find_queue_families(physicalDevice, surface);
	if (!indices.graphicsFamily.has_value()) {
		std::cerr << "failed to find a suitable queue family" << std::endl;
		return LogicalDevice{indices, VK_NULL_HANDLE};
	}

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	
	float queuePriority = 1.0;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	
	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	
	VkDevice device = {};
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		std::cerr << "failed to create logical device" << std::endl;
		return LogicalDevice{indices, VK_NULL_HANDLE};
	}
	
	return LogicalDevice{indices, device};
}

static VkInstance init_vulkan(VkSurfaceKHR surface) {
	static VkDebugUtilsMessengerEXT debugMessenger = {};
	VkInstance instance = create_instance(is_debug());
	if (instance == VK_NULL_HANDLE) {
		std::cerr << "create_instance failed" << std::endl;
		return instance;
	}
	setup_debug_messenger(instance, debugMessenger);
	const auto physicalDevice = pick_physical_device(instance);
	if (VK_NULL_HANDLE == physicalDevice) {
		// TODO(chris): probably double check the leak of the instance is fine since
		// I assume we always maintain an instance for the lifetime of the
		// application (and therefore the os will release it)
		return VK_NULL_HANDLE;
	}
	LogicalDevice logicalDevice = create_logical_device(physicalDevice, surface);
	if (logicalDevice.device == VK_NULL_HANDLE) {
		std::cerr << "create_logical_device failed" << std::endl;
		return VK_NULL_HANDLE;
	}
	return instance;
}

auto main(int argc, char *argv[]) -> int {
	if (!glfwInit()) {
		// TODO: handle failed initialisation
		std::cerr << "fatal: glfwInit failed" << std::endl;
		return 1;
	}

	// print the list of vulkan layers available
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	std::cout << "available layers:" << std::endl;
	for (const auto &layer : availableLayers) {
		std::cout << layer.layerName << std::endl;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "Test", NULL, NULL);
	if (!window) {
		// TODO: handle failed window or context creation
		std::cerr << "fatal: glfwCreateWindow failed" << std::endl;
		return 1;
	}

	VkSurfaceKHR surface;
	const VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (err) {
		std::cerr << "failed to create window surface" << std::endl;
		return 1;
	}

	init_vulkan(surface);
	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
