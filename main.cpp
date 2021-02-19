
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
  planes.push_back(new Plane({0, 0, 0}, {0, 2, 0}, {0, 2, -1}, {0, 0, -1}));                //wall 1
  planes.push_back(new Plane({0, 0, -1}, {0, 2, -1}, {-2, 2, -1}, {-2, 0, -1}));            //wall 2
  planes.push_back(new Plane({-2, 0, -1}, {-2, 2, -1}, {-2, 2, -9}, {-2, 0, -9}));          //wall 3
  planes.push_back(new Plane({-2, 0, -9}, {-2, 2, -9}, {0, 2, -9}, {0, 0, -9}));            //wall 4
  planes.push_back(new Plane({0, 0, -9}, {0, 2, -9}, {0, 2, -10.5}, {0, 0, -10.5}));        //wall 5
  planes.push_back(new Plane({0, 0, -10.5}, {0, 2, -10.5}, {-6, 2, -10.5}, {-6, 0, -10.5}));//wall 6
  planes.push_back(new Plane({-6, 0, -10.5}, {-6, 2, -10.5}, {-6, 2, -7}, {-6, 0, -7}));    //wall 7
  planes.push_back(new Plane({-6, 0, -7}, {-6, 2, -7}, {-10, 2, -7}, {-10, 0, -7}));        //wall 8
  planes.push_back(new Plane({-10, 0, -7}, {-10, 2, -7}, {-10, 2, -4}, {-10, 0, -4}));      //wall 9
  planes.push_back(new Plane({-10, 0, -4}, {-10, 2, -4}, {-16, 2, -4}, {-16, 0, -4}));      //wall 10
  planes.push_back(new Plane({-16, 0, -4}, {-16, 2, -4}, {-16, 2, 0}, {-16, 0, 0}));        //wall 11
  planes.push_back(new Plane({-16, 0, 0}, {-16, 2, 0}, {-6, 2, 0}, {-6, 0, 0}));            //wall 12
  planes.push_back(new Plane({-4.5, 0, 0}, {-4.5, 2, 0}, {0, 2, 0}, {0, 0, 0}));            //wall 13

  // inner room 2
  planes.push_back(new Plane({-8, 0, -10.5}, {-8, 2, -10.5}, {-8, 2, -9}, {-8, 0, -9}));          //wall 1
  planes.push_back(new Plane({-8, 0, -9}, {-8, 2, -9}, {-12.5, 2, -9}, {-12.5, 0, -9}));          //wall 2
  planes.push_back(new Plane({-12.5, 0, -9}, {-12.5, 2, -9}, {-12.5, 2, -5.5}, {-12.5, 0, -5.5}));//wall 3
  planes.push_back(new Plane({-12.5, 0, -5.5}, {-12.5, 2, -5.5}, {-18, 2, -5.5}, {-18, 0, -5.5}));//wall 4
  planes.push_back(new Plane({-18, 0, -5.5}, {-18, 2, -5.5}, {-18, 2, -3}, {-18, 0, -3}));        //wall 5
  planes.push_back(new Plane({-18, 0, -3}, {-18, 2, -3}, {-21, 2, -3}, {-21, 0, -3}));            //wall 6
  planes.push_back(new Plane({-21, 0, -3}, {-21, 2, -3}, {-21, 2, -5.5}, {-21, 0, -5.5}));        //wall 7
  planes.push_back(new Plane({-21, 0, -5.5}, {-21, 2, -5.5}, {-24, 2, -5.5}, {-24, 0, -5.5}));    //wall 8
  planes.push_back(new Plane({-24, 0, -5.5}, {-24, 2, -5.5}, {-26, 2, -9}, {-26, 0, -9}));        //wall 9
  planes.push_back(new Plane({-26, 0, -9}, {-26, 2, -9}, {-26, 2, -11}, {-26, 0, -11}));          //wall 10
  planes.push_back(new Plane({-26, 0, -11}, {-26, 2, -11}, {-25, 2, -11}, {-25, 0, -11}));        //wall 11
  planes.push_back(new Plane({-25, 0, -11}, {-25, 2, -11}, {-25, 2, -9}, {-25, 0, -9}));          //wall 12
  planes.push_back(new Plane({-25, 0, -9}, {-25, 2, -9}, {-14, 2, -9}, {-14, 0, -9}));            //wall 13
  planes.push_back(new Plane({-14, 0, -9}, {-14, 2, -9}, {-14, 2, -10.5}, {-14, 0, -10.5}));      //wall 13
  planes.push_back(new Plane({-8, 0, -10.5}, {-8, 2, -10.5}, {-14, 2, -10.5}, {-14, 0, -10.5}));  //wall 15

  // inner room 3
  for (auto &plain : planes) {
	plain->compile();
  }
  while (!app.getShouldClose()) {
	app.getWindow()->updateFpsCounter();

	auto currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	moveCamera();

	Renderer::clear({0, 0, 0, 1});
	shader.bind();
	shader.setUniformMat4f("u_MVP", camera->getMVP());
	renderScene(&shader, meshes, planes);

	glCall(glfwSwapBuffers(app.getWindow()->getGLFWWindow()));
	glfwPollEvents();
	while (glfwGetTime() < lasttime + 1.0 / 60) {
	  // TODO: Put the thread to sleep, yield, or simply do nothing
	}
	lasttime += 1.0 / 60;
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