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

#ifndef HEADER_SUPERTUX_VIDEO_VK_VULKAN_SCREEN_RENDERER_HPP
#define HEADER_SUPERTUX_VIDEO_VK_VULKAN_SCREEN_RENDERER_HPP

#include <vulkan/vulkan.h>
#include <vector>

#include "video/vk/vulkan_renderer.hpp"

class VulkanScreenRenderer final : public VulkanRenderer
{
public:
  VulkanScreenRenderer(VulkanVideoSystem& video_system);
  ~VulkanScreenRenderer() override;

  virtual void start_draw() override;
  virtual void end_draw() override;
  virtual Rect get_rect() const override;
  virtual Size get_logical_size() const override;
  virtual TexturePtr get_texture() const override { return {}; }

private:
  std::vector<VkSemaphore> m_image_available;
  std::vector<VkSemaphore> m_render_finished;
  std::vector<VkFence> m_fences;
  std::vector<VkFence> m_image_fences;
  unsigned int m_frame;

private:
  VulkanScreenRenderer(const VulkanScreenRenderer&) = delete;
  VulkanScreenRenderer& operator=(const VulkanScreenRenderer&) = delete;
};

#endif
/* EOF */
