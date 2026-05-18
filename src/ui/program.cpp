#include "program.h"

#include <format>
#include <fstream>
#include <stdexcept>

#include <glad/glad.h>

namespace {

std::string read_file(const std::filesystem::path &file_path) {
  std::ifstream file{file_path, std::ios::binary};

  if (!file) {
    throw std::runtime_error(
        std::format("Failed to open {}", file_path.string()));
  }

  auto file_size = std::filesystem::file_size(file_path);

  std::string buffer(file_size, '\0');
  file.read(buffer.data(), file_size);

  return buffer;
}

void check_compile_errors(std::uint32_t shader) {
  int success{};
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::string info_log(512, '\0');
    glGetShaderInfoLog(shader, 512, nullptr, info_log.data());
    throw std::runtime_error(
        std::format("failed to compile shader\n{}", info_log));
  }
}

void check_linking_errors(std::uint32_t program) {
  int success{};
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    std::string info_log(512, '\0');
    glGetProgramInfoLog(program, 512, nullptr, info_log.data());
    throw std::runtime_error(
        std::format("failed to link program\n{}", info_log));
  }
}

} // namespace

namespace ui {

program::program(const std::filesystem::path &vertex_shader_path,
                 const std::filesystem::path &fragment_shader_path) {
  std::uint32_t vertex_shader{glCreateShader(GL_VERTEX_SHADER)};
  auto vertex_shader_string = read_file(vertex_shader_path);
  auto vertex_shader_source = vertex_shader_string.c_str();
  glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
  glCompileShader(vertex_shader);

  check_compile_errors(vertex_shader);

  std::uint32_t fragment_shader{glCreateShader(GL_FRAGMENT_SHADER)};
  auto fragment_shader_string = read_file(fragment_shader_path);
  auto fragment_shader_source = fragment_shader_string.c_str();
  glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
  glCompileShader(fragment_shader);

  check_compile_errors(fragment_shader);

  id_ = glCreateProgram();

  glAttachShader(id_, vertex_shader);
  glAttachShader(id_, fragment_shader);
  glLinkProgram(id_);

  check_linking_errors(id_);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
}

program::~program() {
  glDeleteProgram(id_);
}

void program::use() {
  glUseProgram(id_);
}

std::uint32_t program::id() const {
  return id_;
}

} // namespace ui
