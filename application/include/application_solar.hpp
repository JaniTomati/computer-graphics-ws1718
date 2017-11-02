#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

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

  // void calculateOrbit(glm::fmat4 model_matrix, glm::vec3 planet_distance);

  // caculate and upload the model- and normal matrix
  void uploadPlanetTransforms(planet const& planet_instance) const;
  // react to key input
  void keyCallback(int key, int scancode, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);

  // draw all objects
  void render() const;

 protected:
  void initializeOrbit();
  void initializeStars(unsigned int );
  void initializePlanets();
  void initializeShaderPrograms();
  void initializeGeometry();
  void updateView();

  // cpu representation of model
  model_object planet_object;
  // a vector that saves all the planets that are to be rendered in the program
  std::vector<planet> m_planet_list;

  model_object star_object;

  std::vector<GLfloat> m_star_list;

 model_object orbit_object;

 std::vector<GLfloat> m_orbit_list;
};

#endif
