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

#include "video/vk/vulkan_painter.hpp"

#include "video/vk/vulkan_video_system.hpp"
#include "video/vk/vulkan_texture.hpp"
#include "video/drawing_request.hpp"

VulkanPainter::VulkanPainter(VulkanVideoSystem& video_system) :
  m_video_system(video_system),
  m_vertex_buffer_previous_size(0),
  m_vertex_buffer(VK_NULL_HANDLE),
  m_uv_buffer(VK_NULL_HANDLE),
  m_vertex_buffer_memory(VK_NULL_HANDLE),
  m_uv_buffer_memory(VK_NULL_HANDLE)
{
  create_buffers();
}

VulkanPainter::~VulkanPainter()
{
  vkDestroyBuffer(m_video_system.get_context().get_device(), m_vertex_buffer, nullptr);
  vkFreeMemory(m_video_system.get_context().get_device(), m_vertex_buffer_memory, nullptr);
  vkDestroyBuffer(m_video_system.get_context().get_device(), m_uv_buffer, nullptr);
  vkFreeMemory(m_video_system.get_context().get_device(), m_uv_buffer_memory, nullptr);
}

void
VulkanPainter::draw_texture(const TextureRequest& request)
{
  const VulkanTexture& texture = static_cast<const VulkanTexture&>(request.texture);

  assert(request.srcrects.size() == request.dstrects.size());
  assert(request.srcrects.size() == request.angles.size());

  
}

void
VulkanPainter::create_buffers()
{
  if (m_vertex_buffer)
  {
    vkDestroyBuffer(m_video_system.get_context().get_device(), m_vertex_buffer, nullptr);
    vkFreeMemory(m_video_system.get_context().get_device(), m_vertex_buffer_memory, nullptr);
  }
  if (m_uv_buffer)
  {
    vkDestroyBuffer(m_video_system.get_context().get_device(), m_uv_buffer, nullptr);
    vkFreeMemory(m_video_system.get_context().get_device(), m_uv_buffer_memory, nullptr);
  }

  VkBufferCreateInfo buffer_create_info{};
  buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_create_info.size = m_vertex_buffer_previous_size * sizeof(float);
  buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VkResult result = vkCreateBuffer(m_video_system.get_context().get_device(), &buffer_create_info, nullptr, &m_vertex_buffer);
  VkResult result2 = vkCreateBuffer(m_video_system.get_context().get_device(), &buffer_create_info, nullptr, &m_uv_buffer);
  if (result != VK_SUCCESS || result2 != VK_SUCCESS)
    throw std::runtime_error("failed to create buffers");
  
  VkMemoryRequirements memory_req{};
  vkGetBufferMemoryRequirements(m_video_system.get_context().get_device(), m_vertex_buffer, &memory_req);
  
  VkMemoryAllocateInfo mem_info{};
  mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_info.allocationSize = memory_req.size;
  mem_info.memoryTypeIndex = get_memory_type_index(memory_req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkAllocateMemory(m_video_system.get_context().get_device(), &mem_info, nullptr, &m_vertex_buffer_memory) != VK_SUCCESS ||
      vkAllocateMemory(m_video_system.get_context().get_device(), &mem_info, nullptr, &m_uv_buffer_memory) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate memory");
  
  vkBindBufferMemory(m_video_system.get_context().get_device(), m_vertex_buffer, m_vertex_buffer_memory, 0);
  vkBindBufferMemory(m_video_system.get_context().get_device(), m_uv_buffer, m_uv_buffer_memory, 0);
}

void
VulkanPainter::draw_arrays()
{
  
}

uint32_t
VulkanPainter::get_memory_type_index(uint32_t type, VkMemoryPropertyFlags flags)
{
  VkPhysicalDeviceMemoryProperties properties{};
  vkGetPhysicalDeviceMemoryProperties(m_video_system.get_context().get_physical_device(), &properties);
  for (uint32_t i = 0; i < properties.memoryTypeCount; i++)
  {
    if ((type & (1 << i)) && (properties.memoryTypes[i].propertyFlags & flags) == flags)
      return i;
  }
  return 0;
}

/* EOF */
