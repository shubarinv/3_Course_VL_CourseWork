
#include <glm/gtx/color_space.hpp>
#include <random>

#include "application.hpp"
#include "camera.hpp"
#include "cube_map_texture.hpp"
#include "lights_manager.hpp"
#include "mesh.hpp"

LightsManager *lightsManager;
float lastX = 0;
float lastY = 0;
bool firstMouse = true;
// timing
double deltaTime = 0.0f;// time between current frame and last frame
double lastFrame = 0.0f;
Camera *camera;
int pressedKey = -1;

template<typename Numeric, typename Generator = std::mt19937>
[[maybe_unused]] Numeric random(Numeric from, Numeric to) {
  thread_local static Generator gen(std::random_device{}());

  using dist_type = typename std::conditional<
	  std::is_integral<Numeric>::value, std::uniform_int_distribution<Numeric>, std::uniform_real_distribution<Numeric> >::type;

  thread_local static dist_type dist;

  return dist(gen, typename dist_type::param_type{from, to});
}

[[maybe_unused]] std::vector<glm::vec3> getCoordsForVertices(double xc, double yc, double size, int n) {
  std::vector<glm::vec3> vertices;
  auto xe = xc + size;
  auto ye = yc;
  vertices.emplace_back(xe, yc, ye);
  double alpha = 0;
  for (int i = 0; i < n - 1; i++) {
	alpha += 2 * M_PI / n;
	auto xr = xc + size * cos(alpha);
	auto yr = yc + size * sin(alpha);
	xe = xr;
	ye = yr;
	vertices.emplace_back(xe, yc, ye);
  }
  return vertices;
}

void programQuit([[maybe_unused]] int key, [[maybe_unused]] int action, Application *app) {
  app->close();
  LOG_S(INFO) << "Quiting...";
}

void wasdKeyPress([[maybe_unused]] int key, [[maybe_unused]] int action, [[maybe_unused]] Application *app) {
  if (action == GLFW_PRESS) { pressedKey = key; }
  if (action == GLFW_RELEASE) { pressedKey = -1; }
}

void moveCamera() {
  if (pressedKey == GLFW_KEY_W) { camera->ProcessKeyboard(FORWARD, (float)deltaTime); }
  if (pressedKey == GLFW_KEY_S) { camera->ProcessKeyboard(BACKWARD, (float)deltaTime); }
  if (pressedKey == GLFW_KEY_A) { camera->ProcessKeyboard(LEFT, (float)deltaTime); }
  if (pressedKey == GLFW_KEY_D) { camera->ProcessKeyboard(RIGHT, (float)deltaTime); }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback([[maybe_unused]] GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
	lastX = (float)xpos;
	lastY = (float)ypos;
	firstMouse = false;
  }

  double xoffset = xpos - lastX;
  double yoffset = lastY - ypos;// reversed since y-coordinates go from bottom to top

  lastX = (float)xpos;
  lastY = (float)ypos;

  camera->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] double xoffset, double yoffset) {
  camera->ProcessMouseScroll(yoffset);
}
void renderScene(Shader *shader, std::vector<Mesh *> meshes, std::vector<Plane *> planes);

int main(int argc, char *argv[]) {
  Application app({1280, 720}, argc, argv);
  Application::setOpenGLFlags();
  app.registerKeyCallback(GLFW_KEY_ESCAPE, programQuit);

  app.registerKeyCallback(GLFW_KEY_W, wasdKeyPress);
  app.registerKeyCallback(GLFW_KEY_A, wasdKeyPress);
  app.registerKeyCallback(GLFW_KEY_S, wasdKeyPress);
  app.registerKeyCallback(GLFW_KEY_D, wasdKeyPress);

  lastX = app.getWindow()->getWindowSize().x / 2.0f;
  lastY = app.getWindow()->getWindowSize().y / 2.0f;

  glCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

  Shader shader("shaders/simple_shader.glsl", false);
  shader.bind();
  shader.setUniform1i("u_Texture", 0);

  std::vector<Mesh *> meshes;
  std::vector<Plane *> planes;

  // camera
  camera = new Camera(glm::vec3(0, 1, 0));
  camera->setWindowSize(app.getWindow()->getWindowSize());

  glfwSetCursorPosCallback(app.getWindow()->getGLFWWindow(), mouse_callback);
  glfwSetScrollCallback(app.getWindow()->getGLFWWindow(), scroll_callback);

  double lasttime = glfwGetTime();

  // inner  room 1
  planes.push_back(new Plane({0, 0, 0}, {0, 2, 0}, {0, 2, -1}, {0, 0, -1}));//wall 1
  planes.back()->setTexScale({1,0.5})->addTexture("textures/vol3-brick-01-norm.jpg");
  planes.push_back(new Plane({0, 0, -1}, {0, 2, -1}, {-2, 2, -1}, {-2, 0, -1}));//wall 2
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-2, 0, -1}, {-2, 2, -1}, {-2, 2, -9}, {-2, 0, -9}));//wall 3
  planes.back()->setTexScale({1,3})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-2, 0, -9}, {-2, 2, -9}, {0, 2, -9}, {0, 0, -9}));//wall 4
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({0, 0, -9}, {0, 2, -9}, {0, 2, -10.5}, {0, 0, -10.5}));//wall 5
  planes.back()->setTexScale({1,0.5})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({0, 0, -10.5}, {0, 2, -10.5}, {-6, 2, -10.5}, {-6, 0, -10.5}));//wall 6
  planes.back()->setTexScale({1,2})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-6, 0, -10.5}, {-6, 2, -10.5}, {-6, 2, -7}, {-6, 0, -7}));//wall 7
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-6, 0, -7}, {-6, 2, -7}, {-10, 2, -7}, {-10, 0, -7}));//wall 8
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-10, 0, -7}, {-10, 2, -7}, {-10, 2, -4}, {-10, 0, -4}));//wall 9
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-10, 0, -4}, {-10, 2, -4}, {-16, 2, -4}, {-16, 0, -4}));//wall 10
  planes.back()->setTexScale({1,2})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-16, 0, -4}, {-16, 2, -4}, {-16, 2, 0}, {-16, 0, 0}));//wall 11
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-16, 0, 0}, {-16, 2, 0}, {-6, 2, 0}, {-6, 0, 0}));//wall 12
  planes.back()->setTexScale({1,3})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-4.5, 0, 0}, {-4.5, 2, 0}, {0, 2, 0}, {0, 0, 0}));//wall 13
  planes.back()->setTexScale({1,2})->setTextures(planes[planes.size() - 2]->getTextures());

  // inner room 2
  planes.push_back(new Plane({-8, 0, -10.5}, {-8, 2, -10.5}, {-8, 2, -9}, {-8, 0, -9}));//wall 1
  planes.back()->setTexScale({1,0.5})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-8, 0, -9}, {-8, 2, -9}, {-12.5, 2, -9}, {-12.5, 0, -9}));//wall 2
  planes.back()->setTexScale({1,2})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-12.5, 0, -9}, {-12.5, 2, -9}, {-12.5, 2, -5.5}, {-12.5, 0, -5.5}));//wall 3
  planes.back()->setTexScale({1,2})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-12.5, 0, -5.5}, {-12.5, 2, -5.5}, {-18, 2, -5.5}, {-18, 0, -5.5}));//wall 4
  planes.back()->setTexScale({1,2})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-18, 0, -5.5}, {-18, 2, -5.5}, {-18, 2, -3}, {-18, 0, -3}));//wall 5
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-18, 0, -3}, {-18, 2, -3}, {-21, 2, -3}, {-21, 0, -3}));//wall 6
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-21, 0, -3}, {-21, 2, -3}, {-21, 2, -5.5}, {-21, 0, -5.5}));//wall 7
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-21, 0, -5.5}, {-21, 2, -5.5}, {-24, 2, -5.5}, {-24, 0, -5.5}));//wall 8
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-24, 0, -5.5}, {-24, 2, -5.5}, {-26, 2, -9}, {-26, 0, -9}));//wall 9
  planes.back()->setTexScale({1,0.8})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-26, 0, -9}, {-26, 2, -9}, {-26, 2, -11}, {-26, 0, -11}));//wall 10
  planes.back()->setTexScale({1,0.8})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-26, 0, -11}, {-26, 2, -11}, {-25, 2, -11}, {-25, 0, -11}));//wall 11
  planes.back()->setTexScale({1,0.25})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-25, 0, -11}, {-25, 2, -11}, {-25, 2, -9}, {-25, 0, -9}));//wall 12
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-25, 0, -9}, {-25, 2, -9}, {-14, 2, -9}, {-14, 0, -9}));//wall 13
  planes.back()->setTexScale({1,5})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-14, 0, -9}, {-14, 2, -9}, {-14, 2, -10.5}, {-14, 0, -10.5}));//wall 14
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-8, 0, -10.5}, {-8, 2, -10.5}, {-14, 2, -10.5}, {-14, 0, -10.5}));//wall 15
  planes.back()->setTexScale({1,2})->setTextures(planes[planes.size() - 2]->getTextures());

  // inner room 3
  planes.push_back(new Plane({-18, 0, 0}, {-18, 2, 0}, {-18, 2, -1.5}, {-18, 0, -1.5}));//wall 1
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-18, 0, -1.5}, {-18, 2, -1.5}, {-24, 2, -1.5}, {-24, 0, -1.5}));//wall 2
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-24, 0, -1.5}, {-24, 2, -1.5}, {-24, 2, -3}, {-24, 0, -3}));//wall 3
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-24, 0, -3}, {-24, 2, -3}, {-28, 2, -0.5}, {-28, 0, -0.5}));//wall 4
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-28, 0, -0.5}, {-28, 2, -0.5}, {-28, 2, 0}, {-28, 0, 0}));//wall 5
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-28, 0, 0}, {-28, 2, 0}, {-18, 2, 0}, {-18, 0, 0}));//wall 5
  planes.back()->setTexScale({1,3})->setTextures(planes[planes.size() - 2]->getTextures());

  // inner room 4

  planes.push_back(new Plane({-30, 0, -11.5}, {-30, 2, -11.5}, {-30, 2, -9.5}, {-30, 0, -9.5}));//wall 1
  planes.back()->setTexScale({1,0.5})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-30, 0, -9.5}, {-30, 2, -9.5}, {-33, 2, -7.5}, {-33, 0, -7.5}));//wall 2
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-33, 0, -7.5}, {-33, 2, -7.5}, {-36.5, 2, -7.5}, {-36.5, 0, -7.5}));//wall 3
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-36.5, 0, -7.5}, {-36.5, 2, -7.5}, {-36.5, 2, -11.5}, {-36.5, 0, -11.5}));//wall 4
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-36.5, 0, -11.5}, {-36.5, 2, -11.5}, {-30, 2, -11.5}, {-30, 0, -11.5}));//wall 5
  planes.back()->setTexScale({1,3})->setTextures(planes[planes.size() - 2]->getTextures());

  // inner room 5
  planes.push_back(new Plane({-30, 0, 0}, {-30, 2, 0}, {-30, 2, -1.5}, {-30, 0, -1.5}));//wall 1
  planes.back()->setTexScale({1,0.5})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-30, 0, -1.5}, {-30, 2, -1.5}, {-33, 2, -4.5}, {-33, 0, -4.5}));//wall 2
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-33, 0, -4.5}, {-33, 2, -4.5}, {-36, 2, -4.5}, {-36, 0, -4.5}));//wall 3
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-36, 0, -4.5}, {-36, 2, -4.5}, {-36, 2, 0}, {-36, 0, 0}));//wall 4
  planes.back()->setTexScale({1,1})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-36, 0, 0}, {-36, 2, 0}, {-30, 2, 0}, {-30, 0, 0}));//wall 5
  planes.back()->setTexScale({1,2})->setTextures(planes[planes.size() - 2]->getTextures());

  //map walls
  planes.push_back(new Plane({6, 0, 6}, {6, 2, 6}, {6, 2, -17.5}, {6, 0, -17.5}));//wall right
  planes.back()->setTexScale({1,5.75})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({6, 0, 6}, {6, 2, 6}, {-42.5, 2, 6}, {-42.5, 0, 6}));//wall bottom
  planes.back()->setTexScale({1,16})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-42.5, 0, 6}, {-42.5, 2, 6}, {-42.5, 2, -17.5}, {-42.5, 0, -17.5}));//wall left
  planes.back()->setTexScale({1,5.75})->setTextures(planes[planes.size() - 2]->getTextures());
  planes.push_back(new Plane({-42.5, 0, -17.5}, {-42.5, 2, -17.5}, {6, 2, -17.5}, {6, 0, -17.5}));//wall top
  planes.back()->setTexScale({1,16})->setTextures(planes[planes.size() - 2]->getTextures());

  //floor
  planes.push_back(new Plane({6, 0, 6}, {6, 0, -17.5}, {-42.5, 0, -17.5}, {-42.5, 0, 6}));

  //texts
  planes.push_back(new Plane({-4, 1.5, 5.99}, {-4, 0.5, 5.99}, {-3, 0.5, 5.99}, {-3, 1.5, 5.99}));
  planes.back()->setTexScale({1,1})->addTexture("textures/text.bmp");
  planes.push_back(new Plane({-6, 0.5, -17.499}, {-6, 1.5, -17.499}, {-5, 1.5, -17.499}, {-5, 0.5, -17.499}));
  planes.back()->setTexScale({1,1})->addTexture("textures/text.bmp")->setOrigin({-5.5, 1, -17.499})->setRotation({0, 0, 180});

  //crates
  meshes.push_back(new Mesh("resources/models/Crate1.obj"));
  meshes.back()->setScale({0.5, 0.5, 0.5})->setPosition({5, 0.5, -3})->addTexture("textures/wood2.bmp");
  meshes.push_back(new Mesh(meshes.back()->loadedOBJ));
  meshes.back()->setTextures(meshes[0]->getTextures())->setScale({0.5, 0.5, 0.5})->setPosition({5, 0.5, -5});
  meshes.push_back(new Mesh(meshes[0]->loadedOBJ));
  meshes.back()->setTextures(meshes[0]->getTextures())->setScale({0.5, 0.5, 0.5})->setPosition({5, 0.5, -7});
  meshes.push_back(new Mesh(meshes[0]->loadedOBJ));
  meshes.back()->setTextures(meshes[0]->getTextures())->setScale({0.6, 0.6, 0.6})->setPosition({-13, 0.6, -16.7});

  //wall or smth idk
  meshes.push_back(new Mesh(meshes[0]->loadedOBJ));
  meshes.back()->setTextures(meshes[0]->getTextures())->setScale({0.1, 1, 3})->setPosition({0, 1, -5});

  //pipes
  meshes.push_back(new Mesh("resources/models/cylinder.obj"));
  meshes.back()->setScale({0.5, 1, 0.5})->setPosition({5.4, 0, 5.2})->addTexture("textures/metal.bmp");
  meshes.push_back(new Mesh(meshes[meshes.size() - 1]->loadedOBJ));
  meshes.back()->setTextures(meshes[meshes.size() - 2]->getTextures())->setScale({0.5, 1, 0.5})->setPosition({-20, 0, -16.7});

  meshes.push_back(new Mesh(meshes[meshes.size() - 2]->loadedOBJ));
  meshes.back()->setTextures(meshes[meshes.size() - 2]->getTextures())->setScale({0.5, 1, 0.5})->setPosition({5.3, 0, -16.7});

  meshes.push_back(new Mesh(meshes[meshes.size() - 3]->loadedOBJ));
  meshes.back()->setTextures(meshes[meshes.size() - 2]->getTextures())->setScale({0.5, 1, 0.5})->setPosition({-41.7, 0, -16.7});

  meshes.push_back(new Mesh(meshes[meshes.size() - 4]->loadedOBJ));
  meshes.back()->setTextures(meshes[meshes.size() - 2]->getTextures())->setScale({0.5, 1, 0.5})->setPosition({-41.7, 0, 5.2});

  meshes.push_back(new Mesh("resources/models/StreetLamp.obj"));
  meshes.back()->setPosition({-18, -0.001, 5.3})->setScale({0.15, 0.15, 0.15});
  meshes.push_back(new Mesh("resources/models/StreetLamp.obj"));
  meshes.back()->setPosition({-42, -0.001, -8})->setScale({0.15, 0.15, 0.15})->setOrigin({-42, -0.001, -8})->setRotation({0, 90, 0});
  meshes.push_back(new Mesh("resources/models/bench.blend"));
  meshes.back()->setRotation({270, 0, 180})->setPosition({-16.8, 0.3, 5.2})->setOrigin({-16.8, 0.3, 5.2});
  meshes.push_back(new Mesh(meshes.back()->loadedOBJ));
  meshes.back()->setRotation({270, 0, 90})->setPosition({-41.5, 0.3, -9.4})->setOrigin({-41.5, 0.3, -9.4});
  meshes.push_back(new Mesh("resources/models/Fan.fbx"));
  meshes.back()->setScale({0.035, 0.035, 0.035})->setRotation({0, 0, 0})->setPosition({-5, 2, -4})->setOrigin({-5, 2, -4});
  meshes.push_back(new Mesh("resources/models/Fan.fbx"));
  meshes.back()->setScale({0.035, 0.035, 0.035})->setRotation({0, 0, 0})->setPosition({-28, 2, -5})->setOrigin({-28, 2, -5});
  for (auto &plain : planes) {
	plain->compile();
  }
  for (auto &mesh : meshes) {
	mesh->compile();
  }
  while (!app.getShouldClose()) {
	app.getWindow()->updateFpsCounter();

	auto currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	moveCamera();

	Renderer::clear({0, 0, 0, 1});
	shader.bind();
	camera->passDataToShader(&shader);
	renderScene(&shader, meshes, planes);

	glCall(glfwSwapBuffers(app.getWindow()->getGLFWWindow()));
	glfwPollEvents();
	while (glfwGetTime() < lasttime + 1.0 / 60) {
	  // TODO: Put the thread to sleep, yield, or simply do nothing
	}
	lasttime += 1.0 / 60;
	meshes[meshes.size() - 1]->setRotation(meshes[meshes.size() - 1]->rotation + glm::vec3(0, 2, 0));
	meshes[meshes.size() - 2]->setRotation(meshes[meshes.size() - 1]->rotation + glm::vec3(0, 1, 0));
  }
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
void renderScene(Shader *shader, std::vector<Mesh *> meshes, std::vector<Plane *> planes) {
  for (auto &plane : planes) {
	plane->draw(shader);
  }
  for (auto &mesh : meshes) {
	mesh->draw(shader);
  }
}