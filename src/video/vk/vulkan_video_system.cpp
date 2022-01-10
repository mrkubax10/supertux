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

#include "video/vk/vulkan_video_system.hpp"

#include "supertux/gameconfig.hpp"
#include "video/sdl_surface_ptr.hpp"

VulkanVideoSystem::VulkanVideoSystem() :
  m_context(),
  m_screen_renderer(),
  m_viewport()
{
  create_vk_window();
  create_vk_context();
}

VulkanVideoSystem::~VulkanVideoSystem()
{

}

std::string
VulkanVideoSystem::get_name() const
{
  return "vk";
}

Renderer*
VulkanVideoSystem::get_back_renderer() const
{

}

Renderer&
VulkanVideoSystem::get_lightmap() const
{

}

TexturePtr
VulkanVideoSystem::new_texture(const SDL_Surface& image, const Sampler& sampler)
{

}

void
VulkanVideoSystem::apply_config()
{
  apply_video_mode();
  Size target_size = g_config->use_fullscreen ?
    ((g_config->fullscreen_size == Size(0, 0)) ? m_desktop_size : g_config->fullscreen_size) :
    g_config->window_size;
  m_viewport = Viewport::from_size(target_size, m_desktop_size);
  m_screen_renderer.reset(new VulkanScreenRenderer(*this));

  m_context->on_window_resize();
}

void
VulkanVideoSystem::flip()
{

}

void
VulkanVideoSystem::set_vsync(int mode)
{

}

int VulkanVideoSystem::get_vsync() const
{

}

SDLSurfacePtr
VulkanVideoSystem::make_screenshot()
{

}

void
VulkanVideoSystem::create_vk_window()
{
  create_sdl_window(SDL_WINDOW_VULKAN);
}

void
VulkanVideoSystem::create_vk_context()
{
  m_context.reset(new VulkanContext(*this));
}

/* EOF */
