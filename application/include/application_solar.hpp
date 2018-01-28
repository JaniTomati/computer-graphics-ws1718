#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"
#include "texture_loader.hpp"

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // update uniform locations and values
  void uploadUniforms();
  // update projection matrix
  void updateProjection();
  // calculate model matrix
  glm::fmat4 calculatePlanetModelMatrix(glm::fmat4 model_matrix, planet const& planet_instance) const;

  void calculateOrbit(planet const& planet_instance) const;

  // caculate and upload the model- and normal matrix
  void uploadPlanetTransforms(planet const& planet_instance) const;
  // react to key input
  void keyCallback(int key, int scancode, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);
  // use texture_loader to load textures
  void loadTextures();

  // draw all objects
  void render() const;

 protected:
  void initializeOrbit();
  void initializeStars(unsigned int number_stars);
  void initializePlanets();
  void initializeShaderPrograms();
  void initializeGeometry();
  void initializeFramebuffer();
  void initializeScreenQuad();
  void initializeTextures();
  void initializeSkybox();
  void updateView();

  model_object planet_object; // cpu representation of model
  std::vector<planet> m_planet_list;

  model_object star_object;
  std::vector<GLfloat> m_star_list;

  model_object orbit_object;
  std::vector<GLfloat> m_orbit_list;

  texture_object tex_object;
  std::vector<pixel_data> m_loaded_textures;

  texture_object tex_object_normal;
  std::vector<pixel_data> m_loaded_normal_mappings;

  model_object skybox_object;
  std::vector<glm::vec3> skybox_coordinates;

  std::vector<texture_object> m_texture_objects;
  texture_object m_texture_objects_skybox;

  // buffer objects
  renderbuffer_object rb_object;
  framebuffer_object fb_object;

  texture_object fb_tex_object;
  texture_object quad_tex_object;

  // quad object
  model_object quad_object;

  GLenum draw_buffers[1];
  GLenum status;

  int shader_Mode = 1;
  bool greyscale_Mode = false;
  bool horizontal_Mode = false;
  bool vertical_Mode = false;
  bool blur_Mode = false;
  bool godray_Mode = false;

};

#endif
