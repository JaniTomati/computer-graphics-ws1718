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
  void initializeTextures();
  void updateView();

  model_object planet_object; // cpu representation of model
  std::vector<planet> m_planet_list;

  model_object star_object;
  std::vector<GLfloat> m_star_list;

  model_object orbit_object;
  std::vector<GLfloat> m_orbit_list;

  texture_object texture_object;
  std::map<std::string, pixel_data> m_loaded_textures;

  int shader_Mode = 1;
};

#endif
