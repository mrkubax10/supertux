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

#ifndef HEADER_SUPERTUX_VIDEO_VK_VULKAN_CONTEXT_HPP
#define HEADER_SUPERTUX_VIDEO_VK_VULKAN_CONTEXT_HPP

#include <vulkan/vulkan.h>
#include <boost/optional.hpp>
#include <vector>

#include "video/vk/vulkan_shader.hpp"

class VulkanVideoSystem;
class VulkanContext final
{
public:
  struct QueueFamily
  {
    boost::optional<unsigned int> graphics;
    boost::optional<unsigned int> presentation;
  };
  struct SwapchainDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };

  static const unsigned int SWAPCHAIN_SIZE;
public:
  VulkanContext(VulkanVideoSystem& video_system);
  ~VulkanContext();

  VkDevice get_device() const { return m_device; }
  VkSwapchainKHR get_swapchain() const { return m_swapchain; }
  std::vector<VkCommandBuffer>& get_command_buffers() const { return m_command_buffers; }
  VkQueue get_graphics_queue() const { return m_graphics_queue; }
  VkQueue get_presentation_queue() const { return m_presentation_queue; }
  std::vector<VkImage>& get_swapchain_images() const { return m_swapchain_images; }

private:
  void init_application();
  void select_device();
  void create_logical_device();
  void create_main_surface();
  void create_swapchain();
  void create_image_views();
  void create_render_pass();
  void create_pipeline();
  void create_framebuffer();
  void create_command_pool();
  void create_command_buffers();

  bool is_device_suitable(VkPhysicalDevice device);
  QueueFamily find_queue_families(VkPhysicalDevice device);
  std::vector<VkExtensionProperties> get_extensions_for_device(VkPhysicalDevice device);
  SwapchainDetails find_swapchain_features(VkPhysicalDevice device);
  VkSurfaceFormatKHR choose_swapchain_format(const SwapchainDetails& details);
  VkPresentModeKHR choose_swapchain_present_mode(const SwapchainDetails& details);
  VkExtent2D choose_swap_extent(const SwapchainDetails& details);

private:
  VulkanVideoSystem& m_video_system;
  VkInstance m_vk_instance;
  VkPhysicalDevice m_physical_device;
  VkDevice m_device;
  VkQueue m_graphics_queue;
  VkQueue m_presentation_queue;
  VkSurfaceKHR m_main_surface;
  VkSwapchainKHR m_swapchain;
  VkFormat m_swap_format;
  VkExtent2D m_swap_extent;
  std::vector<VkImage> m_swapchain_images;
  std::vector<VkImageView> m_swapchain_image_views;
  VkRenderPass m_render_pass;
  VkPipelineLayout m_pipeline_layout;
  VkPipeline m_gfx_pipeline;
  std::vector<VkFramebuffer> m_swapchain_framebuffers;
  VkCommandPool m_command_pool;
  std::vector<VkCommandBuffer> m_command_buffers;

  std::unique_ptr<VulkanShader> m_vert_shader;
  std::unique_ptr<VulkanShader> m_frag_shader;

private:
  VulkanContext(const VulkanContext&) = delete;
  VulkanContext& operator=(const VulkanContext&) = delete;
};

#endif
/* EOF */
