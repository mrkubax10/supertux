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

#ifndef HEADER_SUPERTUX_VIDEO_VK_VULKAN_SHADER_HPP
#define HEADER_SUPERTUX_VIDEO_VK_VULKAN_SHADER_HPP

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

class VulkanContext;
class VulkanShader final
{
public:
  static std::unique_ptr<VulkanShader> from_file(const VulkanContext& context, const std::string& filename);
  static std::unique_ptr<VulkanShader> from_bytecode(const VulkanContext& context, const std::vector<char> bytecode);

public:
  VulkanShader(const VulkanContext& context);
  ~VulkanShader();

  void create(std::vector<char> bytecode);

  VkShaderModule get_handle() const { return m_module; }

private:
  const VulkanContext& m_context;
  VkShaderModule m_module;

private:
  VulkanShader(const VulkanShader&) = delete;
  VulkanShader& operator=(const VulkanShader&) = delete;
};

#endif
/* EOF */
