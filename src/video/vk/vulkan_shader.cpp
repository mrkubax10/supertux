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

#include "video/vk/vulkan_shader.hpp"

#include "physfs/ifile_stream.hpp"
#include "video/vk/vulkan_context.hpp"

std::unique_ptr<VulkanShader>
VulkanShader::from_file(const VulkanContext& context, const std::string& filename)
{
  IFileStream stream(filename);
  std::vector<char> bytecode;

  stream.seekg(0, std::ios::end);
  unsigned int file_size = stream.tellg();
  stream.seekg(0);
  bytecode.resize(file_size);
  stream.read(bytecode.data(), file_size);

  return from_bytecode(context, bytecode);
}

std::unique_ptr<VulkanShader>
VulkanShader::from_bytecode(const VulkanContext& context, const std::vector<char> bytecode)
{
  std::unique_ptr<VulkanShader> shader = std::make_unique<VulkanShader>(context);
  shader->create(bytecode);
  return shader;
}

VulkanShader::VulkanShader(const VulkanContext& context) :
  m_context(context),
  m_module(VK_NULL_HANDLE)
{
}

VulkanShader::~VulkanShader()
{
  vkDestroyShaderModule(m_context.get_device(), m_module, nullptr);
}

void
VulkanShader::create(std::vector<char> bytecode)
{
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = bytecode.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

  VkResult result = vkCreateShaderModule(m_context.get_device(), &create_info, nullptr, &m_module);
  if (result != VK_SUCCESS)
    throw std::runtime_error("shader module creation failed");
}

/* EOF */
