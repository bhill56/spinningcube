
#include <iostream>
#include <chrono>
#include <algorithm>

#include <cpplocate/cpplocate.h>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/transform.hpp>

#include <glbinding/gl/gl.h>
#include <glbinding/Version.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/types_to_string.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <globjects/globjects.h>
#include <globjects/base/File.h>
#include <globjects/logging.h>

#include <globjects/Uniform.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/TransformFeedback.h>

#include <string>

#include <cpplocate/cpplocate.h>

#include <globjects/globjects.h>


namespace common
{

std::string determineDataPath()
{
    std::string path = cpplocate::locatePath("data/transformfeedback", "share/globjects/transformfeedback", reinterpret_cast<void *>(&globjects::detachAllObjects));
    if (path.empty()) path = "./data";
    else              path = path + "/data";

    return path;
}

}

using namespace gl;


namespace 
{
    std::unique_ptr<globjects::VertexArray> g_vao = nullptr;
    std::unique_ptr<globjects::Program> g_transformFeedbackProgram = nullptr;
    std::unique_ptr<globjects::TransformFeedback> g_transformFeedback = nullptr;
    std::unique_ptr<globjects::Program> g_shaderProgram = nullptr;

    std::unique_ptr<globjects::File> g_feedbackShaderSource = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> g_feedbackShaderTemplate = nullptr;
    std::unique_ptr<globjects::Shader> g_feedbackShader = nullptr;

    std::unique_ptr<globjects::File> g_vertexShaderSource = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> g_vertexShaderTemplate = nullptr;
    std::unique_ptr<globjects::Shader> g_vertexShader = nullptr;
    std::unique_ptr<globjects::File> g_fragmentShaderSource = nullptr;
    std::unique_ptr<globjects::AbstractStringSource> g_fragmentShaderTemplate = nullptr;
    std::unique_ptr<globjects::Shader> g_fragmentShader = nullptr;

    std::unique_ptr<globjects::Buffer> g_vertexBuffer1 = nullptr;
    std::unique_ptr<globjects::Buffer> g_vertexBuffer2 = nullptr;
    std::unique_ptr<globjects::Buffer> g_colorBuffer = nullptr;

    std::chrono::high_resolution_clock::time_point g_startTime;

    auto g_size = glm::ivec2{};
}

void initialize()
{
    const auto dataPath = common::determineDataPath();

    g_shaderProgram = globjects::Program::create();

    g_vertexShaderSource = globjects::Shader::sourceFromFile(dataPath + "/transformfeedback/simple.vert");
    g_vertexShaderTemplate = globjects::Shader::applyGlobalReplacements(g_vertexShaderSource.get());
    g_vertexShader = globjects::Shader::create(GL_VERTEX_SHADER, g_vertexShaderTemplate.get());

    g_fragmentShaderSource = globjects::Shader::sourceFromFile(dataPath + "/transformfeedback/simple.frag");
    g_fragmentShaderTemplate = globjects::Shader::applyGlobalReplacements(g_fragmentShaderSource.get());
    g_fragmentShader = globjects::Shader::create(GL_FRAGMENT_SHADER, g_fragmentShaderTemplate.get());

    g_shaderProgram->attach(g_vertexShader.get(), g_fragmentShader.get());

    g_transformFeedbackProgram = globjects::Program::create();

    g_feedbackShaderSource = globjects::Shader::sourceFromFile(dataPath + "/transformfeedback/transformfeedback.vert");
    g_feedbackShaderTemplate = globjects::Shader::applyGlobalReplacements(g_feedbackShaderSource.get());
    g_feedbackShader = globjects::Shader::create(GL_VERTEX_SHADER, g_feedbackShaderTemplate.get());

    g_transformFeedbackProgram->attach(g_feedbackShader.get());

    g_transformFeedbackProgram->setUniform("deltaT", 0.0f);

    g_shaderProgram->setUniform("modelView", glm::mat4(1.0f));

    // Read about orthographic projections! Arguments are the left/right/bottom/top
    // of the scene and it returns a matrix that will project stuff to NDC
    //-> NDC = "normalized device coordinates"
    //orthographic projections will shade depending on which "face" is foremost
    //ortho(left, right, bottom, top, near, far)
    //employs foreshortening function to clip cube depending on perspective 
    g_shaderProgram->setUniform("projection", glm::ortho(-0.4f, 1.4f, -0.4f, 1.4f, 0.f, 1.f));

/*
    // Create and setup geometry
    //This includes redundant edge declarations
    auto vertexArray = std::vector<glm::vec4>({
        { 0, 0, 0, 1 } //front face
      , { 1, 0, 0, 1 }
      , { 0, 1, 0, 1 }
      , { 1, 0, 0, 1 }
      , { 0, 1, 0, 1 }
      , { 1, 1, 0, 1 } //end front face

      , { 1, 0, 0, 1 } //right face, lower triangle
      , { 1, 0, 1, 1 }
      , { 1, 1, 0, 1 }
      , { 1, 0, 0, 1 }//right face, upper triangle
      , { 1, 1, 0, 1 }
      , { 1, 1, 1, 1 }

      , { 0, 0, 1, 1 } //left face, lower triangle
      , { 0, 0, 0, 1 }
      , { 0, 1, 1, 1 }
      , { 0, 0, 0, 1 } //left face, upper triangle
      , { 0, 1, 1, 1 }
      , { 0, 1, 0, 1 }

      , { 0, 1, 0, 1 } //top face, lower triangle
      , { 1, 1, 0, 1 }
      , { 0, 1, 1, 1 }
      , { 1, 1, 0, 1 } //top face, upper triangle
      , { 0, 1, 1, 1 }
      , { 1, 1, 1, 1 }

      , { 0, 0, 1, 1 } //bottom face, lower triangle
      , { 1, 0, 1, 1 }
      , { 0, 0, 0, 1 }
      , { 1, 0, 1, 1 } //bottom face, upper triangle
      , { 0, 0, 0, 1 }
      , { 1, 0, 0, 1 }

      , { 0, 0, 1, 1 } //back face, lower triangle
      , { 1, 0, 1, 1 }
      , { 0, 1, 1, 1 }
      , { 1, 0, 1, 1 }
      , { 0, 1, 1, 1 }
      , { 1, 1, 1, 1 } }); 

*/

    //This version of the vertex declaration eliminates redundancy
    auto vertices = std::vector<glm::vec4>({
        { 0, 0, 0, 1 } //bottom left front
      , { 1, 0, 0, 1 } //bottom right front 
      , { 0, 1, 0, 1 } //top left front
      , { 1, 1, 0, 1 } //top right front
      , { 0, 0, 1, 1 } //bottom left back 
      , { 1, 0, 1, 1 } //bottom right back 
      , { 0, 1, 1, 1 } //top left back 
      , { 1, 1, 1, 1 } //top right back 
    }); 

    //indices of cube from vertices 
    //TODO broken
    //This wants to be an index buffer! 
    //Learn about index array buffers
    int vertex_indices[] = {
        0, 1, 1, //front
        1, 2, 3,
        1, 5, 3, //right side
        5, 3, 7, 
        4, 0, 6,  //left side 
        0, 6, 2, 
        2, 3, 6, //top 
        3, 6, 7, 
        0, 1, 4, //bottom 
        1, 4, 5, 
        4, 5, 6, 
        5, 6, 7
    };

        



    //color vector is R, G, B, opacity
    auto colorArray = std::vector<glm::vec4>({
        { 0, 0, 1, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 0, 1, .5 } 
      , { 0, 0, 1, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 0, 1, .5 } 
      , { 0, 0, 1, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 0, 1, .5 } 
      , { 0, 0, 1, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 0, 1, .5 } 
      , { 0, 0, 1, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 0, 1, .5 } 
      , { 0, 0, 1, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 1, 0, .5 }
      , { 0, 0, 1, .5 } });

    //Create the Element Buffer Object
    //Bind the EBO buffer, then set the buffer data for the current bound buffer
    //to the indices that outline the triangles
    auto EBO = globjects::Buffer::create(); 
    EBO->setData(vertex_indices, GL_STATIC_DRAW);

       //GL_STATIC_DRAW: the data is set once and used by the GPU many times
    g_vertexBuffer1 = globjects::Buffer::create();
    g_vertexBuffer1->setData(vertices, GL_STATIC_DRAW);
    g_vertexBuffer2 = globjects::Buffer::create();
    g_vertexBuffer2->setData(vertices, GL_STATIC_DRAW);
    g_colorBuffer = globjects::Buffer::create();
    g_colorBuffer->setData(colorArray, GL_STATIC_DRAW);

    g_vao = globjects::VertexArray::create();

    g_vao->binding(0)->setAttribute(0);
    g_vao->binding(0)->setFormat(4, GL_FLOAT);
    g_vao->binding(0)->setBuffer(g_vertexBuffer1.get(), 0, sizeof(glm::vec4));
    g_vao->enable(0);

    g_vao->binding(1)->setAttribute(1);
    g_vao->binding(1)->setBuffer(g_colorBuffer.get(), 0, sizeof(glm::vec4));
    g_vao->binding(1)->setFormat(4, GL_FLOAT);
    g_vao->enable(1);
 
    g_vao->binding(2)->setAttribute(2);
    g_vao->binding(2)->setFormat(4, GL_FLOAT);
    g_vao->binding(2)->setBuffer(EBO.get(), 0, 2 * sizeof(glm::vec4));
    g_vao->enable(2);



    // Create and setup TransformFeedback
    g_transformFeedback = globjects::TransformFeedback::create();
    g_transformFeedback->setVaryings(g_transformFeedbackProgram.get(), { { "next_position" } }, GL_INTERLEAVED_ATTRIBS);
    g_transformFeedback->unbind();


    g_startTime = std::chrono::high_resolution_clock::now();
}

void deinitialize()
{
    g_vao.reset(nullptr);

    g_transformFeedback.reset(nullptr);
    g_transformFeedbackProgram.reset(nullptr);
    g_shaderProgram.reset(nullptr);

    g_feedbackShader.reset(nullptr);
    g_feedbackShaderTemplate.reset(nullptr);
    g_feedbackShaderSource.reset(nullptr);

    g_vertexShader.reset(nullptr);
    g_vertexShaderTemplate.reset(nullptr);
    g_vertexShaderSource.reset(nullptr);
    g_fragmentShader.reset(nullptr);
    g_fragmentShaderTemplate.reset(nullptr);
    g_fragmentShaderSource.reset(nullptr);

    g_vertexBuffer1.reset(nullptr);
    g_vertexBuffer2.reset(nullptr);
    g_colorBuffer.reset(nullptr);
}

void draw()
{
    const auto t_elapsed = std::chrono::high_resolution_clock::now() - g_startTime;
    g_startTime = std::chrono::high_resolution_clock::now();

    const auto drawBuffer  = g_vertexBuffer1.get();
    const auto writeBuffer = g_vertexBuffer2.get();

    // RECORD / PROCESS

    // What does it mean to bind a vertex array object?
    //Fundamental to drawing an object: 
    //configure vertex attribute pointers once and then bind them to whatever object
    g_vao->bind();

    // What does it mean to set a uniform buffer?
    //Allows us to declare and define global uniform variables
    g_transformFeedbackProgram->setUniform("deltaT", static_cast<float>(t_elapsed.count()) * float(std::nano::num) / float(std::nano::den));

    g_vao->binding(0)->setBuffer(drawBuffer, 0, sizeof(glm::vec4));

    g_transformFeedback->bind();
    writeBuffer->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    // What does it mean to discard the rasterizer?
    glEnable(GL_RASTERIZER_DISCARD);

    // You can only use one shader program a time, hence "use"
    g_transformFeedbackProgram->use();

    g_transformFeedback->begin(GL_TRIANGLES);
    // Why is "drawArrays" being called in transformFeedback?
    //->set the right number of vertices, so since this is a cube we need 
    //2 triangles for 6 faces, so 12 triangles each with 4 values
    // How do we bind to the EBO so that we can perform indexed rendering?
    g_vao->drawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    g_transformFeedback->end();

    // Why is this being disabled?
    //If it was not disabled, primitives would be discarded before the rasterizing phase 
    //Since it's disabled, primitives are passed through to the rasterizing phase to be processed
    glDisable(GL_RASTERIZER_DISCARD);

    // writeBuffer->unbindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0); // not implemented?
    g_transformFeedback->unbind();

    // DRAW

    // g_vao->bind();
    g_vao->binding(0)->setBuffer(writeBuffer, 0, sizeof(glm::vec4));

    const auto side = std::min<int>(g_size.x, g_size.y);

    // Why is glViewport being set here?
    //This specifies how NDC coordinates are mapped to framebuffer coordinates which
    //have just been set
    glViewport((g_size.x - side) / 2, (g_size.y - side) / 2, side, side);

    // What happens when glClear is set?
    //Takes bitwise OR which indicates which buffer is to be cleared 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g_shaderProgram->use();
    g_transformFeedback->draw(GL_TRIANGLES);
    g_shaderProgram->release();

    g_vao->unbind();

    std::swap(g_vertexBuffer1, g_vertexBuffer2);
}


void error(int errnum, const char * errmsg)
{
    globjects::critical() << errnum << ": " << errmsg << std::endl;
}

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height)
{
    g_size = glm::ivec2{ width, height };
}

void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*modes*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_F5 && action == GLFW_RELEASE)
    {
        g_feedbackShaderSource->reload();
        g_vertexShaderSource->reload();
        g_fragmentShaderSource->reload();
    }
}


int main(int /*argc*/, char * /*argv*/[])
{
    // Initialize GLFW
    if (!glfwInit())
        return 1;

    glfwSetErrorCallback(error);

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, true);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#ifdef SYSTEM_DARWIN
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create a context and, if valid, make it current
    GLFWwindow * window = glfwCreateWindow(640, 480, "globjects Transform Feedback", nullptr, nullptr);
    if (window == nullptr)
    {
        globjects::critical() << "Context creation failed. Terminate execution.";

        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);

    // Initialize globjects (internally initializes glbinding, and registers the current context)
    globjects::init([](const char * name) {
        return glfwGetProcAddress(name);
    });

    std::cout << std::endl
        << "OpenGL Version:  " << glbinding::aux::ContextInfo::version() << std::endl
        << "OpenGL Vendor:   " << glbinding::aux::ContextInfo::vendor() << std::endl
        << "OpenGL Renderer: " << glbinding::aux::ContextInfo::renderer() << std::endl << std::endl;

    globjects::info() << "Press F5 to reload shaders." << std::endl << std::endl;

    glfwGetFramebufferSize(window, &g_size[0], &g_size[1]);
    initialize();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        draw();
        glfwSwapBuffers(window);
    }
    deinitialize();

    // Properly shutdown GLFW
    glfwTerminate();

    return 0;
}