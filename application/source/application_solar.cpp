#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
{
  initializeGeometry();
  initializeShaderPrograms();
  initializePlanets();
}

void ApplicationSolar::render() const {
  // bind shader to upload uniforms
  glUseProgram(m_shaders.at("planet").handle);

  // iterate planet vector and create planet transforms for each planet
  for (auto const& planet : m_planet_list) {
    uploadPlanetTransforms(planet);

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }
}

void ApplicationSolar::updateView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();

  // bind new shader
  glUseProgram(m_shaders.at("planet").handle);

  updateView();
  updateProjection();
}

// calculate a planets model matrix
glm::fmat4 ApplicationSolar::createPlanetModelMatrix(glm::fmat4 model_matrix, planet const& planet_instance) const {
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime() * planet_instance.m_rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3 {0.0f, 0.0f, -1.0f * planet_instance.m_distance_to_origin});
  model_matrix = glm::scale(model_matrix, glm::fvec3 {planet_instance.m_size, planet_instance.m_size, planet_instance.m_size});
  return model_matrix;
}

// caculate and upload the model- and normal matrix
void ApplicationSolar::uploadPlanetTransforms(planet const& planet_instance) const {
  // create model matrix for our given planet
  glm::fmat4 model_matrix;
  // planet transforms for _moon planets
  if (planet_instance.m_planet_type == _moon) {
    for (auto const& orbit : m_planet_list) {
      if (orbit.m_name == planet_instance.m_orbit_origin) {
        // transform orbit planet
        model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime() * orbit.m_rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
        model_matrix = glm::translate(model_matrix, glm::fvec3 {0.0f, 0.0f, -1.0f * orbit.m_distance_to_origin});

        // transform moon planet
        model_matrix = createPlanetModelMatrix(model_matrix, planet_instance);

        // upload model matrix
        glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                           1, GL_FALSE, glm::value_ptr(model_matrix));
      }
    }
  } else {
    // transform planet (where orbit planet is sun)
    model_matrix = createPlanetModelMatrix(model_matrix, planet_instance);
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
  }
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));
}

// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods) {
  if ((key == GLFW_KEY_W || key == GLFW_KEY_UP) && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
    updateView();
  }
  else if ((key == GLFW_KEY_S || key == GLFW_KEY_DOWN) && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
    updateView();
  }
  else if ((key == GLFW_KEY_A || key == GLFW_KEY_LEFT) && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-0.1f, 0.0f, 0.0f});
    updateView();
  }
  else if ((key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.1f, 0.0f, 0.0f});
    updateView();
  }
  else if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.1f, 0.0f});
    updateView();
  }
  else if (key == GLFW_KEY_C && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, -0.1f, 0.0f});
    updateView();
  }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_y, double pos_x) {
  // mouse handling
  m_view_transform = glm::rotate(m_view_transform, 0.025f, glm::fvec3{pos_x, pos_y, 0.0f});
  updateView();
}

// fill planet list
void ApplicationSolar::initializePlanets() {
  // initialize planets
  planet sun {"sun", 139.278f, 0.0f, 0.0f, "sun", _sun};
  planet earth {"earth", 12.756f, 365.2f, 1496.00f, "sun", _planet};
  planet mercury {"mercury", 4.879f, 88.0f, 579.00f, "sun", _planet};
  planet venus {"venus", 12.104f, 224.7f, 1082.00f, "sun", _planet};
  planet mars {"mars", 6.792f, 687.0f, 2279.0f, "sun", _planet};
  planet jupiter {"jupiter", 142.984f, 4331.0f, 7786.0f, "sun", _planet};
  planet saturn {"saturn", 120.536f, 10747.0f, 14335.0f, "sun", _planet};
  planet uranus {"uranus", 51.118f, 30589.0f, 28725.0f, "sun", _planet};
  planet neptune {"neptune", 49.528f, 59800.0f, 44951.0f, "sun", _planet};
  planet pluto {"pluto", 2.370f, 90560.0f, 59064.0f, "sun", _planet};
  planet moon {"moon", 3.475f, 27.3f, 38.40f, "earth", _moon};

  // insert planets
  m_planet_list.insert(m_planet_list.end(),{sun, earth, mercury, venus, mars,
                       jupiter, saturn, uranus, neptune, pluto, moon});
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/simple.vert",
                                           m_resource_path + "shaders/simple.frag"});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
}

// load models
void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object
  planet_object.num_elements = GLsizei(planet_model.indices.size());
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}
