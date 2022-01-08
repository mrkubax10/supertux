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

#ifndef HEADER_SUPERTUX_VIDEO_VK_VULKAN_VIDEO_SYSTEM_HPP
#define HEADER_SUPERTUX_VIDEO_VK_VULKAN_VIDEO_SYSTEM_HPP

#include <memory>

#include "video/sdlbase_video_system.hpp"
#include "video/vk/vulkan_context.hpp"
#include "video/vk/vulkan_screen_renderer.hpp"

class VulkanVideoSystem final : public SDLBaseVideoSystem
{
public:
  VulkanVideoSystem();
  ~VulkanVideoSystem() override;

  virtual std::string get_name() const override;
  virtual Renderer* get_back_renderer() const override;
  virtual Renderer& get_renderer() const override { return *m_screen_renderer; }
  virtual Renderer& get_lightmap() const override;
  virtual TexturePtr new_texture(const SDL_Surface& image, const Sampler& sampler = Sampler()) override;
  virtual const Viewport& get_viewport() const override;
  virtual void apply_config() override;
  virtual void flip() override;
  virtual void set_vsync(int mode) override;
  virtual int get_vsync() const override;
  virtual SDLSurfacePtr make_screenshot() override;

  SDL_Window* get_window() { return m_sdl_window.get(); }
  VulkanContext& get_context() { return *m_context; }

private:
  void create_vk_window();
  void create_vk_context();

private:
  std::unique_ptr<VulkanContext> m_context;
  std::unique_ptr<VulkanScreenRenderer> m_screen_renderer;

private:
  VulkanVideoSystem(const VulkanVideoSystem&) = delete;
  VulkanVideoSystem& operator=(const VulkanVideoSystem&) = delete;
};

#endif
/* EOF */
