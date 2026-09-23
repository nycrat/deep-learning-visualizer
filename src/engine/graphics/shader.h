#pragma once

/** @file */

#include <cstdint>
#include <filesystem>

namespace engine::graphics {

/**
 * Represents an OpenGL shader program.
 */
class shader {
public:
  /**
   * Constructs a program that compiles and links a vertex and fragment shader.
   *
   * @param vertex_shader_path The path to a .vert shader file, relative to the
   *                           project root.
   * @param fragment_shader_path The path to a .frag shader file, relative to
   *                             the project root.
   */
  shader(const std::filesystem::path &vertex_shader_path,
         const std::filesystem::path &fragment_shader_path);

  /**
   * Destructor that deletes the OpenGL program.
   */
  ~shader();

  shader(const shader &) = delete;
  shader &operator=(const shader &) = delete;
  shader(shader &&) = delete;
  shader &operator=(shader &&) = delete;

  /**
   * Installs this program in the current OpenGL context.
   */
  void use();

  [[nodiscard]] std::uint32_t id() const;

private:
  std::uint32_t id_;
};

} // namespace engine::graphics
