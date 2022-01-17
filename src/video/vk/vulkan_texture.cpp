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

#include "video/vk/vulkan_texture.hpp"

#include "video/vk/vulkan_video_system.hpp"

VulkanTexture::VulkanTexture(VulkanVideoSystem& video_system int width, int height, boost::optional<Color> fill_color) :
  m_video_system(video_system),
  m_pixel_buffer(VK_NULL_HANDLE),
  m_pixel_buffer_memory(VK_NULL_HANDLE)
{
  VkBufferCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  create_info.size = width * height * 4;
  create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult result = vkCreateBuffer(video_system.get_context().get_device(), &create_info, nullptr, &m_pixel_buffer);
  if (result != VK_SUCCESS)
    throw std::runtime_error("failed to allocate pixel memory");
}