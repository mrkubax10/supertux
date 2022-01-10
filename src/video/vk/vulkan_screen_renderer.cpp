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

#include "video/vk/vulkan_screen_renderer.hpp"

#include "video/vk/vulkan_video_system.hpp"

namespace{
const unsigned int MAX_FRAMES = 2;
}

VulkanScreenRenderer::VulkanScreenRenderer(VulkanVideoSystem& video_system) :
  VulkanRenderer(video_system),
  m_image_available(),
  m_render_finished(),
  m_fences(),
  m_image_fences(),
  m_frame(0)
{
  m_image_available.resize(MAX_FRAMES);
  m_render_finished.resize(MAX_FRAMES);
  m_fences.resize(MAX_FRAMES);
  m_image_fences.resize(m_video_system.get_context().get_swapchain_images().size(), VK_NULL_HANDLE);
  VkSemaphoreCreateInfo semaphore_create_info{};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_create_info{};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (unsigned int i = 0; i < MAX_FRAMES; i++)
  {
    VkResult result1 = vkCreateSemaphore(m_video_system.get_context().get_device(), &semaphore_create_info, nullptr, &m_image_available[i]);
    VkResult result2 = vkCreateSemaphore(m_video_system.get_context().get_device(), &semaphore_create_info, nullptr, &m_render_finished[i]);
    VkResult result3 = vkCreateFence(m_video_system.get_context().get_device(), &fence_create_info, nullptr, &m_fences[i]);
    if (result1 != VK_SUCCESS || result2 != VK_SUCCESS || result3 != VK_SUCCESS)
      throw std::runtime_error("failed to create semaphore/fence");
  }
}

VulkanScreenRenderer::~VulkanScreenRenderer()
{
  for (unsigned int i = 0; i < MAX_FRAMES; i++)
  {
    vkDestroySemaphore(m_video_system.get_context().get_device(), m_image_available[i], nullptr);
    vkDestroySemaphore(m_video_system.get_context().get_device(), m_render_finished[i], nullptr);
    vkDestroyFence(m_video_system.get_context().get_device(), m_fences[i], nullptr);
  }
}

void
VulkanScreenRenderer::start_draw()
{

}

void
VulkanScreenRenderer::end_draw()
{
  vkWaitForFences(m_video_system.get_context().get_device(), 1, &m_fences[m_frame], VK_TRUE, UINT64_MAX);
  vkResetFences(m_video_system.get_context().get_device(), 1, &m_fences[m_frame]);

  unsigned int image_index;
  vkAcquireNextImageKHR(m_video_system.get_context().get_device(), m_video_system.get_context().get_swapchain(), UINT64_MAX, m_image_available[m_frame], VK_NULL_HANDLE, &image_index);

  if (m_image_fences[image_index] != VK_NULL_HANDLE)
    vkWaitForFences(m_video_system.get_context().get_device(), &m_image_fences[image_index], VK_TRUE, UINT64_MAX);
  m_image_fences[image_index] = m_fences[m_frame];
  
  VkSubmitInfo submit_info{};
  VkPipelineStageFlags flags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &m_image_available[m_frame];
  submit_info.pWaitDstStageMask = flags;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &m_video_system.get_context().get_command_buffers().at(image_index);
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &m_render_finished[m_frame];

  vkResetFences(m_video_system.get_context().get_device(), 1, &m_fences[m_frame]);

  vkQueueSubmit(m_video_system.get_context().get_graphics_queue(), 1, &submit_info, m_fences[m_frame]);

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &m_render_finished[m_frame];
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &m_video_system.get_context().get_swapchain();
  present_info.pImageIndices = &image_index;

  vkQueuePresentKHR(m_video_system.get_context().get_presentation_queue(), &present_info);

  m_frame = (m_frame + 1) % MAX_FRAMES;
}

Rect
VulkanScreenRenderer::get_rect()
{

}

Size
VulkanScreenRenderer::get_logical_size()
{

}

/* EOF */
