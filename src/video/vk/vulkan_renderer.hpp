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

#ifndef HEADER_SUPERTUX_VIDEO_VK_VULKAN_RENDERER_HPP
#define HEADER_SUPERTUX_VIDEO_VK_VULKAN_RENDERER_HPP

#include "video/renderer.hpp"
#include "video/vk/vulkan_painter.hpp"

class VulkanVideoSystem;

class VulkanRenderer : public Renderer
{
public:
  VulkanRenderer(VulkanVideoSystem& video_system);

  virtual Painter& get_painter() { return m_painter; }

protected:
  VulkanVideoSystem& m_video_system;
  VulkanPainter m_painter;

private:
  VulkanRenderer(const VulkanRenderer&) = delete;
  VulkanRenderer& operator=(const VulkanRenderer&) = delete;
};

#endif
/* EOF */
