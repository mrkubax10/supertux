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

#ifndef HEADER_SUPERTUX_VIDEO_VK_VULKAN_PAINTER_HPP
#define HEADER_SUPERTUX_VIDEO_VK_VULKAN_PAINTER_HPP

#include <vulkan/vulkan.h>

#include "video/painter.hpp"

class VulkanVideoSystem;

class VulkanPainter final : public Painter
{
public:
  VulkanPainter(VulkanVideoSystem& video_system);
  ~VulkanPainter() override;

  virtual void draw_texture(const TextureRequest& request) override;
  virtual void draw_gradient(const GradientRequest& request) override;
  virtual void draw_filled_rect(const FillRectRequest& request) override;
  virtual void draw_inverse_ellipse(const InverseEllipseRequest& request) override;
  virtual void draw_line(const LineRequest& request) override;
  virtual void draw_triangle(const TriangleRequest& request) override;
  virtual void clear(const Color& color) override;
  virtual void get_pixel(const GetPixelRequest& request) const override;
  virtual void set_clip_rect(const Rect& rect) override;
  virtual void clear_clip_rect() override;
  

private:
  VulkanVideoSystem& m_video_system;

private:
  VulkanPainter(const VulkanPainter&) = delete;
  VulkanPainter& operator=(const VulkanPainter&) = delete;
};

#endif
/* EOF */
