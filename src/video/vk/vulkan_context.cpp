//  SuperTux
//  Copyright (C) 2022 mrkubax10 <mrkubax10@onet.pl>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "video/vk/vulkan_context.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <algorithm>
#include <cstdint>
#include <boost/algorithm/clamp.hpp>

#include "video/vk/vulkan_video_system.hpp"
#include "util/log.hpp"

namespace{
std::vector<const char*> required_extensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
}

const unsigned int VulkanContext::SWAPCHAIN_SIZE = 1;

VulkanContext::VulkanContext(VulkanVideoSystem& video_system) :
  m_video_system(video_system),
  m_vk_instance(VK_NULL_HANDLE),
  m_physical_device(VK_NULL_HANDLE),
  m_device(VK_NULL_HANDLE),
  m_graphics_queue(VK_NULL_HANDLE),
  m_presentation_queue(VK_NULL_HANDLE),
  m_main_surface(VK_NULL_HANDLE),
  m_swapchain(VK_NULL_HANDLE),
  m_swap_format(),
  m_swap_extent(),
  m_swapchain_images(),
  m_swapchain_image_views(),
  m_render_pass(VK_NULL_HANDLE),
  m_pipeline_layout(VK_NULL_HANDLE),
  m_gfx_pipeline(VK_NULL_HANDLE),
  m_swapchain_framebuffers(),
  m_command_pool(VK_NULL_HANDLE),
  m_command_buffers(),
  m_vert_shader(),
  m_frag_shader()
{
  init_application();
  create_main_surface();
  select_device();
  create_logical_device();
  create_swapchain();
  create_image_views();
  create_render_pass();
  create_pipeline();
  create_framebuffer();
  create_command_pool();
  create_command_buffers();
}

VulkanContext::~VulkanContext()
{
  vkDeviceWaitIdle(m_device);
  vkDestroyDevice(m_device, nullptr);
  vkDestroySurfaceKHR(m_vk_instance, m_main_surface, nullptr);
  vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
  for (unsigned int i = 0; i < m_swapchain_framebuffers.size(); i++)
  {
    vkDestroyFramebuffer(m_device, m_swapchain_framebuffers[i], nullptr);
  }
  for (unsigned int i = 0; i < m_swapchain_image_views.size(); i++)
  {
    vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
  }
  vkDestroyPipeline(m_device, m_gfx_pipeline, nullptr);
  vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
  vkDestroyRenderPass(m_device, m_render_pass, nullptr);
  vkDestroyCommandPool(m_device, m_command_pool, nullptr);
  vkDestroyInstance(m_vk_instance, nullptr);
}

void
VulkanContext::init_application()
{
  VkApplicationInfo application{};
  application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application.pNext = nullptr;
  application.pApplicationName = "supertux2";
  application.applicationVersion = VK_MAKE_VERSION(0, 6, 3);
  application.pEngineName = "supertux2";
  application.engineVersion = VK_MAKE_VERSION(0, 6, 3);
  application.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &application;
  create_info.enabledLayerCount = 0;
  unsigned int extensions_count = 0;
  SDL_Vulkan_GetInstanceExtensions(m_video_system.get_window(), &extensions_count, nullptr);
  const char** extensions = new const char*[extensions_count];
  SDL_Vulkan_GetInstanceExtensions(m_video_system.get_window(), &extensions_count, extensions);
  create_info.enabledExtensionCount = extensions_count;
  create_info.ppEnabledExtensionNames = extensions;

  VkResult result = vkCreateInstance(&create_info, nullptr, &m_vk_instance);
  if (result != VK_SUCCESS)
    throw std::runtime_error("failed to create vk renderer!");

  delete[] extensions;
}

void
VulkanContext::select_device()
{
  unsigned int device_count = 0;
  vkEnumeratePhysicalDevices(m_vk_instance, &device_count, nullptr);
  if (device_count == 0)
    throw std::runtime_error("failed to find GPU with Vulkan support");
  VkPhysicalDevice devices[device_count];
  vkEnumeratePhysicalDevices(m_vk_instance, &device_count, devices);
  for (unsigned int i = 0; i < device_count; i++)
  {
    if (is_device_suitable(devices[i]))
    {
      m_physical_device = devices[i];
      log_info << "found suitable graphics card with ID " << i << std::endl;
      break;
    }
  }
  if (m_physical_device == VK_NULL_HANDLE)
    throw std::runtime_error("none of GPU with Vulkan support is suitable");
}

void
VulkanContext::create_logical_device()
{
  QueueFamily family = find_queue_families(m_physical_device);
  float queue_priority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  unsigned int families[] = {family.graphics.value(), family.presentation.value()};
  for (unsigned int i = 0; i < sizeof(families); i++)
  {
    VkDeviceQueueCreateInfo device_queue_create_info{};
    device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_create_info.queueFamilyIndex = families[i];
    device_queue_create_info.queueCount = 1;
    device_queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(device_queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features{};

  VkDeviceCreateInfo device_create_info{};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.pQueueCreateInfos = queue_create_infos.data();
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pEnabledFeatures = &device_features;
  device_create_info.enabledExtensionCount = required_extensions.size();
  device_create_info.ppEnabledExtensionNames = required_extensions.data();
  device_create_info.enabledLayerCount = 0;

  VkResult result = vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device);
  if (result != VK_SUCCESS)
    throw std::runtime_error("failed to create logical device");

  vkGetDeviceQueue(m_device, family.graphics.value(), 0, &m_graphics_queue);
  vkGetDeviceQueue(m_device, family.presentation.value(), 0, &m_presentation_queue);
}

void
VulkanContext::create_main_surface()
{
  SDL_Vulkan_CreateSurface(m_video_system.get_window(), m_vk_instance, &m_main_surface);
}

void
VulkanContext::create_swapchain()
{
  SwapchainDetails details = find_swapchain_features(m_physical_device);
  VkSurfaceFormatKHR surface_format = choose_swapchain_format(details);
  VkPresentModeKHR present_mode = choose_swapchain_present_mode(details);
  m_swap_extent = choose_swap_extent(details);
  m_swap_format = surface_format.format;

  unsigned int image_count = details.capabilities.minImageCount + SWAPCHAIN_SIZE;
  if (image_count > details.capabilities.maxImageCount)
    image_count = details.capabilities.minImageCount;

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = m_main_surface;
  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = m_swap_extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  QueueFamily family = find_queue_families(m_physical_device);
  unsigned int families[] = { family.graphics.value(), family.presentation.value() };
  if (family.graphics != family.presentation)
  {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = families;
  }
  else
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  create_info.preTransform = details.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = VK_NULL_HANDLE;

  VkResult result = vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain);
  if (result != VK_SUCCESS)
    throw std::runtime_error("failed to create swapchain");

  unsigned int swap_image_count = 0;
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &swap_image_count, nullptr);
  m_swapchain_images.resize(swap_image_count);
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &swap_image_count, m_swapchain_images.data());
}

void
VulkanContext::create_image_views()
{
  m_swapchain_image_views.resize(m_swapchain_images.size());
  for (unsigned int i = 0; i < m_swapchain_images.size(); i++)
  {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = m_swapchain_images[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = m_swap_format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(m_device, &create_info, nullptr, &m_swapchain_image_views[i]);
    if (result != VK_SUCCESS)
      throw std::runtime_error("failed to create image view");
  }
}

void
VulkanContext::create_render_pass()
{
  VkAttachmentDescription attachment{};
  attachment.format = m_swap_format;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference attachment_reference{};
  attachment_reference.attachment = 0; // WARNING!!
  attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDependency subpass_dep{};
  subpass_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dep.dstSubpass = 0;
  subpass_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dep.srcAccessMask = 0;
  subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &attachment_reference;

  VkRenderPassCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &attachment;
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass;
  create_info.dependencyCount = 1;
  create_info.pDependencies = &subpass_dep;

  VkResult result = vkCreateRenderPass(m_device, &create_info, nullptr, &m_render_pass);
  if (result != VK_SUCCESS)
    throw new std::runtime_error("failed to create render pass");
}

void
VulkanContext::create_pipeline()
{
  m_vert_shader = VulkanShader::from_file(*this, "shader/vk/shader_vert.spv");
  m_frag_shader = VulkanShader::from_file(*this, "shader/vk/shader_frag.spv");

  VkPipelineShaderStageCreateInfo vert_create_info{};
  vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_create_info.module = m_vert_shader->get_handle();
  vert_create_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_create_info{};
  frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_create_info.module = m_frag_shader->get_handle();
  frag_create_info.pName = "main";

  VkPipelineShaderStageCreateInfo stages[] = { vert_create_info, frag_create_info };

  VkPipelineVertexInputStateCreateInfo input_create_info{};
  input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  input_create_info.vertexBindingDescriptionCount = 0;
  input_create_info.vertexAttributeDescriptionCount = 0;

  VkPipelineInputAssemblyStateCreateInfo assembly_create_info{};
  assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  assembly_create_info.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)m_swap_extent.width;
  viewport.height = (float)m_swap_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = m_swap_extent;

  VkPipelineViewportStateCreateInfo viewport_state_create_info{};
  viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state_create_info.viewportCount = 1;
  viewport_state_create_info.scissorCount = 1;
  viewport_state_create_info.pViewports = &viewport;
  viewport_state_create_info.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
  rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer_create_info.depthClampEnable = VK_FALSE;
  rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
  rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer_create_info.lineWidth = 1.0f;
  rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer_create_info.depthBiasEnable = VK_FALSE;
  rasterizer_create_info.depthBiasConstantFactor = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisample_create_info{};
  multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_create_info.sampleShadingEnable = VK_FALSE;
  multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo color_blend_create_info{};
  color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_create_info.logicOpEnable = VK_FALSE;
  color_blend_create_info.attachmentCount = 1;
  color_blend_create_info.pAttachments = &color_blend_attachment;

  VkPipelineLayoutCreateInfo layout_create_info{};
  layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  
  VkResult result = vkCreatePipelineLayout(m_device, &layout_create_info, nullptr, &m_pipeline_layout);
  if (result != VK_SUCCESS)
    throw std::runtime_error("failed to create gfx pipeline layout");
  
  VkGraphicsPipelineCreateInfo gfx_pipeline_create_info{};
  gfx_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  gfx_pipeline_create_info.stageCount = 2;
  gfx_pipeline_create_info.pStages = stages;
  gfx_pipeline_create_info.pVertexInputState = &input_create_info;
  gfx_pipeline_create_info.pInputAssemblyState = &assembly_create_info;
  gfx_pipeline_create_info.pViewportState = &viewport_state_create_info;
  gfx_pipeline_create_info.pRasterizationState = &rasterizer_create_info;
  gfx_pipeline_create_info.pMultisampleState = &multisample_create_info;
  gfx_pipeline_create_info.pColorBlendState = &color_blend_create_info;
  gfx_pipeline_create_info.layout = m_pipeline_layout;
  gfx_pipeline_create_info.renderPass = m_render_pass;
  gfx_pipeline_create_info.subpass = 0;

  VkResult result2 = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &gfx_pipeline_create_info, nullptr, &m_gfx_pipeline);
  if (result2 != VK_SUCCESS)
    throw new std::runtime_error("failed to create gfx pipeline");
}

void
VulkanContext::create_framebuffer()
{
  m_swapchain_framebuffers.resize(m_swapchain_image_views.size());
  for(unsigned int i = 0; i < m_swapchain_image_views.size(); i++)
  {
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = m_render_pass;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &m_swapchain_image_views[i];
    create_info.width = m_swap_extent.width;
    create_info.height = m_swap_extent.height;
    create_info.layers = 1;
    
    VkResult result = vkCreateFramebuffer(m_device, &create_info, nullptr, &m_swapchain_framebuffers[i]);
    if (result != VK_SUCCESS)
      throw new std::runtime_error("failed to create framebuffer");
  }
}

void
VulkanContext::create_command_pool()
{
  QueueFamily family = find_queue_families(m_physical_device);

  VkCommandPoolCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.queueFamilyIndex = family.graphics.value();

  VkResult result = vkCreateCommandPool(m_device, &create_info, nullptr, &m_command_pool);
  if (result != VK_SUCCESS)
    throw new std::runtime_error("failed to create command pool");
}

void
VulkanContext::create_command_buffers()
{
  m_command_buffers.resize(m_swapchain_framebuffers.size());

  VkCommandBufferAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.commandPool = m_command_pool;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = (uint32_t)m_command_buffers.size();

  VkResult result = vkAllocateCommandBuffers(m_device, &allocate_info, m_command_buffers.data());
  if (result != VK_SUCCESS)
    throw new std::runtime_error("failed to create command buffers");
  
  VkCommandBufferBeginInfo command_buffer_info{};
  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VkClearValue clear_value = { 0.0f, 0.0f, 0.0f, 1.0f };
  for (unsigned int i = 0; i < m_command_buffers.size(); i++)
  {
    VkResult result2 = vkBeginCommandBuffer(m_command_buffers[i], &command_buffer_info);
    if (result2 != VK_SUCCESS)
      throw std::runtime_error("failed to start command buffer");
    
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_render_pass;
    render_pass_info.framebuffer = m_swapchain_framebuffers[i];
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = m_swap_extent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;
    vkCmdBeginRenderPass(m_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_gfx_pipeline);

    vkCmdDraw(m_command_buffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(m_command_buffers[i]);

    VkResult result3 = vkEndCommandBuffer(m_command_buffers[i]);
    if (result3 != VK_SUCCESS)
      throw std::runtime_error("failed to end command buffer");
  }
}

bool
VulkanContext::is_device_suitable(VkPhysicalDevice device)
{
  QueueFamily family = find_queue_families(device);
  bool extensions_supported = true;
  bool swapchain_supported = false;
  // TODO:
  /*std::vector<VkExtensionProperties> extensions = get_extensions_for_device(device);
  for (unsigned int i = 0; i < sizeof(required_device_extensions) / sizeof(const char*); i++)
  {
    if (std::find(extensions.begin(), extensions.end(), required_device_extensions[i]) == extensions.end())
    {
      extensions_supported = false;
      break;
    }
  }*/
  if (extensions_supported)
  {
    SwapchainDetails details = find_swapchain_features(device);
    swapchain_supported = !details.formats.empty() && !details.present_modes.empty();
  }
  return family.graphics.has_value() && family.presentation.has_value() && extensions_supported && swapchain_supported;
}

VulkanContext::QueueFamily
VulkanContext::find_queue_families(VkPhysicalDevice device)
{
  QueueFamily out{};
  unsigned int queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
  VkQueueFamilyProperties queue_families[queue_family_count];
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
  VkBool32 presentation_support = false;
  for(unsigned int i = 0; i < queue_family_count; i++)
  {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      out.graphics = i;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_main_surface, &presentation_support);
    if (presentation_support)
      out.presentation = i;
  }
  return out;
}

std::vector<VkExtensionProperties>
VulkanContext::get_extensions_for_device(VkPhysicalDevice device)
{
  unsigned int extension_count = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> out(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, out.data());

  return out;
}

VulkanContext::SwapchainDetails
VulkanContext::find_swapchain_features(VkPhysicalDevice device)
{
  SwapchainDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_main_surface, &details.capabilities);

  unsigned int format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_main_surface, &format_count, nullptr);
  if (format_count != 0)
  {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_main_surface, &format_count, details.formats.data());
  }

  unsigned int present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_main_surface, &present_mode_count, nullptr);
  if (present_mode_count != 0)
  {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_main_surface, &present_mode_count, details.present_modes.data());
  }

  return details;
}

VkSurfaceFormatKHR
VulkanContext::choose_swapchain_format(const SwapchainDetails& details)
{
  for (unsigned int i = 0; i < details.formats.size(); i++)
  {
    if (details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && details.formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
      return details.formats[i];
  }
  return details.formats[0];
}

VkPresentModeKHR
VulkanContext::choose_swapchain_present_mode(const SwapchainDetails& details)
{
  for (unsigned int i = 0; i < details.present_modes.size(); i++)
  {
    if (details.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
      return details.present_modes[i];
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
VulkanContext::choose_swap_extent(const SwapchainDetails& details)
{
  if (details.capabilities.currentExtent.width != UINT32_MAX)
    return details.capabilities.currentExtent;
  int w, h;
  SDL_Vulkan_GetDrawableSize(m_video_system.get_window(), &w, &h);
  VkExtent2D extent = { static_cast<unsigned int>(w), static_cast<unsigned int>(h) };
  extent.width = boost::algorithm::clamp(extent.width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
  extent.height = boost::algorithm::clamp(extent.height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);
  return extent;
}

/* EOF */
