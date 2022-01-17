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

#ifndef HEADER_SUPERTUX_VIDEO_VK_VULKAN_TEXTURE_HPP
#define HEADER_SUPERTUX_VIDEO_VK_VULKAN_TEXTURE_HPP

#include <boost/optional.hpp>
#include <vulkan/vulkan.h>

#include "video/texture.hpp"
#include "video/color.hpp"
#include "video/sampler.hpp"

class VulkanVideoSystem;

class VulkanTexture final : public Texture
{
public:
  VulkanTexture(VulkanVideoSystem& video_system, int width, int height, boost::optional<Color> fill_color = boost::none);
  VulkanTexture(VulkanVideoSystem& video_system, const SDL_Surface& image, const Sampler& sampler);
  ~VulkanTexture() override;

private:
  VulkanVideoSystem& m_video_system;
  VkBuffer m_pixel_buffer;
  VkDeviceMemory m_pixel_buffer_memory;

private:
  VulkanTexture(const VulkanTexture&) = delete;
  VulkanTexture& operator=(const VulkanTexture&) = delete;
};

#endif
/* EOF */
