#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <map>
#include <glbinding/gl/gl.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>

// use gl definitions from glbinding
using namespace gl;

// planet type enum
typedef enum {
  _moon,
  _sun,
  _planet
} Planet_Type;

// gpu representation of model
struct model_object {
  // vertex array object
  GLuint vertex_AO = 0;
  // vertex buffer object
  GLuint vertex_BO = 0;
  // index buffer object
  GLuint element_BO = 0;
  // primitive type to draw
  GLenum draw_mode = GL_NONE;
  // indices number, if EBO exists
  GLsizei num_elements = 0;
};

// gpu representation of texture
struct texture_object {
  // handle of texture object
  GLuint handle = 0;
  // binding point
  GLenum target = GL_NONE;
};

// gpu representation of framebuffer
struct framebuffer_object {
  GLuint handle = 0;
};

// gpu representation of renderbuffer
struct renderbuffer_object {
  GLuint handle = 0;
};

// shader handle and uniform storage
struct shader_program {
  shader_program(std::string const& vertex, std::string const& fragment)
   :vertex_path{vertex}
   ,fragment_path{fragment}
   ,handle{0}
   {}

  // path to shader source
  std::string vertex_path;
  std::string fragment_path;
  // object handle
  GLuint handle;
  // uniform locations mapped to name
  std::map<std::string, GLint> u_locs{};
};

struct planet {
  planet(std::string const& name, float size, float rotation_speed, float self_rotation_speed,
        float distance_to_origin, std::string const& orbit_origin,
        Planet_Type const& type, glm::vec3 const& color, int texture_index, int normal_index) :
    m_name {name},
    m_size {float(size * 0.01f)},
    m_rotation_speed {float(rotation_speed * 0.0005f)},
    m_self_rotation_speed {float(self_rotation_speed * 0.1f)},
    m_distance_to_origin {float(distance_to_origin * 0.01f)},
    m_orbit_origin {orbit_origin},
    m_planet_type {type},
    m_planet_color {color},
    m_texture_index {texture_index},
    m_normal_index {normal_index} {}

  std::string m_name;
  float m_size;
  float m_rotation_speed;
  float m_self_rotation_speed;
  float m_distance_to_origin;
  std::string m_orbit_origin; // orbit planet
  Planet_Type m_planet_type;  // type of planet (_moon, _sun, _planet)
  glm::vec3 m_planet_color;
  int m_texture_index;        // set planet texture index
  int m_normal_index;
};

struct texture {
  texture(std::string const& name, std::string const& file_path) :
    m_name {name},
    m_file_path {file_path} {}

  std::string m_name;
  std::string m_file_path;
};
// Particle struct for point
struct particle {
  particle(glm::vec3 const& pos, glm::vec3 const& velocity, glm::vec4 const& color,
           float size, float angle, float life) :
    m_pos {pos},
    m_velocity {velocity},
    m_color {color},
    m_size {size},
    m_angle {angle},
    m_life {life} {}

  glm::vec3 m_pos, m_velocity;
  glm::vec4 m_color;
  float m_size;
  float m_angle;
  float m_life;
};

#endif
