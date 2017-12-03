#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding
using namespace gl;

// dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <math.h>

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,m_planet_list{}
 ,star_object{}
 ,m_star_list{}
 ,orbit_object{}
 ,m_orbit_list{}
 ,tex_object{}
 ,m_loaded_textures{}
 ,shader_Mode{}
{
  initializePlanets();
  initializeStars(50000);
  initializeOrbit();
  initializeShaderPrograms();
  initializeGeometry();
  initializeTextures();
}

void ApplicationSolar::render() const {
  // render Stars
  glBindVertexArray(star_object.vertex_AO);
  glUseProgram(m_shaders.at("star").handle);
  //glDrawElements(star_object.draw_mode, star_object.num_elements, model::INDEX.type, NULL);
  glDrawArrays(star_object.draw_mode, 0, star_object.num_elements);

  // bind shader to upload uniforms
  glUseProgram(m_shaders.at("planet").handle);
  // iterate planet vector and create planet transforms for each planet
  for (auto const& planet : m_planet_list) {
    // calculates the orbit for each planet and moon
    calculateOrbit(planet);
    // render Orbit
    glBindVertexArray(orbit_object.vertex_AO);
    glUseProgram(m_shaders.at("orbit").handle);
    glDrawArrays(orbit_object.draw_mode, 0, orbit_object.num_elements);

    // activate Texture Unit to which to bind texture
    glActiveTexture(GL_TEXTURE0);
    // bind Texture Object to 2d texture binding point of unit
    glBindTexture(GL_TEXTURE_2D, m_texture_objects[planet.m_texture_index].handle);

    // calculates model- and normal-matrix
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
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                      1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ViewMatrix"),
                      1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ViewMatrix"),
                      1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
                      1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                      1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ProjectionMatrix"),
                      1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                      1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
                      1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();

  updateView();
  updateProjection();
}

// calculate a planets model matrix
glm::fmat4 ApplicationSolar::calculatePlanetModelMatrix(glm::fmat4 model_matrix, planet const& planet_instance) const {
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime() * planet_instance.m_rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3 {0.0f, 0.0f, -1.0f * planet_instance.m_distance_to_origin});
  model_matrix = glm::scale(model_matrix, glm::fvec3 {planet_instance.m_size, planet_instance.m_size, planet_instance.m_size});
  return model_matrix;
}

// calculate orbit depending from origin
void ApplicationSolar::calculateOrbit(planet const& planet_instance) const {
  float planet_distance = planet_instance.m_distance_to_origin;
  // compute orbit for static origin (sun)
  if (planet_instance.m_orbit_origin == "sun") {
    glm::fmat4 model_matrix = glm::scale(glm::fmat4{}, glm::fvec3 {planet_distance, planet_distance, planet_distance});
    glUseProgram(m_shaders.at("orbit").handle);
    glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
    1, GL_FALSE, glm::value_ptr(model_matrix));
  } else {
    // compute orbit for moving origin
    for (auto const& orbit_base : m_planet_list) {
      if (orbit_base.m_name == planet_instance.m_orbit_origin) {
        glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime() * orbit_base.m_rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
        model_matrix = glm::translate(model_matrix, glm::fvec3 {0.0f, 0.0f, -1.0f * orbit_base.m_distance_to_origin});
        model_matrix = glm::scale(model_matrix, glm::fvec3 {planet_distance, planet_distance, planet_distance});
        glUseProgram(m_shaders.at("orbit").handle);
        glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
        1, GL_FALSE, glm::value_ptr(model_matrix));
      }
    }
  }
}

// caculate and upload the model- and normal matrix
void ApplicationSolar::uploadPlanetTransforms(planet const& planet_instance) const {
  // create model matrix for our given planet
  glm::fmat4 model_matrix;
  glm::fmat4 orbit_matrix;
  glm::fmat4 normal_matrix;
  // planet transforms for _moon planets
  if (planet_instance.m_planet_type == _moon) {
    // find the planet which is orbited by the given moon
    for (auto const& orbit : m_planet_list) {
      if (orbit.m_name == planet_instance.m_orbit_origin) {
        // transform orbit planet
        model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime() * orbit.m_rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
        model_matrix = glm::translate(model_matrix, glm::fvec3 {0.0f, 0.0f, -1.0f * orbit.m_distance_to_origin});

        // self rotation
        model_matrix = glm::rotate(model_matrix, float(glfwGetTime() * orbit.m_self_rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});

        // transform moon planet
        model_matrix = calculatePlanetModelMatrix(model_matrix, planet_instance);
        normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
        // upload model matrix
        glUseProgram(m_shaders.at("planet").handle);
        glUniform1i(m_shaders.at("planet").u_locs.at("ShaderMode"), shader_Mode);
        glUniform3f(m_shaders.at("planet").u_locs.at("ColorVector"),
                    planet_instance.m_planet_color.x, planet_instance.m_planet_color.y, planet_instance.m_planet_color.z);
        glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                           1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniform1i(m_shaders.at("planet").u_locs.at("ColorTex"), 0);

        // extra matrix for normal transformation to keep them orthogonal to surface
        // glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
        //                   1, GL_FALSE, glm::value_ptr(normal_matrix));
        break;
      }
    }
  } else if (planet_instance.m_planet_type == _sun){
    // transform planet (where orbit planet is sun)
    model_matrix = calculatePlanetModelMatrix(model_matrix, planet_instance);

    // self rotation
    model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime() * planet_instance.m_self_rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});

    glUseProgram(m_shaders.at("sun").handle);
    glUniform1i(m_shaders.at("sun").u_locs.at("ShaderMode"), shader_Mode);
    glUniform3f(m_shaders.at("sun").u_locs.at("ColorVector"),
                planet_instance.m_planet_color.x, planet_instance.m_planet_color.y, planet_instance.m_planet_color.z);
    glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniform1i(m_shaders.at("sun").u_locs.at("ColorTex"), 0);
  } else {
    model_matrix = calculatePlanetModelMatrix(model_matrix, planet_instance);
    normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);

    // self rotation
    model_matrix = glm::rotate(model_matrix, float(glfwGetTime() * planet_instance.m_self_rotation_speed), glm::fvec3{0.0f, 1.0f, 0.0f});

    glUseProgram(m_shaders.at("planet").handle);
    glUniform1i(m_shaders.at("planet").u_locs.at("ShaderMode"), shader_Mode);
    glUniform3f(m_shaders.at("planet").u_locs.at("ColorVector"),
                planet_instance.m_planet_color.x, planet_instance.m_planet_color.y, planet_instance.m_planet_color.z);
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniform1i(m_shaders.at("planet").u_locs.at("ColorTex"), 0);


    // extra matrix for normal transformation to keep them orthogonal to surface
    // glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
    //                   1, GL_FALSE, glm::value_ptr(normal_matrix));
  }
}

// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods) {
  // move forwards
  if ((key == GLFW_KEY_W || key == GLFW_KEY_UP) && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
    updateView();
  }
  // move backwards
  else if ((key == GLFW_KEY_S || key == GLFW_KEY_DOWN) && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
    updateView();
  }
  // move to the left
  else if ((key == GLFW_KEY_A || key == GLFW_KEY_LEFT) && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-0.1f, 0.0f, 0.0f});
    updateView();
  }
  // move to the right
  else if ((key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.1f, 0.0f, 0.0f});
    updateView();
  }
  // move upwards
  else if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.1f, 0.0f});
    updateView();
  }
  // move downwards
  else if (key == GLFW_KEY_C && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, -0.1f, 0.0f});
    updateView();
  }

  else if (key == GLFW_KEY_1) {
    shader_Mode = 1;
    updateView();
  }

  else if (key == GLFW_KEY_2) {
    shader_Mode = 2;
    updateView();
  }

}

// handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_y, double pos_x) {
  // mouse handling
  m_view_transform = glm::rotate(m_view_transform, -0.025f, glm::fvec3{pos_x, pos_y, 0.0f});
  updateView();

  // NOT WORKING AS INTENDED YET
  // if(pos_y > pos_x) {
  //   m_view_transform = glm::rotate(m_view_transform, -0.025f, glm::fvec3{0.0f, pos_y, 0.0f});
  //   updateView();
  // } else if(pos_y < pos_x){
  //   m_view_transform = glm::rotate(m_view_transform,(float) -pos_x, glm::fvec3{0.0f, 1.0f, 0.0f});
  //   updateView();
  // }
}

void ApplicationSolar::loadTextures() {
  // initialize textures
  texture sun_texture {"sun", m_resource_path + "textures/sun2k.png"};
  texture earth_texture {"earth", m_resource_path + "textures/earth2k.png"};
  texture mercury_texture {"mercury", m_resource_path + "textures/mercury2k.png"};
  texture venus_texture {"venus", m_resource_path + "textures/venus2k.png"};
  texture mars_texture {"mars", m_resource_path + "textures/mars2k.png"};
  texture jupiter_texture {"jupiter", m_resource_path + "textures/jupiter2k.png"};
  texture saturn_texture {"saturn", m_resource_path + "textures/saturn2k.png"};
  texture uranus_texture {"uranus", m_resource_path + "textures/uranus2k.png"};
  texture neptune_texture {"neptune", m_resource_path + "textures/neptune2k.png"};
  texture pluto_texture {"pluto", m_resource_path + "textures/pluto2k.png"};
  texture moon_texture {"moon", m_resource_path + "textures/moon2k.png"};

  // insert textures to m_texture_list
  std::vector<texture> texture_list;
  texture_list.insert(texture_list.end(),{sun_texture, earth_texture,
                      venus_texture, mars_texture, jupiter_texture,
                      mercury_texture, saturn_texture, uranus_texture,
                      neptune_texture, pluto_texture, moon_texture});

  // save loaded textures in map<name, pixel_data>
  for (auto const& texture : texture_list) {
    pixel_data loaded_texture = texture_loader::file(texture.m_file_path);
    std::cout << "Load " << texture.m_name << " texture!" << std::endl;
    m_loaded_textures.push_back(loaded_texture);
  }
}

// fill planet list
void ApplicationSolar::initializePlanets() {
  // initialize planets
  planet sun {"sun", 300.0f, 0.0f, 10.0f, 0.0f, "sun", _sun, glm::vec3 {1.0, 1.0, 0}, 0};
  planet earth {"earth", 12.756f, 365.2f, 23.9f, 1496.00f, "sun", _planet, glm::vec3 {0.0, 1.0, 0.0}, 1};
  planet mercury {"mercury", 4.879f, 88.0f, 1407.6f, 579.00f, "sun", _planet, glm::vec3 {0.5, 0.5, 0.5}, 2};
  planet venus {"venus", 12.104f, 224.7f, -5832.5f, 1082.00f, "sun", _planet, glm::vec3 {0.7, 0.5, 0.1}, 3};
  planet mars {"mars", 6.792f, 687.0f, 24.6f, 2279.0f, "sun", _planet, glm::vec3 {0.5, 0.5, 0.2}, 4};
  planet jupiter {"jupiter", 142.984f, 4331.0f, 9.9f, 7786.0f, "sun", _planet, glm::vec3 {0.4, 0.4, 0.4}, 5};
  planet saturn {"saturn", 120.536f, 10747.0f, 10.7f, 14335.0f, "sun", _planet, glm::vec3 {0.9, 0.8, 0.4}, 6};
  planet uranus {"uranus", 51.118f, 30589.0f, -17.2f, 28725.0f, "sun", _planet, glm::vec3 {0.0, 1.0, 0.0}, 7};
  planet neptune {"neptune", 49.528f, 59800.0f, 16.1f, 44951.0f, "sun", _planet, glm::vec3 {0.0, 0.0, 1.0}, 8};
  planet pluto {"pluto", 2.370f, 90560.0f, -153.3f, 59064.0f, "sun", _planet, glm::vec3 {0.0, 1.0, 0.0}, 9};
  planet moon {"moon", 3.475f, 27.3f*100.0f, 655.7f, 38.40f, "earth", _moon, glm::vec3 {0.0, 1.0, 1.0}, 10};

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
  // m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["ColorVector"] = -1;
  m_shaders.at("planet").u_locs["ShaderMode"] = -1;
  m_shaders.at("planet").u_locs["ColorTex"] = -1;

  // store shader program objects in container
  m_shaders.emplace("sun", shader_program{m_resource_path + "shaders/sun.vert",
                                        m_resource_path + "shaders/sun.frag"});
  // request uniform locations for shader program
  m_shaders.at("sun").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sun").u_locs["ViewMatrix"] = -1;
  m_shaders.at("sun").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("sun").u_locs["ColorVector"] = -1;
  m_shaders.at("sun").u_locs["ShaderMode"] = -1;
  m_shaders.at("sun").u_locs["ColorTex"] = -1;

  // store star shader program objects in container
  m_shaders.emplace("star", shader_program{m_resource_path + "shaders/star.vert",
                                        m_resource_path + "shaders/star.frag"});
  // request uniform locations for star shader program
  m_shaders.at("star").u_locs["ViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;

  // store orbit shader program objects in container
  m_shaders.emplace("orbit", shader_program{m_resource_path + "shaders/orbit.vert",
                                        m_resource_path + "shaders/orbit.frag"});
  // request uniform locations for orbit shader program
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;
}

// fill m_star_list with random star values for given number of stars
void ApplicationSolar::initializeStars(unsigned int number_stars) {
  for (unsigned int i = 0; i < 6 * number_stars; ++i) {
    m_star_list.push_back(static_cast<GLfloat> (rand() % 1000 + 1) - 500);
  }
}

// fill m_orbit_list with with values for circle representation of orbit
void ApplicationSolar::initializeOrbit() {
  for (unsigned int i = 0; i < 359; ++i) {
    m_orbit_list.push_back(static_cast<GLfloat> (cos((i * M_PI)/180)));
    m_orbit_list.push_back(static_cast<GLfloat> (0.0f));
    m_orbit_list.push_back(static_cast<GLfloat> (-sin((i * M_PI)/180)));
  }
}

// load models
void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD);

  model star_model = model{m_star_list, (model::POSITION + model::NORMAL), {1}};

  model orbit_model = model{m_orbit_list, (model::POSITION), {1}};

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
  // activate third attribute on gpu
  glEnableVertexAttribArray(2);
  // third attribute 3 is 3 floats with no offset & stride
  glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);

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

  // generate everything for star_model as well
  glGenVertexArrays(1, &star_object.vertex_AO);
  glBindVertexArray(star_object.vertex_AO);

  glGenBuffers(1, &star_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * star_model.data.size(), star_model.data.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::POSITION]);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::NORMAL]);

  glGenBuffers(1, &star_object.element_BO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, star_object.element_BO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * star_model.indices.size(), star_model.indices.data(), GL_STATIC_DRAW);
  star_object.draw_mode = GL_POINTS;
  star_object.num_elements = GLsizei(star_model.data.size() / 6);

  // generate everything for orbit_model as well
  glGenVertexArrays(1, &orbit_object.vertex_AO);
  glBindVertexArray(orbit_object.vertex_AO);

  glGenBuffers(1, &orbit_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * orbit_model.data.size(), orbit_model.data.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, orbit_model.vertex_bytes, orbit_model.offsets[model::POSITION]);

  glGenBuffers(1, &orbit_object.element_BO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, orbit_object.element_BO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * orbit_model.indices.size(), orbit_model.indices.data(), GL_STATIC_DRAW);

  orbit_object.draw_mode = GL_LINE_LOOP;
  orbit_object.num_elements = GLsizei(orbit_model.data.size() / 3);

  glBindVertexArray(0);
}

void ApplicationSolar::initializeTextures() {
  // load textures using texture loader
  loadTextures();
  int num_planets = m_planet_list.size();

  // Texture specification
  for (int i = 0; i < num_planets; ++i) {
    // 1. activate Texture Unit to which to bind texture
    glActiveTexture(GL_TEXTURE0);
    // 2. generate texture object
    glGenTextures(1, &tex_object.handle);
    // 3. bind Texture Object to 2d texture binding point of unit
    glBindTexture(GL_TEXTURE_2D, tex_object.handle);
    // 4. define interpolation type when fragment covers multiple texels (texture pixels)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // 5. define interpolation type when fragment does not exactly cover one texel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // set the wrap parameter for texture coordinate s and t
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // 6. format Texture Object bound to the 2d binding point
    glTexImage2D(GL_TEXTURE_2D, 0, m_loaded_textures[i].channels, m_loaded_textures[i].width, m_loaded_textures[i].height, 0,
                 m_loaded_textures[i].channels, m_loaded_textures[i].channel_type, m_loaded_textures[i].ptr());

    m_texture_objects.push_back(tex_object);
  }
}

// deconstruct everything
ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);

  glDeleteBuffers(1, &orbit_object.vertex_BO);
  glDeleteBuffers(1, &orbit_object.element_BO);
  glDeleteVertexArrays(1, &orbit_object.vertex_AO);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}
