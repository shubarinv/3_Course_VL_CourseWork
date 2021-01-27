
#include "application.hpp"
#include "camera.hpp"
#include "lights_manager.hpp"
#include "mesh.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "plane.h"
#include "car_manager.hpp"
#include <random>
#include <glm/gtx/color_space.hpp>
#include "cube_map_texture.hpp"



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
Numeric random(Numeric from, Numeric to) {
    thread_local static Generator gen(std::random_device{}());

    using dist_type = typename std::conditional<
            std::is_integral<Numeric>::value, std::uniform_int_distribution<Numeric>, std::uniform_real_distribution<Numeric> >::type;

    thread_local static dist_type dist;

    return dist(gen, typename dist_type::param_type{from, to});
}

std::vector<glm::vec3> getCoordsForVertices(double xc, double yc, double size, int n) {
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
    if (pressedKey == GLFW_KEY_W) { camera->ProcessKeyboard(FORWARD, (float) deltaTime); }
    if (pressedKey == GLFW_KEY_S) { camera->ProcessKeyboard(BACKWARD, (float) deltaTime); }
    if (pressedKey == GLFW_KEY_A) { camera->ProcessKeyboard(LEFT, (float) deltaTime); }
    if (pressedKey == GLFW_KEY_D) { camera->ProcessKeyboard(RIGHT, (float) deltaTime); }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float) xpos;
        lastY = (float) ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;// reversed since y-coordinates go from bottom to top

    lastX = (float) xpos;
    lastY = (float) ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera->ProcessMouseScroll(yoffset);
}

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

    glDepthFunc(GL_LESS);
    glCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    Shader shader_tex("../shaders/lighting_shader.glsl", false);
    shader_tex.bind();
    shader_tex.setUniform1i("NUM_POINT_LIGHTS", 0);
    shader_tex.setUniform1i("NUM_SPOT_LIGHTS", 0);
    shader_tex.setUniform1i("NUM_DIR_LIGHTS", 0);

    Shader shader_skybox("../shaders/skybox_shader.glsl");
    shader_skybox.bind();
    shader_skybox.setUniform1i("skybox", 0);
    shader_skybox.setUniform1f("intensity", 0.3);
    ObjLoader objLoader;
    std::vector<Mesh *> meshes;
    float skyboxIntensity=0;

    //  water.setUniform1i("numDiffLights", 1);

    std::vector<Plane *> planes;
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };
// skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------

    std::vector<std::string> faces{
                    "../textures/skybox/right.jpg",
                    "../textures/skybox/left.jpg",
                    "../textures/skybox/top.jpg",
                    "../textures/skybox/bottom.jpg",
                    "../textures/skybox/front.jpg",
                    "../textures/skybox/back.jpg"};
    unsigned int cubemapTexture = CubeMapTexture::loadCubemap(faces);


    meshes.push_back(new Mesh("../resources/models/mountain.obj"));
    meshes.back()->addTexture("../textures/mountain.png")->setScale({0.2, 1, 0.2})->setPosition(
            {196, 10, -173})->setRotation({180, 43, 0})->compile();

    meshes.push_back(new Mesh(meshes.back()->loadedOBJ));
    meshes.back()->addTexture("../textures/mountain.png")->setScale({0.2, 1.2, 0.45})->setPosition(
            {289, 10, -88})->setRotation({180, 245, 0})->compile();

    meshes.push_back(new Mesh("../resources/models/lake.obj"));
    meshes.back()->addTexture("../textures/sand.png")->setScale({0.1, 0.1, 0.1})->setPosition(
            {0, -0.011, 0})->setRotation({0, 0, 0})->compile();
    lightsManager = new LightsManager;
    lightsManager->addLight(
            LightsManager::DirectionalLight("sun", {75, 0, 0}, {0.1, 0.1, 0.1}, {1,0.5,0.3}, {1,0.5,0.3}));


    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {60, 60, 60}, false));
    planes.back()->addTexture("../textures/Water_002_COLOR.png")->
            addTexture("../textures/Water_001_SPEC.png")->setPosition({-30, -1.3, 30})->compile();

    auto boatObj = objLoader.loadObj("../resources/models/boat.obj");
    meshes.push_back(new Mesh(boatObj));
    meshes.back()->addTexture("../textures/wood.png")->setScale({0.3, 0.3, 0.3})->setPosition(
            {20, -1.5, 0})->setOrigin({20, -1.5, 0})->setRotation({10, 90, 0})->compile();

    Mesh boat(boatObj);
    boat.addTexture("../textures/wood.png")->setScale({0.3, 0.3, 0.3})->setPosition(
            {0, -1.3, 0})->setOrigin({0, -1.3, 0})->setRotation({0, 70, 0})->compile();

    auto treeObj = objLoader.loadObj("../resources/models/lowpolytree.obj");
    auto trees = getCoordsForVertices(0, 0, 100, 400);
    float boatRot = {0};
    bool boatRotPos = true;
    float lightRot = {0};
    for (auto &tree:trees) {
        meshes.push_back(new Mesh(treeObj));
        auto scale = random<float>(0.5f, 2.5f);
        if (scale >= 1) {
            meshes.back()->setPosition(
                    {tree.x + random<float>(-30.f, 30.f), tree.y + scale + 1, tree.z + random<float>(-30.f, 30.f)})->
                    setScale({scale, scale, scale})->compile();
        } else {
            meshes.back()->setPosition(
                    {tree.x + random<float>(-30.f, 30.f),
                     tree.y + scale + 0.3, tree.z + random<float>(-30.f, 30.f)})->
                    setScale({scale, scale, scale})->compile();
        }

    }

    // sand to grass blend
    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {60, 60, 30}, true));
    planes.back()->addTexture("../textures/sand_and_grass_blend.png")->setPosition({-30, 0, 60})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {60, 60, 30}, true));
    planes.back()->addTexture("../textures/sand_and_grass_blend.png")->setPosition({-30, 0, 60})->setRotation(
            {0, 180, 0})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {60, 60, 30}, true));
    planes.back()->addTexture("../textures/sand_and_grass_blend.png")->setPosition({-30, 0, 60})->setRotation(
            {0, 90, 0})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {60, 60, 30}, true));
    planes.back()->addTexture("../textures/sand_and_grass_blend.png")->setPosition({-30, 0, 60})->setRotation(
            {0, 270, 0})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {30, 30, 30}, true));
    planes.back()->addTexture("../textures/sand_and_grass_blend_angle.png")->setPosition({-60, 0, 60})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {30, 30, 30}, true));
    planes.back()->addTexture("../textures/sand_and_grass_blend_angle.png")->setPosition({-60, 0, 60})->setRotation(
            {0, 90, 0})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {30, 30, 30}, true));
    planes.back()->addTexture("../textures/sand_and_grass_blend_angle.png")->setPosition({-60, 0, 60})->setRotation(
            {0, 180, 0})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {30, 30, 30}, true));
    planes.back()->addTexture("../textures/sand_and_grass_blend_angle.png")->setPosition({-60, 0, 60})->setRotation(
            {0, 270, 0})->compile();

    //grass

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {600, 600, 600}, {8, 24}));
    planes.back()->addTexture("../textures/grass.png")->setPosition({-660, 0, 300})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {600, 600, 600}, {8, 24}));
    planes.back()->addTexture("../textures/grass.png")->setPosition({-660, 0, 300})->setRotation(
            {0, 180, 0})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {120, 600, 400}, {24, 8}));
    planes.back()->addTexture("../textures/grass.png")->setPosition({-60, 0, -60})->compile();

    planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {120, 600, 400}, {24, 8}));
    planes.back()->addTexture("../textures/grass.png")->setPosition({-60, 0, -60})->setRotation({0, 180, 0})->compile();

    // camera
    camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
    camera->setWindowSize(app.getWindow()->getWindowSize());

    glfwSetCursorPosCallback(app.getWindow()->getGLFWWindow(), mouse_callback);
    glfwSetScrollCallback(app.getWindow()->getGLFWWindow(), scroll_callback);

    CarManager carManager(lightsManager);
    carManager.addPath(getCoordsForVertices(0, 0, 155, 18000), true, -0.02, 90);
    carManager.addPath(getCoordsForVertices(0, 0, 160, 9000), true, -0.04, 90);
    CarManager::Path path(getCoordsForVertices(0, 0, 180, 9000), true, -0.04, 90);
    for (int i = 0; i < path.path.size(); i += 40) {
        auto segment = path.path[i];
        planes.push_back(new Plane({0, 0, 0}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {15, 10, 30}, false));
        planes.back()->setColor({1, 0, 0})->setPosition({segment.x, 0.01, segment.z})->setOrigin(
                {segment.x, 0.01, segment.z})->setRotation(
                {0, path.initialRot + (path.anglePerTick * (float) i), 0})->compile();
    }
    carManager.addCars(15);


    while (!app.getShouldClose()) {
        app.getWindow()->updateFpsCounter();
        auto currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        moveCamera();
        Renderer::clear({0, 0, 0, 1});


        camera->passDataToShader(&shader_tex);
        lightsManager->passDataToShader(&shader_tex);
        //plane.draw(&shader_tex);
        for (auto &plane:planes) {
            plane->draw(&shader_tex);
        }
        for (auto &mesh:meshes) {
            mesh->draw(&shader_tex);
        }
        carManager.draw(&shader_tex);
        boat.draw(&shader_tex);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        shader_skybox.bind();
        shader_skybox.setUniform1f("intensity", skyboxIntensity);
        auto view = glm::mat4(glm::mat3(camera->GetViewMatrix())); // remove translation from the view matrix
        shader_skybox.setUniformMat4f("view", view);
        shader_skybox.setUniformMat4f("projection", camera->getProjection());
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glCall(glfwSwapBuffers(app.getWindow()->getGLFWWindow()));
        glfwPollEvents();

        if (boatRotPos)
            boatRot += 0.05;
        else {
            boatRot -= 0.05;
        }
        if (boatRot > 3.0f || boatRot < -5.f) {
            boatRotPos = !boatRotPos;
        }
        boat.setRotation({boatRot, 70, boatRot});
        LOG_S(INFO) << "LightPos: "<<lightRot;
        lightRot += 0.002;
        if (lightRot >= 8) {
            lightRot = 0.005;
        }

       if (lightRot > 5||lightRot<0.5 ) {
           lightsManager->getDirLightByName("sun")->diffuse = {0, 0, 0};
           lightsManager->getDirLightByName("sun")->specular = {0, 0, 0};
           skyboxIntensity=0.4;
       } else {
           lightsManager->getDirLightByName("sun")->specular = {1,0.95,0.79};
           lightsManager->getDirLightByName("sun")->diffuse = {1,0.95,0.79};
           if(lightRot>4){
               lightsManager->getDirLightByName("sun")->specular = {1,lightsManager->getDirLightByName("sun")->specular.y-0.001,lightsManager->getDirLightByName("sun")->specular.z-0.001};
               lightsManager->getDirLightByName("sun")->diffuse = {1,lightsManager->getDirLightByName("sun")->diffuse.y-0.001,lightsManager->getDirLightByName("sun")->diffuse.z-0.001};
               skyboxIntensity-=0.0015;
           }else{
               lightsManager->getDirLightByName("sun")->specular ={1,lightsManager->getDirLightByName("sun")->specular.y+0.001,lightsManager->getDirLightByName("sun")->specular.z+0.001};
               lightsManager->getDirLightByName("sun")->diffuse ={1,lightsManager->getDirLightByName("sun")->diffuse.y+0.001,lightsManager->getDirLightByName("sun")->diffuse.z+0.001};
               skyboxIntensity+=0.002;
           }
           if(skyboxIntensity>1){
               skyboxIntensity=1;
           }
           if(skyboxIntensity<0.3){
               skyboxIntensity=0.3;
           }
       }
        lightsManager->getDirLightByName("sun")->direction = {sin(lightRot), cos(lightRot), cos(lightRot)};
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
