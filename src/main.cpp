#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int loadCubemap(vector<std::string> &faces);
void renderQuad();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool hdr = true;
float exposure = 0.4f;
bool bloom = true;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutoff;
    float outerCutOff;

    glm::vec3 specular;
    glm::vec3 diffuse;
    glm::vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 saturnPosition = glm::vec3(0.0f);
    float saturnScale = 3.0f;
    glm::vec3 ufoPosition = glm::vec3(-1.2f, 8.0f, -1.0f);
    float ufoScale = 0.5f;
    glm::vec3 housePosition = glm::vec3(0.8f, 4.0f, 0.0f);
    float houseScale = 0.4f;
    glm::vec3 mushroomPosition = glm::vec3(-0.4f ,3.3f, -0.4f);
    float mushroomScale = 0.008f;

    DirectionalLight directionalLight;
    SpotLight ufoSpotLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 4.0f, 16.0f)) {}

};

ProgramState *programState;

void DrawImGui();

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);

    programState = new ProgramState;
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // face cull
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // skybox
    float skyboxVertices[] = {
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

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // build and compile shaders
    // -------------------------
    Shader ufoShader("resources/shaders/ufo.vs", "resources/shaders/ufo.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader saturnShader("resources/shaders/saturn.vs", "resources/shaders/saturn.fs");
    Shader hdrShader("resources/shaders/hdr.vs","resources/shaders/hdr.fs");    // load models
    Shader bloomShader("resources/shaders/bloom.vs","resources/shaders/bloom.fs");
    Shader blurShader("resources/shaders/blur.vs","resources/shaders/blur.fs");

    // load models
    // -----------
    Model saturnModel("resources/objects/saturn/Stylized_Planets.obj");
    saturnModel.SetShaderTextureNamePrefix("material.");

    Model ufoModel("resources/objects/ufo/UFO.obj");
    ufoModel.SetShaderTextureNamePrefix("material.");

    Model houseModel("resources/objects/house/uploads_files_4118883_Orange_Hause.obj");
    ufoModel.SetShaderTextureNamePrefix("material.");

    Model mushroomModel("resources/objects/mushroom/Mushrooms1.obj");
    mushroomModel.SetShaderTextureNamePrefix("material.");

    DirectionalLight& directionalLight = programState->directionalLight;
    directionalLight.direction = glm::vec3(-10.0f, -5.0f, -2.0f);
    directionalLight.ambient = glm::vec3(0.2, 0.2, 0.2);
    directionalLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    directionalLight.specular = glm::vec3(2.5);

    SpotLight& ufoSpotLight = programState->ufoSpotLight;
    ufoSpotLight.ambient = glm::vec3(0.3f);
    ufoSpotLight.diffuse = glm::vec3(2.0f, 7.0f, 2.0f);
    ufoSpotLight.specular = glm::vec3(1.0f);
    ufoSpotLight.position = programState->ufoPosition;
    ufoSpotLight.direction = programState->saturnPosition - programState->ufoPosition;
    ufoSpotLight.cutoff = glm::cos(glm::radians(8.5f));
    ufoSpotLight.outerCutOff = glm::cos(glm::radians(10.5f));
    ufoSpotLight.constant = 1.0f;
    ufoSpotLight.linear = 0.35f;
    ufoSpotLight.quadratic = 0.44f;

    vector<std::string> faces {
            FileSystem::getPath("resources/textures/skybox/right.png"),
            FileSystem::getPath("resources/textures/skybox/left.png"),
            FileSystem::getPath("resources/textures/skybox/top.png"),
            FileSystem::getPath("resources/textures/skybox/bottom.png"),
            FileSystem::getPath("resources/textures/skybox/front.png"),
            FileSystem::getPath("resources/textures/skybox/back.png")
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    unsigned int hdrFBO;
    glGenFramebuffers(1,&hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);

    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout<<"SOMETHING AIN'T RIGHT!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    hdrShader.use();
    hdrShader.setInt("hdrBuffer", 0);

    blurShader.use();
    blurShader.setInt("image", 0);

    bloomShader.use();
    bloomShader.setInt("scene", 0);
    bloomShader.setInt("bloomBlur", 1);
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ufoShader.use();
        ufoShader.setVec3("directionalLight.direction", directionalLight.direction);
        ufoShader.setVec3("directionalLight.ambient", 1.5f, 1.5f, 1.7f);
        ufoShader.setVec3("directionalLight.diffuse", directionalLight.diffuse);
        ufoShader.setVec3("directionalLight.specular", directionalLight.specular);
        ufoShader.setVec3("viewPosition", programState->camera.Position);
        ufoShader.setFloat("material.shininess", 32.0f);
        ufoShader.setFloat("material.specular", 0.05f);
        ufoShader.setVec3("ambientLight", glm::vec3(3.0f));

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),(float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ufoShader.setMat4("projection", projection);
        ufoShader.setMat4("view", view);

        // render the ufo model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,programState->ufoPosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->ufoScale));    // it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0, 0.0, 1.0));
        model = glm::rotate(model, glm::radians(float(20 * (glfwGetTime()))), glm::vec3(0.0, 1.0, 0.0));
        ufoShader.setMat4("model", model);
        ufoModel.Draw(ufoShader);

        saturnShader.use();
        saturnShader.setVec3("directionalLight.direction", directionalLight.direction);
        saturnShader.setVec3("directionalLight.ambient", directionalLight.ambient);
        saturnShader.setVec3("directionalLight.diffuse", directionalLight.diffuse);
        saturnShader.setVec3("directionalLight.specular", directionalLight.specular);
        saturnShader.setVec3("ufoLight.ambient", ufoSpotLight.ambient);
        saturnShader.setVec3("ufoLight.diffuse", ufoSpotLight.diffuse);
        saturnShader.setVec3("ufoLight.specular", ufoSpotLight.specular);
        saturnShader.setVec3("ufoLight.position", programState->ufoPosition);
        saturnShader.setVec3("ufoLight.direction", glm::vec3(sin(glfwGetTime()) * 1.2f,-1.0f,cos(glfwGetTime()) * 1.5f) - programState->ufoPosition);
        saturnShader.setFloat("ufoLight.cutOff", ufoSpotLight.cutoff);
        saturnShader.setFloat("ufoLight.outerCutOff", ufoSpotLight.outerCutOff);
        saturnShader.setFloat("ufoLight.constant", ufoSpotLight.constant);
        saturnShader.setFloat("ufoLight.linear", ufoSpotLight.linear);
        saturnShader.setFloat("ufoLight.quadratic", ufoSpotLight.quadratic);

        saturnShader.setVec3("viewPosition", programState->camera.Position);
        saturnShader.setFloat("material.shininess", 32.0f);
        saturnShader.setFloat("material.specular", 0.05f);
        saturnShader.setMat4("projection", projection);
        saturnShader.setMat4("view", view);

        // render the saturn model
        model = glm::mat4(1.0f);
        model = glm::translate(model,programState->saturnPosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->saturnScale));    // it's a bit too big for our scene, so scale it down
        saturnShader.setMat4("model", model);
        saturnModel.Draw(saturnShader);

        // render the house model
        model = glm::mat4(1.0f);
        model = glm::translate(model,programState->housePosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->houseScale));    // it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(-12.0f), glm::vec3(0.0, 0.0, 1.0));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0, 1.0, 0.0));
        saturnShader.setMat4("model", model);
        houseModel.Draw(saturnShader);

        // render mushroom model
        model = glm::mat4(1.0f);
        model = glm::translate(model,programState->mushroomPosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->mushroomScale));    // it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0, 0.0, 1.0));
        saturnShader.setMat4("model", model);
        mushroomModel.Draw(saturnShader);


        // draw skyboxa
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        bool horizontal = true, first_iteration = true;
        int amount = 10;
        blurShader.use();
        for (int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);



        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloomShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        bloomShader.setBool("bloom", bloom);
        bloomShader.setFloat("exposure", exposure);
        renderQuad();

        if (programState->ImGuiEnabled)
            DrawImGui();



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(UP, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (exposure >0.0f)
            exposure -= 0.03f;
        else
            exposure = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        exposure += 0.03f;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Camera info");
    const Camera& c = programState->camera;
    ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
    ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
    ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
    ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            programState->CameraMouseMovementUpdateEnabled = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        hdr = !hdr;
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        bloom = !bloom;
    }
}

unsigned int loadCubemap(vector<std::string> &faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}