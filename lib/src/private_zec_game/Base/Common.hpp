#pragma once
#include <zec/Common.hpp>

#if defined(ZEC_SYSTEM_GENERIC)
	#define GLFW_INCLUDE_VULKAN
	#include <GLFW/glfw3.h>
	#include <vulkan/vulkan.h>

	#if GLFW_VERSION_MAJOR > 3
		#define GLFW_SUPPORT_JOYSTICK_HAT true
	#elif GLFW_VERSION_MAJOR == 3
		#if GLFW_VERSION_MINOR >= 3
			#define GLFW_SUPPORT_JOYSTICK_HAT true
		#endif
	#endif
#endif
