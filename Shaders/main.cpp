/*!*****************************************************************************
\file main.cpp
\author Vadim Surov (vsurov@digipen.edu)
\co-author Ban Kai Wei Benjamin (k.ban@digipen.edu)
\par Course: CSD2150
\par Programming Assignment 11: 
\date 03/20/2026 (MM/DD/YYYY)
\brief Application entry point: initializes GLFW/Windows, constructs demo windows
       and rendering passes, registers callbacks, and runs the render loop.

       Assignment 2 Ray Casting,  WASD to move left click, drag mse to rotate.
       Assignment 3 Ray Tracing, no controls
       Assignment 4 Blinn-Phong Shading, mouuse scroll and mouse drag
       Assignment 5 Envrionment Mapping , mouse scroll to zoom and LMB to move
       Assignment 8 Night Sky with PBR,cartoon and fog and discard
                    mouse scroll to zoom and LMB to move lights/camera
                    // W and O to switch modes
       Assignment 9 Cartoon
                    Obit Obj
                    Orbit Light
                    Blur Toggle
                    Edge Toggle
                    Edge threshold adj
                    alpha factor adj
    Assignment 10 Portal
    Assignment 11 Ogre
                    Option 1,2,3
                    1. Ogre with Gpass
                    2. Light Dir
                    3. View Dir

*******************************************************************************/
#include <iostream> // input/output
#include "windowmanager.h"
#include "trianglemeshes.h"

//


//#include "object.h"
const int MARGIN = 2; // Defines margin size for windows
const int NOMARGIN = 0;      // To set no margin for a window
constexpr int W = 200;
constexpr int H = 120;
constexpr int GAP = 10;

//assignment 1 stuff
enum class Tabs
{
    ANIMATION = 0,
    COLOR = 1,
    MOUSE = 2
};

inline std::ostream& operator<<(std::ostream& os, Tabs tab)
{
    switch (tab)
    {
    case Tabs::ANIMATION:
        return os << "Animation";
    case Tabs::COLOR:
        return os << "Color";
    case Tabs::MOUSE:
        return os << "Mouse";
    default:
        return os << "Unknown Tab Error";
    }
}

std::vector<GLFWwindow*> tab[3];

void ShowSelectedTab(int currentTab)
{
    for (auto* window : tab[0])
        (currentTab == 0) ? glfwShowWindow(window) : glfwHideWindow(window);

    for (auto* window : tab[1])
        (currentTab == 1) ? glfwShowWindow(window) : glfwHideWindow(window);

    for (auto* window : tab[2])
        (currentTab == 2) ? glfwShowWindow(window) : glfwHideWindow(window);
}


bool inCircle(float mx, float my, float cx, float cy, float r)
{
    // distance of x and distance of y from mouse to center
    float dx = mx - cx, dy = my - cy;
    // squared distance
    return dx * dx + dy * dy <= r * r;
}
// Tabs
static Tabs current_tab = Tabs::ANIMATION;
static const int max_tab_count = 3;
const float r = 10.0f;
const float spacing = 30.0f;
const float y_height = 30.0f;
//end of ass1

//Assignment2 RayCasting demo
namespace RayCasting
{
    // Map dimensions
    const int MAP_WIDTH = 16;
    const int MAP_HEIGHT = 16;

    int worldMap[MAP_WIDTH][MAP_HEIGHT] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,2,2,2,0,0,3,3,3,0,0,0,1},
        {1,0,0,0,2,0,0,0,0,0,0,3,0,0,0,1},
        {1,0,0,0,2,0,0,0,0,0,0,3,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,5,0,0,0,0,0,0,4,0,0,0,1},
        {1,0,0,0,5,0,0,0,0,0,0,4,0,0,0,1},
        {1,0,0,0,5,5,5,0,0,4,4,4,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    // Create or resize the buffers when the window size changes
    void resizeBuffers(cg::ManagedWindow* pWindow)
    {
        if (!pWindow || !pWindow->pWindow)
            return;

        // Make the window's OpenGL context current before touching GL objects.
        glfwMakeContextCurrent(pWindow->pWindow);

        // Ensure SSBO exists for ray distances; create if not
        if (pWindow->context.gluintparam[0] == 0) // Distance SSBO
            glGenBuffers(1, &pWindow->context.gluintparam[0]);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pWindow->context.gluintparam[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
            static_cast<GLsizeiptr>(pWindow->width * sizeof(float)),
            nullptr, GL_DYNAMIC_DRAW);

        // Ensure SSBO exists for ray texture coordinates; create if not
        if (pWindow->context.gluintparam[1] == 0) // Texture coordinates SSBO
            glGenBuffers(1, &pWindow->context.gluintparam[1]);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pWindow->context.gluintparam[1]);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
            static_cast<GLsizeiptr>(pWindow->width * sizeof(float)),
            nullptr, GL_DYNAMIC_DRAW);

        // Ensure SSBO exists for ray texture ids; create if not
        if (pWindow->context.gluintparam[2] == 0) // Texture ids SSBO
            glGenBuffers(1, &pWindow->context.gluintparam[2]);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pWindow->context.gluintparam[2]);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
            static_cast<GLsizeiptr>(pWindow->width * sizeof(int)),
            nullptr, GL_DYNAMIC_DRAW);

        // Create SSBO for the map
        if (pWindow->context.gluintparam[3] == 0)  // Map SSBO
            glGenBuffers(1, &pWindow->context.gluintparam[3]);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pWindow->context.gluintparam[3]);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
            MAP_WIDTH * MAP_HEIGHT * sizeof(int), worldMap, GL_STATIC_DRAW);

        // Unbind to avoid accidental state leaks
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        //std::cout << "Map SSBO = " << pWindow->context.gluintparam[3] << "\n";
    }

    // Compute distances using ray casting
    void computeDistances(cg::ManagedWindow* pWindow)
    {
        if (!pWindow || !pWindow->pWindow)
            return;

        if (pWindow->width != pWindow->context.intparam[0])
        {
            resizeBuffers(pWindow);
            pWindow->context.intparam[0] = pWindow->width;
        }

        // Ensure the correct OpenGL context is current. Dispatching compute work requires
        // the program to be created/used on the same current context.
        glfwMakeContextCurrent(pWindow->pWindow);

        // Use the compute program (now guaranteed to be built on the current context)
        pWindow->computeShader.use();
        pWindow->computeShader.setUniform("playerPos", pWindow->context.vec2param[0]);
        pWindow->computeShader.setUniform("playerDir", pWindow->context.vec2param[1]);
        pWindow->computeShader.setUniform("mapWidth", MAP_WIDTH);
        pWindow->computeShader.setUniform("mapHeight", MAP_HEIGHT);
        pWindow->computeShader.setUniform("numRays", pWindow->width);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pWindow->context.gluintparam[0]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pWindow->context.gluintparam[1]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pWindow->context.gluintparam[2]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pWindow->context.gluintparam[3]);

        // Dispatch compute threads (num work groups = window width; shader should use local size accordingly)
        glDispatchCompute(pWindow->width, 1, 1);

        // Ensure writes are visible to subsequent stages
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        pWindow->computeShader.disuse();
    }
}//end of namespace Raycasting

/*
* ass5
*/
static int dynamic_option = 1;

/*
* ASS8 stuff
*/
#define FOG_COLOR { 1.0f, 1.0f, 1.0f, 1.0f }
/*
* ASS9 stuff
*/
GLuint texture0 = 0; //ping
GLuint texture1 = 0; //pong
GLuint texture2 = 0; //edge
GLuint texture3 = 0; //cartoon
GLuint texture4 = 0;
//GLuint texture5 = 0;
//static int ShadingOption = 2; // default inverse
//static int FilterOption = 2; // default inverse


static bool BlurFilter = true; // 
static bool EdgeFilter = true; // 

//static GLint gOptionLoc = -1; // cache uniform location

static float EdgeThreshold = 0.15f;
static float AlphaFactor = 1.f;



/*
* Assignment 10
*/
GLuint textureA = 0; //for portal
//glm::vec3 worldPos{};
static bool PortalPlaced = false;
static glm::mat4 PortalMatrix = glm::mat4(1.0f);

static glm::mat4 BuildPortalMatrix(glm::vec3 target, glm::vec3 eye)
{
    // fixed projector in world, eye/projectorPos

    glm::mat4 view = glm::lookAt(
        eye,
        target,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );


    glm::mat4 proj = glm::perspective(
        glm::radians(30.0f),
        1.0f,
        0.2f,
        1000.0f
    );

    glm::mat4 bias(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
    );

    return bias * proj * view;
}
/*
* a11
*/
#define ANINSTANCE(x, z) { MATERIAL_OBJECT, glm::translate(glm::mat4(1.0f), {x, 0.0f, z}) }

#define INSTANCES(x)\
                            ANINSTANCE(x, -20.0f-5.0f),\
                            ANINSTANCE(x, -18.0f-5.0f),\
                            ANINSTANCE(x, -16.0f-5.0f),\
                            ANINSTANCE(x, -14.0f-5.0f),\
                            ANINSTANCE(x, -12.0f-5.0f),\
                            ANINSTANCE(x, -10.0f-5.0f),\
                            ANINSTANCE(x,  -8.0f-5.0f),\
                            ANINSTANCE(x,  -6.0f-5.0f),\
                            ANINSTANCE(x,  -4.0f-5.0f),\
                            ANINSTANCE(x,  -2.0f-5.0f),\
                            ANINSTANCE(x,   0.0f-5.0f),\
                            ANINSTANCE(x,   2.0f-5.0f),\
                            ANINSTANCE(x,   4.0f-5.0f),\
                            ANINSTANCE(x,   6.0f-5.0f),\
                            ANINSTANCE(x,   8.0f-5.0f),\
                            ANINSTANCE(x,  10.0f-5.0f),\
                            ANINSTANCE(x,  12.0f-5.0f),\
                            ANINSTANCE(x,  14.0f-5.0f),\
                            ANINSTANCE(x,  16.0f-5.0f),\
                            ANINSTANCE(x,  18.0f-5.0f),\
                            ANINSTANCE(x,  20.0f-5.0f)
static int TextureMode = 1;
glm::vec4 BG_color = { 0.5f, 0.5f, 0.5f, 1.0f };
//GLuint g0 = 0, g1 = 0, g2 = 0, g3 = 0;
//===================MAIN===================================================================================
// Entry point of the application
int main(int argc, char** argv)
{
    int retcode = 0; // Return code for the application
    int I = 0;
    // Create a window manager instance
    auto& windowManager = cg::WindowManager::getInstance();

    try
    {
        // Initialize GLFW
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW.");

        // Debug context setup for OpenGL if in debug mode
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

        // Configure OpenGL context properties
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        GLFWwindow* pMainWindow = nullptr; // Pointer to the main window

        // Add the main window with its properties
        pMainWindow = windowManager.addWindow({
            cg::State::SHOW,        // State (SHOW, HIDE)
            nullptr,                // Parent window (nullptr for top-level window)
            DEFAULT,                // Viewport (DEFAULT, {GAP}, {WIDTH, HEIGHT}, 
            //   {GRIDELEMENT, SIZEOFGRID, GAP}, {X, Y, WIDTH, HEIGHT})
"CSD2151 Assignment 12 (Portfolio) - Ban Kai Wei Benjamin (k.ban)",    // Title
NONE,                   // Background color (NONE, BLACK, GREY, WHITE, RED, GREEN, 
//   BLUE, {R,G,B,A})
NONE,                   // Textures ({filename})
NONE,                   // Compute shader
{                       // Rendering passes
    {                       // Pass 0
        { NOMARGIN },           // Viewport
        NONE,                   // Background color
        DISABLE,                // Depth test
        { { QUAD } },           // Objects
        NONE,                   // Cameras
        NONE,                   // Lights
        DEFAULT,                // Rendering target
        {                       // Vertex and fragment shaders
            #include "Shaders/Shader.vert.glsl"
            ,
            #include "Shaders/Shader.frag.glsl"  
        }
    }
},

// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused)
    {
        std::cout << std::endl;
        std::cout << "Shaders Application (version "
            << __DATE__ << " " << __TIME__ << ")";
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
                   
    The instructor provided the base graphics framework, such as the rendering pipeline scaffolding, 
    shader integration, window/pass setup, and supporting infrastructure. 
    My contribution focused on implementing and modifying the assignment-specific rendering techniques,
    shader logic, and effects within that framework to produce the results shown in the Window
    
    This application showcases the capabilities of modern real-time 2D and
    3D computer graphics used in interactive simulations and games.
    It leverages the power of your graphics card and advanced shader
    technology to deliver smooth, visually rich experiences.
    The demo illustrates how multiple independent graphical elements can
    be rendered and interacted with using intuitive keyboard and mouse controls.
Controls:
    - Switch Tab: Press Tab to cycle through tabs or click on the circles.
    - Resize: Drag the window edges to adjust the application size.
    
    - Select: Click on any demo window to focus it and set it as active.
    - Open: Press Enter or double-click to launch the selected demo in a new window.
    - Save Screenshot: Press 'P' to save a screenshot of the active window.
    - Exit: Press Esc to close the application.
)";
                }
},

            // InitCallback - called for initializing shaders across
            // the entire rendering process or individual passes when required
            [](cg::ManagedWindow* pWindow, int pass)
            {
                                if (pass == -1)
                { // Initial setup for the entire rendering process
                    // std::cout << "pass: " << pass << "\n";//prints -1
                    pWindow->passes[0].renderShader.use();
                    pWindow->passes[0].renderShader.setUniform("uTab", 0);
                    pWindow->passes[0].renderShader.setUniform("radius", r);
                    pWindow->passes[0].renderShader.setUniform("spacing", spacing);
                    pWindow->passes[0].renderShader.setUniform("y_height", y_height);
                    pWindow->passes[0].renderShader.setUniform(
                        "uCursor",
                        glm::vec2(pWindow->width * 0.5f, pWindow->height * 0.5f));
                }
            },

            // SizeCallback - called when the window is resized
            [](cg::ManagedWindow* pWindow, int width, int height)
            {
                cg::WindowManager::getInstance().resizeChildren(pWindow->pWindow, width, height);
            },

            // KeyCallback - called when a key is pressed
            [](cg::ManagedWindow* pWindow, int key)
            {
                // change tab
                if (key == GLFW_KEY_TAB)
                {
                    std::cout << "Tab Key pressed\n";
                    current_tab = static_cast<Tabs>(
                        (static_cast<int>(current_tab) + 1) % max_tab_count);

                    std::cout << "[Changing Tabs] using TAB key, Switched to tab: " << current_tab << "\n";

                    // Upload from CPU to GPU
                    pWindow->passes[0].renderShader.use();
                    pWindow->passes[0].renderShader.setUniform("uTab", static_cast<int>(current_tab));
                    ShowSelectedTab(static_cast<int>(current_tab));
                    return false; // Do not propagate the event
                } // end of key tab

                return true; // Propagate the event by default
            },

            // MousebuttonCallback - called when a mouse button is pressed
            [](cg::ManagedWindow* pWindow, int button, int xpos, int ypos)
            {

            },

            // ScrollCallback - called when the mouse wheel is scrolled
            [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
            {
            },

            // CursorposCallback - called when the mouse is moved
            [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
            {
                pWindow->passes[0].renderShader.use();
                pWindow->passes[0].renderShader.setUniform(
                    "uCursor",
                    glm::vec2((float)xpos, (float)(pWindow->height - ypos))
                );
            },
            //MouseclickCallback - called when a mouse btm is single clicked
            [](cg::ManagedWindow* pWindow, int xpos, int ypos) {
                pWindow->passes[0].renderShader.use();

                    float cx = pWindow->width * 0.5f;
                    float mx = (float)xpos;
                    float my = (float)(pWindow->height - ypos); // flipped

                    if (inCircle(mx, my, cx - spacing, y_height, r))
                    {
                        current_tab = Tabs::ANIMATION;
                        std::cout << "[Changing Tabs] using LMB, Switched to tab :" << current_tab << "\n";
                    }
                    else if (inCircle(mx, my, cx, y_height, r))
                    {
                        current_tab = Tabs::COLOR;
                        std::cout << "[Changing Tabs] using LMB, Switched to tab :" << current_tab << "\n";
                    }
                    else if (inCircle(mx, my, cx + spacing, y_height, r))
                    {
                        current_tab = Tabs::MOUSE;
                        std::cout << "[Changing Tabs] using LMB, Switched to tab :" << current_tab << "\n";
                    }
                    pWindow->passes[0].renderShader.setUniform("uTab", static_cast<int>(current_tab));

                    ShowSelectedTab(static_cast<int>(current_tab));
                
            },
            [](cg::ManagedWindow* pWindow, int xpos, int ypos) { //MousedoubleclickCallback
                //cg::WindowManager::getInstance().copyWindow(*pWindow);
            }


            });
        //=================================Assignmnet 1========================================
                    // Add the main window with its properties
            tab[0].push_back(windowManager.addWindow({
                cg::State::SHOW,        // State (SHOW, HIDE)
                pMainWindow,                // Parent window (nullptr for top-level window)
                { 0, 9, GAP },                // Viewport (DEFAULT, {GAP}, {WIDTH, HEIGHT}, 
                //   {GRIDELEMENT, SIZEOFGRID, GAP}, {X, Y, WIDTH, HEIGHT})
                "Assignment 1 (tabs) - k.ban",    // Title
                NONE,                   // Background color (NONE, BLACK, GREY, WHITE, RED, GREEN, 
                //   BLUE, {R,G,B,A})
                NONE,                   // Textures ({filename})
                NONE,                   // Compute shader
                {                       // Rendering passes
                    {                       // Pass 0
                        { MARGIN  },           // Viewport
                        NONE,                   // Background color
                        DISABLE,                // Depth test
                        { { QUAD } },           // Objects
                        NONE,                   // Cameras
                        NONE,                   // Lights
                        DEFAULT,                // Rendering target
                        {                       // Vertex and fragment shaders
                            #include "Shaders/a1.vert.glsl"
                            ,
                            #include "Shaders/a1.frag.glsl"  
                        }
                    }
                },

// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }

    if (focused)
    {
        std::cout << std::endl << R"(
        Assignment 1 (Tabs)
            This demo implements a tabbed interface within a single window using
            custom shaders. Each tab displays different content, and users can
            switch between tabs using mouse clicks or keyboard shortcuts.
        Controls:
            - Switch Tab: Press keys 1-3 to switch tabs or click on the circles.
            - Resize: Drag the window edges to adjust the application size.
            
            - Save Screenshot: Press 'P' to save a screenshot of the active window.
            - Exit: Press Esc to close the application.
        )";
    }
},

                // InitCallback - called for initializing shaders across
                // the entire rendering process or individual passes when required
                [](cg::ManagedWindow* pWindow, int pass)
                {
                                    if (pass == -1)
                    { // Initial setup for the entire rendering process
                        // std::cout << "pass: " << pass << "\n";//prints -1
                        pWindow->passes[0].renderShader.use();
                        pWindow->passes[0].renderShader.setUniform("uTab", 0);
                        pWindow->passes[0].renderShader.setUniform("radius", r);
                        pWindow->passes[0].renderShader.setUniform("spacing", spacing);
                        pWindow->passes[0].renderShader.setUniform("y_height", y_height);
                        //cg::TriangleMesh::Preload(OGRE);
                    }
                },

                // SizeCallback - called when the window is resized
                [](cg::ManagedWindow* pWindow, int width, int height)
                {
                },

                // KeyCallback - called when a key is pressed
                [](cg::ManagedWindow* pWindow, int key)
                {
                    // change tab
                    if (key == GLFW_KEY_TAB)
                    {
                        std::cout << "Tab Key pressed\n";
                        current_tab = static_cast<Tabs>(
                            (static_cast<int>(current_tab) + 1) % max_tab_count);

                        std::cout << "[Changing Tabs] using TAB key, Switched to tab: " << current_tab << "\n";

                        // Upload from CPU to GPU
                        pWindow->passes[0].renderShader.use();
                        pWindow->passes[0].renderShader.setUniform("uTab", static_cast<int>(current_tab));

                        return false; // Do not propagate the event
                    } // end of key tab

                    return true; // Propagate the event by default
                },

                // MousebuttonCallback - called when a mouse button is pressed
                [](cg::ManagedWindow* pWindow, int button, int xpos, int ypos)
                {
    
                },

                // ScrollCallback - called when the mouse wheel is scrolled
                [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
                {
                },

                // CursorposCallback - called when the mouse is moved
                [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                {
                },
                //MouseclickCallback - called when a mouse btm is single clicked
                [](cg::ManagedWindow* pWindow, int xpos, int ypos) {
                                    pWindow->passes[0].renderShader.use();

                            float cx = pWindow->width * 0.5f;
                            float mx = (float)xpos;
                            float my = (float)(pWindow->height - ypos); // flipped

                            if (inCircle(mx, my, cx - spacing, y_height, r))
                            {
                                current_tab = Tabs::ANIMATION;
                                std::cout << "[Changing Tabs] using LMB, Switched to tab :" << current_tab << "\n";
                            }
                            else if (inCircle(mx, my, cx, y_height, r))
                            {
                                current_tab = Tabs::COLOR;
                                std::cout << "[Changing Tabs] using LMB, Switched to tab :" << current_tab << "\n";
                            }
                            else if (inCircle(mx, my, cx + spacing, y_height, r))
                            {
                                current_tab = Tabs::MOUSE;
                                std::cout << "[Changing Tabs] using LMB, Switched to tab :" << current_tab << "\n";
                            }

                            pWindow->passes[0].renderShader.setUniform("uTab", static_cast<int>(current_tab));
                        
                },
                [](cg::ManagedWindow* pWindow, int xpos, int ypos) { //MousedoubleclickCallback
                    cg::WindowManager::getInstance().copyWindow(*pWindow);
                        }


                }));
                I += W + GAP;
        //==================================Assignment2==========================================
        //	//================================================================================================================================================
        //	// Add the sub window with its properties for assignment2

        if (!GLEW_ARB_compute_shader)
            std::cout << "   Compute shaders not supported!" << std::endl;
        else
        {
            tab[0].push_back(windowManager.addWindow({
                cg::State::SHOW,            // State
                pMainWindow,                // Parent window
                { 1, 9, GAP },   // Viewport
                "Assignment 2 (Ray Casting) - k.ban",              // Title
                NONE,                      // Background color
                NONE,                       // Textures
                {                           // Compute shader
                    #include "Shaders/RayCasting.comp.glsl"
                },
                {                           // Rendering passes
                    {                           // Pass 0
                        { MARGIN  },                 // Viewport
                        NONE,                       // Background color
                        DISABLE,                    // Depth test
                        { { QUAD } },               // Objects
                        NONE,                       // Cameras
                        NONE,                       // Lights
                        DEFAULT,                    // Rendering target
                        {                           // Vertex and fragment shaders
                            #include "Shaders/PassThrough.vert.glsl"
                            ,
                            #include "Shaders/RayCasting.frag.glsl"
                        },
                    }
                },

// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }
    if (focused)
    {
        std::cout << std::endl << R"(
        Assignment 2 (Ray Casting)
            This is an implementation of the Ray Casting method. This rendering method
            serves as the foundation for many classic 'first-person shooter'-style games.
            The scene is defined as a 2D map, and the player's position is tracked within it.
            The GPU is also used to calculate distances to walls during rendering, enabling
            efficient real-time visualization.
        Controls:
            - Move Player: Use W, A, S, and D keys to move.
            - Change Orientation: Drag left or right with the mouse to rotate the view.
        )";
    }
},

                // InitCallback - called for initializing shaders across
                // the entire rendering process or individual passes when required
                [](cg::ManagedWindow* pWindow, int pass)
                {

                    if (pass == -1)
                    {
                       pWindow->context.intparam[0] = 0; // Previous width
                       pWindow->context.vec2param[0] = glm::vec2{ 8.0f, 8.0f };  // Position
                       pWindow->context.vec2param[1] = glm::vec2{ 0.0f, -1.0f }; // Direction

                       // initialize SSBO handles 
                       pWindow->context.gluintparam[0] = 0; // distances SSBO
                       pWindow->context.gluintparam[1] = 0; // coords SSBO
                       pWindow->context.gluintparam[2] = 0; // ids SSBO
                       pWindow->context.gluintparam[3] = 0; // map SSBO


                       //std::cout << "Map SSBO = " << pWindow->context.gluintparam[3] << "\n";
                   }

                   RayCasting::computeDistances(pWindow);
               },

                // SizeCallback - called when the window is resized
                nullptr,

                // KeyCallback - called when a key is pressed
                [](cg::ManagedWindow* pWindow, int key)
                {
                    //wasd key movement based on raycast demo
                    switch (key)
                    {
                    case 'W':
                        pWindow->context.vec2param[0] += pWindow->context.vec2param[1] /
                            glm::vec2(5.0f, 5.0f);
                        return false; // Do not propagate the event

                    case 'A':
                        pWindow->context.vec2param[0] -=
                            glm::vec2(-pWindow->context.vec2param[1].y,
                            pWindow->context.vec2param[1].x) / glm::vec2(5.0f, 5.0f);
                        return false; // Do not propagate the event

                    case 'S':
                        pWindow->context.vec2param[0] -= pWindow->context.vec2param[1] /
                            glm::vec2(5.0f, 5.0f);
                        return false; // Do not propagate the event

                    case 'D':
                        pWindow->context.vec2param[0] +=
                            glm::vec2(-pWindow->context.vec2param[1].y,
                            pWindow->context.vec2param[1].x) / glm::vec2(5.0f, 5.0f);
                        return false; // Do not propagate the event
                    }

                    return true; // Propagate the event by default
                },

                // MousebuttonCallback - called when a mouse button is pressed
                nullptr,

                // ScrollCallback - called when the mouse wheel is scrolled
                nullptr,

                // CursorposCallback - called when the mouse is moved
                [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset,
                        double yoffset, int mods)
                {
                    //raycastdemo
                    // rotate direction by +/-5 degrees based on xoffset
                    if (glm::abs(xoffset) > glm::abs(yoffset))
                    {
                        if (glm::abs(xoffset) > 1.0)
                        {
                            const float angleDeg = (xoffset < 0.0) ? -5.0f : 5.0f;
                            const float angle = glm::radians(angleDeg);
                            const float c = glm::cos(angle);
                            const float s = glm::sin(angle);
                            glm::mat2 rot = glm::mat2(c, -s, s, c);
                            pWindow->context.vec2param[1] = glm::normalize(rot *
                                pWindow->context.vec2param[1]);
                        }
                    }
                },
                //MouseclickCallback - called when a mouse btm is single clicked
                [](cg::ManagedWindow* pWindow, int xpos, int ypos) {

                },
                [](cg::ManagedWindow* pWindow, int xpos, int ypos) { //MousedoubleclickCallback
                    cg::WindowManager::getInstance().copyWindow(*pWindow);
                 }

                }));
        }
        I += W + GAP;
        //==================================Assignment3==========================================



        tab[0].push_back(windowManager.addWindow({
                cg::State::SHOW,            // State
                pMainWindow,                // Parent window
                { 2, 9, GAP },   // Viewport
                "Assignment 3 (Ray Tracing) - k.ban",              // Title
                BLACK,                      // Background color
                NONE,                       // Textures
                NONE,
                // Compute shader
            {                           // Rendering passes
                {                           // Pass 0
                    { MARGIN  },                 // Viewport
                    NONE,                       // Background color
                    DISABLE,                    // Depth test
                    { { QUAD } },               // Objects
                    NONE,                       // Cameras
                    NONE,                       // Lights
                    DEFAULT,                    // Rendering target
                    {                           // Vertex and fragment shaders
                        #include "Shaders/PassThrough.vert.glsl"
                        ,
                        #include "Shaders/RayTracing.frag.glsl"
                    },
                }
            },
// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }
    if (focused)
    {
        std::cout << std::endl << R"(
        Assignment 3 (Ray Tracing)
            This demo illustrates the use of ray tracing to render spheres representing
            the Sun, Earth, and Moon, with the Sun and Moon orbiting around the Earth.
            Shading is computed based on surface normals and the Sun's position,
            producing realistic lighting effects.
        Controls:
            - This demo does not have interactive controls.
            )";
    }
},
                    // InitCallback
                    nullptr,
                    // SizeCallback
                    nullptr,
                    // KeyCallback
                    nullptr,
                    // MousebuttonCallback
                    nullptr,
                    // ScrollCallback
                    nullptr,
                    // CursorposCallback
                    nullptr,
                    // MouseclickCallback
                    nullptr,
                    // MousedoubleclickCallback
                    [](cg::ManagedWindow* pWindow, int xpos, int ypos)
                    {
                        cg::WindowManager::getInstance().copyWindow(*pWindow);
                    }
            }));
        I = 0;
        //==============================ASSIGNMENT 4=============================================
        //=======================================================================================
        tab[0].push_back(windowManager.addWindow({
                cg::State::SHOW,            // State
                pMainWindow,                // Parent window
                { 3, 9, GAP },   // Viewport
                "Assignment 4 (Blinn-Phong Shading) - k.ban",              // Title
                BLACK,                      // Background color, window UI
                NONE,                       // Textures
                NONE,
                // Compute shader
                {                           // Rendering passes
                    {                           // Pass 0
                        { MARGIN  },                 // Viewport
                        BLACK,                       // Background color, 3D rebder ckear
                        ENABLE,                    // Depth test, need enable so can cull
                        { //QUAD,
                            {
                            CUBE,
                            PHONG_MATERIAL_ORANGE,
                            NOTRANSFORM,
                            VISIBLE
                            },
                            {
                            TORUS,
                            PHONG_MATERIAL_ORANGE,
                            NOTRANSFORM,
                            VISIBLE
                            },
                            {
                            SPHERE,
                            PHONG_MATERIAL_ORANGE,
                            NOTRANSFORM,
                            VISIBLE
                            },
                            {
                            TEAPOT,
                            PHONG_MATERIAL_ORANGE,
                            NOTRANSFORM,
                            VISIBLE
                            },
                            {
                            PLANE,
                            PHONG_MATERIAL_BLUE,
                            NOTRANSFORM,
                            VISIBLE
                            },


                        },  // Objects                        
                            { {cg::CameraType::ORBITING, glm::vec3(3.6f,5.6f,17.7f)}},                       // Cameras
                            {glm::vec3(0.f,5.f,0.f)},                                                       // Lights
                            DEFAULT,                    // Rendering target
                            {                           // Vertex and fragment shaders
                                #include "Shaders/ViewSpaceTransform.vert.glsl"
                                ,
                                #include "Shaders/BlinnPhong.frag.glsl"
                            },
                        }
                },

// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }
    if (focused)
    {
        std::cout << std::endl << R"(
        Assignment 4 (Blinn-Phong Shading)
            This demo showcases the Blinn-Phong shading technique, which produces smooth,
            realistic surfaces by interpolating vertex normals per pixel and applying
            a lighting model that combines ambient, diffuse, and specular components.
            This approach creates sharp, well-defined highlights, in contrast to
            the softer, blurred highlights seen with Gouraud shading.
        Controls:
            - Orbit Object: Drag with the left mouse button; scroll to zoom.
            - Orbit Light: Hold Shift and drag; scroll to adjust distance.
        )";
    }
},

                    // InitCallback
                    [](cg::ManagedWindow* pWindow, int pass)
                    {

                        if (pass != 0) { return; }

                              float t = static_cast<float>(glfwGetTime());

                              //objects[0] ->cube
                              //objects[1] ->torus
                              //objects[2] ->sphere
                              //objects[3] ->teacub
                              //objects[4] ->plane

                              // rotate cube
                              {
                                  glm::mat4 M(1.0f);
                                  M = glm::translate(M, glm::vec3(8,0,0));
                                  M = glm::rotate(M, t, glm::vec3(1, 1, 1));
                                  //  M = glm::scale(M, glm::vec3(1.0f));
                                    pWindow->passes[0].objects[0].instances[0].modeltransform = M;
                                }
                              // rotate torus
                              {
                                  glm::mat4 M(1.0f);
                                  M = glm::translate(M, glm::vec3(-8, 0, 0));
                                  M = glm::rotate(M, t, glm::vec3(1, 0, 1));
                                  // M = glm::scale(M, glm::vec3(1.0f));
                                   pWindow->passes[0].objects[1].instances[0].modeltransform = M;
                               }
                              // rotate sphere
                              {


                                  // smooth scale
                                  float s = 1.0f + 0.3f * sin(t);

                                  glm::mat4 M(1.0f);
                                  M = glm::translate(M, glm::vec3(0, 0, 8));
                                  // M = glm::rotate(M, a, glm::vec3(0, 1, 0));
                                   M = glm::scale(M, glm::vec3(s));
                                   pWindow->passes[0].objects[2].instances[0].modeltransform = M;
                               }
                              // rotate teacub
                              {
                                  glm::mat4 M(1.0f);
                                  M = glm::translate(M, glm::vec3(0, 0, -8));
                                  M = glm::rotate(M, t, glm::vec3(0, 1, 0));
                                  //M = glm::scale(M, glm::vec3(1.0f));
                                  pWindow->passes[0].objects[3].instances[0].modeltransform = M;
                              }

                              //plane
                              {
                                  glm::mat4 M(1.0f);
                                  M = glm::translate(M, glm::vec3(0.0f, -3.0f, 0.0f));
                                  //M = glm::rotate(M, a, glm::vec3(0, 1, 0));
                                  //M = glm::scale(M, glm::vec3(1.0f));
                                  pWindow->passes[0].objects[4].instances[0].modeltransform = M;
                              }


                               },

                    // SizeCallback - called when the window is resized
                    nullptr,

                    // KeyCallback - called when a key is pressed
                      nullptr,

                    // MousebuttonCallback - called when a mouse button is pressed
                    nullptr,
                    // ScrollCallback - called when the mouse wheel is scrolled
                    [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
                    {
                        if (mods == GLFW_MOD_SHIFT)
                            pWindow->passes[0].lights[0].onScroll(xoffset, yoffset, 0);
                        else
                            pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                    },

                    // CursorposCallback - called when the mouse is moved
                    [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                    {
                        if (mods == GLFW_MOD_SHIFT)
                            pWindow->passes[0].lights[0].onCursor(xoffset, yoffset, 0);
                        else
                            pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);


                        //std::cout << "cam x :" << pWindow->passes[0].cameras[0].getPosition().x << "\n";
                        //std::cout << "cam y :" << pWindow->passes[0].cameras[0].getPosition().y << "\n";
                        //std::cout << "cam z :" << pWindow->passes[0].cameras[0].getPosition().z << "\n";

                    },
                    //MouseclickCallback - called when a mouse btm is single clicked
                    [](cg::ManagedWindow* pWindow, int xpos, int ypos) {

                    },
                    [](cg::ManagedWindow* pWindow, int xpos, int ypos) { //MousedoubleclickCallback
                        cg::WindowManager::getInstance().copyWindow(*pWindow);
                    }

                    }));
                //I = 0;
                I += W + GAP;
        //=======================================================================================
        //==============================ASSIGNMENT 5============================================

                tab[0].push_back(windowManager.addWindow({
                    cg::State::SHOW,            // State
                    pMainWindow,                // Parent window
                    { 4, 9, GAP },              // Viewport
                   "Assignment 5 (Environment Mapping) - k.ban",      // Title
                    BLACK,                      // Background color
                    NONE, // dont need since we using checkcered board texture
                    NONE,                       // Compute shader
                    {                           // Rendering passes

                                        {                           // Pass 0
                    { MARGIN },                 // Viewport
                    NONE,                       // Background color
                    ENABLE,                     // Depth test
                    { { SKYBOX } },             // Objects
                    { {{ 3.0f, 2.0f, 0.0f }} }, // Cameras
                    NONE,                       // Lights
                    DEFAULT,                    // Rendering target
                    {                           // Vertex and fragment shaders
                        #include "Shaders/Skybox.vert.glsl"
                        ,
                        #include "Shaders/SkyBox.frag.glsl"
                    }
                },
                {                           // Pass 1
                    { MARGIN },                 // Viewport
                    NONE,                       // Background color
                    ENABLE,                     // Depth test
                    DEMOOBJECTS_ENVIRONMENTMAPPING,    // Objects
                    { { cg::CameraType::WALKING, { 0.0f, 2.5f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 90.0f } }, // Cameras
                    NONE,                       // Lights
                    DEFAULT,                    // Rendering target
                    {                           // Vertex and fragment shaders
                        #include "Shaders/ReflectionAndRefraction.vert.glsl"
                        ,
                        #include "Shaders/ReflectionAndRefraction.frag.glsl"
                    }
                }

                    }, // end of pass params
            // Callbacks 
// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }
    if (focused)
    {
        std::cout << std::endl << R"(
        Assignment 5 (Environment Mapping)
            This demo showcases the environment mapping technique that uses cube maps
            to simulate reflective and transparent surfaces along with rendering
            the surrounding environment. The cube map is applied to an object's
            surface to approximate how the environment would appear in its reflection.
            For transparent objects, light rays passing through are refracted slightly,
            creating a realistic distortion effect.
        Controls:
            - Orbit Object: Drag with the left mouse button; scroll to zoom.
            - Change Reflection Factor: Hold Ctrl and scroll the mouse wheel.
            - Change Ratio Of Indices Of Refraction: Hold Alt and scroll the mouse wheel.
            - Rendering Options: Press 1 or 2 to toggle dynamic mode.
        )";
    }
},
            // InitCallback
            [](cg::ManagedWindow* pWindow, int pass)
            {
                if (pass != -1) { 
                    pWindow->passes[1].renderShader.use();
                    pWindow->passes[1].renderShader.setUniform("dynamic_option", dynamic_option);
                }
            }, // InitCallback
            nullptr, // SizeCallback
            // KeyCallback - called when a key is pressed
                [](cg::ManagedWindow* pWindow, int key)
                {
                    //wasd key movement based on raycast demo
                    switch (key)
                    {
                    case '1':
                        dynamic_option = 1;
                        pWindow->passes[1].renderShader.use();
                        pWindow->passes[1].renderShader.setUniform("dynamic_option", dynamic_option);
                        return false; // Do not propagate the event

                    case '2':
                        dynamic_option = 2;
                        pWindow->passes[1].renderShader.use();
                        pWindow->passes[1].renderShader.setUniform("dynamic_option", dynamic_option);
                        return false; // Do not propagate the event
                    }
                    return true; // Propagate the event by default
                }, // KeyCallback
            nullptr, // MousebuttonCallback
            [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
            {
              auto clamp = [](float v, float lo, float hi)
                {
                    return (v < lo) ? lo : (v > hi) ? hi : v;
                };

              // SHIFT + scroll -> reflectionFactor
              if (mods & GLFW_MOD_CONTROL)
              {
                  auto& mat = pWindow->passes[1].objects[0].instances[0].material;
                  auto& rf = mat.params["material.reflectionFactor"];
                  if (rf.empty()) rf.push_back(0.9f);

                  rf[0] += static_cast<float>(yoffset) * 0.05f;
                  rf[0] = clamp(rf[0], 0.0f, 1.0f);
              }
              // CTRL + scroll -> eta
              else if (mods & GLFW_MOD_ALT)
              {
                  auto& mat = pWindow->passes[1].objects[0].instances[0].material;
                  auto& eta = mat.params["material.eta"];
                  if (eta.empty()) eta.push_back(0.94f);

                  eta[0] += static_cast<float>(yoffset) * 0.02f;
                  eta[0] = clamp(eta[0], 0.1f, 3.0f);
              }
              // normal scroll -> zoom both cameras
              else
              {
                  pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                  pWindow->passes[1].cameras[0].onScroll(xoffset, yoffset);
              }

            },

            // CursorposCallback - called when the mouse is moved
            [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
            {
                //if (mods == GLFW_MOD_SHIFT)
                //    pWindow->passes[0].lights[0].onCursor(xoffset, yoffset, 0);
                //else {
                //     pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                //     pWindow->passes[1].cameras[0].onCursor(xoffset, yoffset);
                //}
                //dont need lights in this assignment
                     pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                     pWindow->passes[1].cameras[0].onCursor(xoffset, yoffset);
            },
            //MouseclickCallback - called when a mouse btm is single clicked
            [](cg::ManagedWindow* pWindow, int xpos, int ypos) {

            },
            [](cg::ManagedWindow* pWindow, int xpos, int ypos) { //MousedoubleclickCallback
            cg::WindowManager::getInstance().copyWindow(*pWindow);
            }

            }));
        I += W + GAP;
        //=======================================================================================
        //==============================ASSIGNMENT 8============================================

        /* //set to walking 
        * { cg::CameraType::WALKING, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 75.0f }
        */

        tab[0].push_back(windowManager.addWindow({
           cg::State::SHOW,            // State
           pMainWindow,                    // Parent window
            { 5, 9, GAP },                    // Viewport
           "Assignment 8 (Night Sky) - k.ban",  // Title
           BLACK,                      // Background color
           {                           // Textures
               {
                   "NightSky/night_negx.png", "NightSky/night_posx.png",
                   "NightSky/night_posy.png", "NightSky/night_negy.png",
                   "NightSky/night_posz.png", "NightSky/night_negz.png"
               }
           },
           NONE,                       // Compute shader
           {                           // Rendering passes
               {                           // Pass 0
                   { MARGIN },                 // Viewport
                   NONE,                       // Background color
                   ENABLE,                     // Depth test
                   { { SKYBOX } },             // Objects
                   { {{ 6.0f, 0.0f, 4.0f }} }, // Cameras
                   NONE,                       // Lights
                   DEFAULT,                    // Rendering target
                   {                           // Vertex and fragment shaders
                       #include "Shaders/Skybox.vert.glsl"
                       ,
                       #include "Shaders/NightSkybox.frag.glsl"
                   }
               },
               {                           // Pass 1
                   { MARGIN },                 // Viewport
                   NONE,                       // Background color
                   ENABLE,                     // Depth test
                   {                           // Objects
                       {
                           PLANE,
                           MATERIAL_CHECKERBOARD,
                           glm::translate(glm::scale(glm::mat4(1.0f), {3.0f, 1.0f, 3.0f}), {0.0f, -1.0f, 0.0f})
                       },
                       {
                           SPHERE,
                           MATERIAL_PBR_PHONG,
                           glm::translate(glm::mat4(1.0f), {3.0f, 0.0f, 0.0f})
                       },
                       {
                           CUBE,
                           MATERIAL_DISCARD,
                           glm::translate(glm::mat4(1.0f), {-3.0f, 0.0f, 0.0f})
                       },
                       {
                           TEAPOT,
                           MATERIAL_CARTOON,
                           glm::translate(glm::mat4(1.0f), {0.0f, -0.5f, 0.0f})
                       }
                   },
                   { { cg::CameraType::WALKING, 
                     { 4.54f, 0.0f, 3.64f }, //cam position
                     { 0.0f, 1.0f, 0.0f }, //view target 
                       75.0f } //fov 
                   }, // Cameras
                   { {{ 4.0f, 3.0f, 4.0f }} }, // Lights
                   DEFAULT,                    // Rendering target
                   {                           // Vertex and fragment shaders
                       #include "Shaders/ViewSpaceTransform.vert.glsl" 
                       ,
                       #include "Shaders/Ass8NightSky.frag.glsl" 
                   }
               }
           }, //callbacks
// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }
    if (focused)
    {
        std::cout << std::endl << R"(
        Assignment 8 (NightSky)
            This demo combines multiple rendering techniques into a single multi-pass
            process. It integrates the skybox and procedural textures from the previous
            assignments with new features, including fog, cartoon style shading, and
            fragment discarding. Two types of camera movement are also demonstrated:
            orbiting and walking.
        Controls:
            - Orbit Object: Drag with the left mouse button; scroll to zoom.
            - Orbit Light: Hold Shift and drag; scroll to adjust distance.
            - Rendering Options: Press W or O to switch the camera movement types.
        )";
    }
},

            // InitCallback - called for initializing shaders across
            // the entire rendering process or individual passes when required
            [](cg::ManagedWindow* pWindow, int pass)
            {

                if (pass == -1)
                {
                    // Fog parameters
                    pWindow->context.floatparam[0] = 2.0f;   // fog.minDist
                    pWindow->context.floatparam[1] = 12.0f;  // fog.maxDist
                    pWindow->context.vec4param[0] = FOG_COLOR;     // fog.color
                    //change to pass1
                    pWindow->passes[1].renderShader.use();
                    pWindow->passes[1].renderShader.setUniform("fog.minDist", pWindow->context.floatparam[0]);
                    pWindow->passes[1].renderShader.setUniform("fog.maxDist", pWindow->context.floatparam[1]);
                    pWindow->passes[1].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);

                    // Spotlight parameters
                    pWindow->passes[1].renderShader.setUniform("light[0].Direction", glm::vec3(0.0f, -1.0f, 0.0f));
                    pWindow->passes[1].renderShader.setUniform("light[0].Exponent", 1.0f);
                    pWindow->passes[1].renderShader.setUniform("light[0].Cutoff", 0.45f);
                }
            },

            // SizeCallback - called when the window is resized
            nullptr,

            // KeyCallback - called when a key is pressed
            [](cg::ManagedWindow* pWindow, int key)
            {
                //W O key movement 
                switch (key)
                {
                case 'W':
                    std::cout << "Option W - Walking camera" << std::endl;
                    pWindow->passes[0].cameras[0].setType(cg::CameraType::WALKING);
                    return false; // Do not propagate the event

                case 'O':
                    std::cout << "Option O - Orbiting camera" << std::endl;
                    pWindow->passes[0].cameras[0].setType(cg::CameraType::ORBITING);
                    return false; // Do not propagate the event
                }

                return true; // Propagate the event by default
            },

            // MousebuttonCallback - called when a mouse button is pressed
            nullptr,

            // ScrollCallback - called when the mouse wheel is scrolled
            [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
            {
                if (mods == GLFW_MOD_SHIFT)
                    pWindow->passes[1].lights[0].onScroll(xoffset, yoffset, 0);
                else
                    pWindow->passes[1].cameras[0].onScroll(xoffset, yoffset);
            },

            // CursorposCallback - called when the mouse is moved
            [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset,
                    double yoffset, int mods)
            {
                if (mods == GLFW_MOD_SHIFT)
                    pWindow->passes[1].lights[0].onCursor(xoffset, yoffset, 0);
                else
                    pWindow->passes[1].cameras[0].onCursor(xoffset, yoffset);

                //std::cout << "cam x :" << pWindow->passes[1].cameras[0].getPosition().x << "\n";
                //std::cout << "cam y :" << pWindow->passes[1].cameras[0].getPosition().y << "\n";
                //std::cout << "cam z :" << pWindow->passes[1].cameras[0].getPosition().z << "\n";
            },
            //MouseclickCallback - called when a mouse btm is single clicked
            [](cg::ManagedWindow* pWindow, int xpos, int ypos) {

            },
            [](cg::ManagedWindow* pWindow, int xpos, int ypos) { //MousedoubleclickCallback
                cg::WindowManager::getInstance().copyWindow(*pWindow);
            }
          
        }));
        I = 0;
        //===
        // 
        
         //=======================================================================================
        //==============================ASSIGNMENT 9============================================

        /* //set to walking
        * { cg::CameraType::WALKING, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 75.0f }
        */

        /*
        *        Assignment 9 Cartoon
                    Obit Obj
                    Orbit Light
                    Blur Toggle
                    Edge Toggle
                    Edge threshold adj
                    alpha factor adj

        Controls:
       - Orbit Object: Drag with the left mouse button; scroll to zoom.
       - Orbit Light: Hold Shift and drag; scroll to adjust distance.
       - Blur Filter: Press 1 to enable and 2 to disable.
       - Edge Filter: Press 3 to enable and 4 to disable.
       - Edge Threshold: Hold Alt and scroll to adjust.
       - Alpha Factor: Hold Control and scroll to adjust.
        */

        tab[0].push_back(windowManager.addWindow({
           cg::State::SHOW,            // State
           pMainWindow,                    // Parent window
            { 6, 9, GAP },                    // Viewport
           "Assignment 9 (Cartoon) - k.ban",  // Title
           BLACK,                      // Background color
           { { texture0, GL_TEXTURE0},
             { texture1, GL_TEXTURE1},
             { texture2, GL_TEXTURE2},
             { texture3, GL_TEXTURE3},
             { texture4, GL_TEXTURE4},
           },  // Textures   
           NONE,                       // Compute shader
           {                           // Rendering passes
               {                           // Pass 0,  standard Blinn-Phong shading
                   { MARGIN },                 // Viewport
                   WHITE,                       // Background color
                   ENABLE,                     // Depth test
                   DEMOOBJECTS_FILTERS,             // Objects
                   { DEFAULT }, // Cameras
                   { DEFAULT } ,                       // Lights
                    {
                        cg::FBO::Type::Texture,
                        GL_TEXTURE0,
                        &texture0
                    },                    // Rendering target
                   {                           // Vertex and fragment shaders
                       #include "Shaders/ViewSpaceTransform.vert.glsl"
                       ,
                       #include "Shaders/BlinnPhong.frag.glsl"
                   }
               },
                //Pass1 Vertical gaussian blur filter
               {                           // Pass 1
                   { MARGIN },                 // Viewport
                   NONE,                       // Background color
                   DISABLE,                     // Depth test
                   {                           // Objects
                        QUAD
                   },
                   { NONE }, // Cameras
                   { NONE }, // Lights
                   {
                        cg::FBO::Type::Texture,
                        GL_TEXTURE1,
                        &texture1
                    },                   // Rendering target
                   {                           // Vertex and fragment shaders
                       #include "Shaders/PassThrough.vert.glsl" 
                       ,
                       #include "Shaders/BlurFilter.1.frag.glsl" 
                   }
               },
            //Pass2 Horizontal gaussian blur filter
               {                           // Pass 2
                   { MARGIN },                 // Viewport
                   NONE,                       // Background color
                   DISABLE,                     // Depth test
                   {                           // Objects
                        QUAD
                   },
                   { NONE }, // Cameras
                   { NONE }, // Lights
                   {
                        cg::FBO::Type::Texture,
                        GL_TEXTURE2,
                        &texture2
                    },                   // Rendering target
                   {                           // Vertex and fragment shaders
                       #include "Shaders/PassThrough.vert.glsl" 
                       ,
                       #include "Shaders/BlurFilter.2.frag.glsl" 
                   }
            },
             
            //Pass3 Sobel Edge detection Filter
               {                           // Pass 3
                   { MARGIN },                 // Viewport
                   NONE,                       // Background color
                   DISABLE,                     // Depth test
                   {                           // Objects
                        QUAD
                   },
                   { NONE }, // Cameras
                   { NONE }, // Lights
                   {
                        cg::FBO::Type::Texture,
                        GL_TEXTURE3,
                        &texture3
                    },
                   {                           // Vertex and fragment shaders
                       #include "Shaders/PassThrough.vert.glsl" 
                       ,
                       #include "Shaders/EdgeFilters.frag.glsl" 
                   }
               },
            //Pass4 Cartoon shading model
                           {                           // Pass 4
                   { MARGIN },                 // Viewport
                   WHITE,                       // Background color
                   ENABLE,                     // Depth test
                                             // Objects
                       DEMOOBJECTS_FILTERS
                   ,
                   { DEFAULT  }, // Cameras
                   { DEFAULT  }, // Lights
                   {
                        cg::FBO::Type::Texture,
                        GL_TEXTURE4,
                        &texture4
                    },
                   {                           // Vertex and fragment shaders
                       #include "Shaders/ViewSpaceTransform.vert.glsl" 
                       ,
                       #include "Shaders/BlinnPhongCartoon.frag.glsl" 
                   }
               },
            //Pass5 blend edge detection and cartoon
                           {                           // Pass 5
                   { MARGIN },                 // Viewport
                   NONE,                       // Background color
                   DISABLE,                     // Depth test
                   {                           // Objects
                        QUAD
                   },
                   { NONE }, // Cameras
                   { NONE }, // Lights
                   {
                        NONE
                    },                   // Rendering target
                   {                           // Vertex and fragment shaders
                       #include "Shaders/PassThrough.vert.glsl" 
                       ,
                       #include "Shaders/finalpass.frag.glsl" 
                   }
               }
           },// END OF PASSES
// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }
    if (focused)
    {
        std::cout << std::endl << R"(
        Assignment 9 (Cartoon)
            This demo showcases a multi-pass cartoon-style rendering pipeline that creates
            non-photorealistic effects with crisp edge outlines. The process applies a
            Gaussian blur filter to reduce noise, followed by Sobel edge detection to
            identify object boundaries. The scene is rendered twice: once with standard
            Blinn-Phong shading for edge analysis, and again with cartoon-style shading
            for the final cell-shaded appearance. The final pass composites the edge
            outlines with the cartoon shading to produce a stylized 2D animation effect.
        Controls:
            - Orbit Object: Drag with the left mouse button; scroll to zoom.
            - Orbit Light: Hold Shift and drag; scroll to adjust distance.
            - Blur Filter: Press 1 to enable and 2 to disable.
            - Edge Filter: Press 3 to enable and 4 to disable.
            - Edge Threshold: Hold Alt and scroll to adjust.
            - Alpha Factor: Hold Control and scroll to adjust.
        )";
    }

    if (focused) {
        std::cout << "Blur Filter is " << (BlurFilter ? "ON" : "OFF") << "\n";
        std::cout << "Edge Filter is " << (EdgeFilter ? "ON" : "OFF") << "\n";
    }
                    
},

            // InitCallback - called for initializing shaders across
            // the entire rendering process or individual passes when required
            [](cg::ManagedWindow* pWindow, int pass)
            {

                if (pass == -1)
                {

                    //BlueFilter
                    pWindow->passes[1].renderShader.use();
                    pWindow->passes[1].renderShader.setUniform("BlurFilter", BlurFilter);
                    pWindow->passes[2].renderShader.use();
                    pWindow->passes[2].renderShader.setUniform("BlurFilter", BlurFilter);
                    pWindow->passes[3].renderShader.use();
                    //Edge Filter
                    pWindow->passes[3].renderShader.setUniform("EdgeFilter", EdgeFilter);
                    pWindow->passes[3].renderShader.use();
                    pWindow->passes[3].renderShader.setUniform("EdgeFilter", EdgeFilter);


                    //edge
                    pWindow->passes[3].renderShader.use();
                    pWindow->passes[3].renderShader.setUniform("EdgeThreshold", EdgeThreshold);
                    pWindow->passes[3].renderShader.setUniform("EdgeColor", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                    pWindow->passes[3].renderShader.setUniform("BackgroundColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

                    //Alpha Factor
                    pWindow->passes[5].renderShader.use();
                    pWindow->passes[5].renderShader.setUniform("AlphaFactor", AlphaFactor);
                }
            },

            // SizeCallback - called when the window is resized
            nullptr,

            // KeyCallback - called when a key is pressed
            [](cg::ManagedWindow* pWindow, int key)
            {
                if (key == '1')
                {
                    BlurFilter = true;
                    std::cout << "Option 1 - Enable blur filter\n";

                    pWindow->passes[1].renderShader.use();
                    pWindow->passes[1].renderShader.setUniform("BlurFilter", BlurFilter);
                    pWindow->passes[2].renderShader.use();
                    pWindow->passes[2].renderShader.setUniform("BlurFilter", BlurFilter);

                    std::cout << "BlurFilter? " << BlurFilter <<"\n";
                    return false;
                }

                if (key == '2')
                {
                    BlurFilter = false;
                    std::cout << "Option 2 - Disable blur filter\n";

                    pWindow->passes[1].renderShader.use();
                    pWindow->passes[1].renderShader.setUniform("BlurFilter", BlurFilter);
                    pWindow->passes[2].renderShader.use();
                    pWindow->passes[2].renderShader.setUniform("BlurFilter", BlurFilter);
                    std::cout << "BlurFilter? " << BlurFilter << "\n";
                    return false;
                }

                if (key == '3')
                {
                    EdgeFilter = true;
                    std::cout << "Option 3 - Enable edge filter\n";

                    pWindow->passes[3].renderShader.use();
                    pWindow->passes[3].renderShader.setUniform("EdgeFilter", EdgeFilter);

                    pWindow->passes[0].backgroundColor = WHITE;
                    std::cout << "EdgeFilter? " << EdgeFilter << "\n";
                    return false;
                }

                if (key == '4')
                {
                    EdgeFilter = false;
                    std::cout << "Option 4 - Disable edge filter\n";

                    pWindow->passes[3].renderShader.use();
                    pWindow->passes[3].renderShader.setUniform("EdgeFilter", EdgeFilter);

                    pWindow->passes[0].backgroundColor = BLACK;

                    std::cout << "EdgeFilter? " << EdgeFilter << "\n";
                    return false;
                }

                return true; // Propagate the event by default
            },

            // MousebuttonCallback - called when a mouse button is pressed
            nullptr,

            // ScrollCallback - called when the mouse wheel is scrolled
            [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
            {
                if (mods == GLFW_MOD_SHIFT) {
                    pWindow->passes[0].lights[0].onScroll(xoffset, yoffset, 0);
                    pWindow->passes[4].lights[0].onScroll(xoffset, yoffset, 0);
                }               
                else if (mods == GLFW_MOD_CONTROL) {
                    //AlphaFactor
                    AlphaFactor += static_cast<float>(yoffset) * 0.05f;
                    if (AlphaFactor < 0.0f) AlphaFactor = 0.0f;
                    if (AlphaFactor > 1.0f) AlphaFactor = 1.0f;

                    pWindow->passes[5].renderShader.use();
                    pWindow->passes[5].renderShader.setUniform("AlphaFactor", AlphaFactor);

                    std::cout << "AlphaFactor = " << AlphaFactor << "\n";
                }
             
                else if (mods == GLFW_MOD_ALT) {
                    //edge factor
                    EdgeThreshold += static_cast<float>(yoffset) * 0.02f;
                    if (EdgeThreshold < 0.0f) EdgeThreshold = 0.0f;
                    if (EdgeThreshold > 1.0f) EdgeThreshold = 1.0f;

                    pWindow->passes[3].renderShader.use();
                    pWindow->passes[3].renderShader.setUniform("EdgeThreshold", EdgeThreshold);

                    std::cout << "EdgeThreshold = " << EdgeThreshold << "\n";
                }             
                else {
                    pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                    pWindow->passes[4].cameras[0].onScroll(xoffset, yoffset);
                }
          
            },

            // CursorposCallback - called when the mouse is moved
            [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset,
                    double yoffset, int mods)
            {
                //if (mods == GLFW_MOD_SHIFT)
                //    pWindow->passes[0].lights[0].onCursor(xoffset, yoffset, 0);
                //else
                //    pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);


                if (mods == GLFW_MOD_SHIFT) {
                    pWindow->passes[0].lights[0].onCursor(xoffset, yoffset, 0);
                    pWindow->passes[4].lights[0].onCursor(xoffset, yoffset, 0);
                }
                else if (mods == GLFW_MOD_CONTROL) {
                    //AlphaFactor
                }

                else if (mods == GLFW_MOD_ALT) {
                    //edge factor
                }
                else {
                    pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                    pWindow->passes[4].cameras[0].onCursor(xoffset, yoffset);
                }

            },
            //MouseclickCallback - called when a mouse btm is single clicked
            [](cg::ManagedWindow* pWindow, int xpos, int ypos) {

            },
            [](cg::ManagedWindow* pWindow, int xpos, int ypos) { //MousedoubleclickCallback
                cg::WindowManager::getInstance().copyWindow(*pWindow);
            }

            }));

            I += W + GAP;
            //=======================================================================================
            //==============================ASSIGNMENT 10============================================

            //"Pisa/pisa_posx.hdr", "Pisa/pisa_negx.hdr",
            //    "Pisa/pisa_posy.hdr", "Pisa/pisa_negy.hdr",
            //    "Pisa/pisa_posz.hdr", "Pisa/pisa_negz.hdr"

            tab[0].push_back(windowManager.addWindow({
                        cg::State::SHOW,            // State
                        pMainWindow,                // Parent window
                        { 7, 9, GAP },              // Viewport
                        "Assignment 10 (Portal) - k.ban",      // Title
                        BLACK,                      // Background color
                        {

                    //{{"Pisa/pisa_posx.hdr", "Pisa/pisa_negx.hdr",
                    //"Pisa/pisa_posy.hdr", "Pisa/pisa_negy.hdr",
                    //"Pisa/pisa_posz.hdr", "Pisa/pisa_negz.hdr"},GL_TEXTURE0},
                    // {textureA, GL_TEXTURE1}
                    {textureA, GL_TEXTURE7},
                    {{"Pisa/pisa_posx.hdr", "Pisa/pisa_negx.hdr",
                    "Pisa/pisa_posy.hdr", "Pisa/pisa_negy.hdr",
                    "Pisa/pisa_posz.hdr", "Pisa/pisa_negz.hdr"},GL_TEXTURE6}

                },                       // Textures
                NONE,                       // Compute shader
                {                           // Rendering passes
            {  // Pass 0 : render portal texture into textureA
                { MARGIN },                 // Viewport
                NONE,                       // Background color
                DISABLE,                     // Depth test
                { {QUAD} },    // Objects
                NONE, // Cameras
                NONE,                       // Lights
                {
                    cg::FBO::Type::Texture,
                    GL_TEXTURE7,
                    &textureA
                },                    // Rendering target
                {                           // Vertex and fragment shaders
                    #include "Shaders/PassThrough.vert.glsl"
                    ,
                    #include "Shaders/Portal.frag.glsl"
                }
            },

            {                               // Pass 1
                { MARGIN },                 // Viewport
                BLACK,                       // Background color
                ENABLE,                     // Depth test
                {  {SKYBOX} } ,             // Objects
                { { cg::CameraType::WALKING, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 75.0f }  }, // Cameras
                NONE,                       // Lights
                DEFAULT,                    // Rendering target
                {                           // Vertex and fragment shaders
                    #include "Shaders/PortalSkyBox.vert.glsl"
                    ,
                    #include "Shaders/PortalSkyBox.frag.glsl"
                }
            },

                }, // end of pass params

                // Callbacks 
// FocusCallback - called when the window gains or loses focus
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }
    if (focused)
    {
        std::cout << std::endl << R"(
        Assignment 10 (Portal)
            This demo showcases the integration of cube mapping and projected texture
            techniques with dynamic texture rendering. Cube mapping is used to render
            the skybox, while projected textures visualize a gate (portal) to another
            dimension or location in the world.
        Controls:
            - Rotating And Moving Camera: Drag with the left mouse button and scroll
                the mouse wheel.
            - Open Portal: Click the left mouse button at the desired location to
                place the new portal.
        )";
    }
}, // FocusCallback
                // InitCallback - called for initializing shaders across
                // the entire rendering process or individual passes when required
                [](cg::ManagedWindow* pWindow, int pass)
                {

                    if (pass == -1)
                    {
                        glm::mat4 projView = glm::lookAt(glm::vec3(3.0f, 7.0f, 7.0f), glm::vec3(-2.0f, -4.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
                        glm::mat4 projProj = glm::perspective(glm::radians(30.0f), 1.0f, 0.2f, 1000.0f);
                        glm::mat4 bias = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f)), glm::vec3(0.5f));

                        pWindow->passes[1].renderShader.use();
                        pWindow->passes[1].renderShader.setUniform("ProjectorMatrix", bias * projProj * projView);

                    }

                    if (pass == -1)
                    {
                        PortalPlaced = false;
                    }
                }, // InitCallback
                nullptr, // SizeCallback
                nullptr, // KeyCallback
                // MousebuttonCallback - called when a mouse button is pressed
                [](cg::ManagedWindow* pWindow, int button, int xpos, int ypos)
                {
                       
                 }, // MousebuttonCallback
                 [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
                 {
                       pWindow->passes[1].cameras[0].onScroll(xoffset, yoffset);
                 },

                // CursorposCallback - called when the mouse is moved
                [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                {
                         pWindow->passes[1].cameras[0].onCursor(xoffset, yoffset);
                },
                //MouseclickCallback - called when a mouse btm is single clicked
                [](cg::ManagedWindow* pWindow, int xpos, int ypos) {
                        if (xpos >= 0 && xpos < pWindow->width && ypos >= 0 && ypos < pWindow->height)                              
                        {
                            // Read depth buffer under mouse
                            float depth = 1.0f;
                            //read pixels stored into depth
                            glReadPixels(xpos, pWindow->passes[1].height - ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
                            std::cout << "depth after glReadPixels = " << depth << std::endl;
                            if (depth >= 1.0f)
                            {
                                //std::cout << "No valid surface hit.\n";
                                return;
                            }
                            // Convert to world coordinates
                            glm::vec3 worldPos = glm::unProject(
                                glm::vec3(xpos, pWindow->passes[1].height - ypos, depth),
                                pWindow->passes[1].cameras[0].getLookAt(),        // view
                                pWindow->passes[1].cameras[0].getPerspective(
                                    static_cast<float>(pWindow->passes[1].width)
                                    / pWindow->passes[1].height),                 // projection
                                glm::vec4(0, 0, pWindow->passes[1].width, pWindow->passes[1].height)  // viewport
                            );

                            std::cout << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << "...\r";

                            PortalPlaced = true;

                            glm::mat4 view = pWindow->passes[1].cameras[0].getLookAt();
                            glm::mat4 invView = glm::inverse(view);
                            glm::vec3 eye = glm::vec3(invView[3]);
                            PortalMatrix = BuildPortalMatrix(worldPos, eye);


                            pWindow->passes[1].renderShader.use();
                            pWindow->passes[1].renderShader.setUniform("ProjectorMatrix", PortalMatrix);

                            // PortalPlaced = false;
                            std::cout << "Portal placed at "
                                << worldPos.x << ", "
                                << worldPos.y << ", "
                                << worldPos.z << "\n";
                        }             
                    },
                    [](cg::ManagedWindow* pWindow, int xpos, int ypos) { //MousedoubleclickCallback
                        cg::WindowManager::getInstance().copyWindow(*pWindow);
                    }

                }));

                I += W + GAP;
          //==============================================================================================
          //==============================END OF ASSIGNMENT 10============================================
            //=======================================================================================
            //==============================ASSIGNMENT 11============================================
            /*
            * Deferred shading first pass to just store geometry information in g buffer
            * For normals (lightdir,viewdir and texcoord)
            * 
            * second pass read from g buffer evaluate reflecction model
            * and produce final color for pixel
            * 
            */
                 //DEFERED SHADING
                tab[0].push_back(windowManager.addWindow({
                    cg::State::SHOW,            // State
                    pMainWindow,                    // Parent window
                    { 8, 9, GAP },                    // Viewport
                    "Assignment 10 (Ogres) - k.ban",         // Title
                    BLACK,                      // Background color
                    {
                        { { "Ogre/ogre_diffuse.png" },   GL_TEXTURE5 },
                        { { "Ogre/ogre_normalmap.png" }, GL_TEXTURE6 }
                    },                       // Textures
                    NONE,                       // Compute shader
                    {                           // Rendering passes
                        {                           // Pass 0
                            { MARGIN  },                 // Viewport
                            BLACK,                      // Background color
                            ENABLE,                     // Depth test
                            {                           // Objects
                                {
                                    OGRE,
                                    {
                                        INSTANCES(-15.0f),
                                        INSTANCES(-12.0f),
                                        INSTANCES(-9.0f),
                                        INSTANCES(-6.0f),
                                        INSTANCES(-3.0f),
                                        INSTANCES(0.0f),
                                        INSTANCES(3.0f),
                                        INSTANCES(6.0f),
                                        INSTANCES(9.0f),
                                        INSTANCES(12.0f),
                                        INSTANCES(15.0f),
                                    }
                                }
                            },
                            { {{ 0.0f, 5.0f, 5.0f }} },     // Cameras
                            { { { 0.0f, 5.0f, 5.0f } } },                           // Lights
                            { cg::FBO::Type::Deferred },    // Rendering target
                            {                               // Vertex and fragment shaders
                                #include "Shaders/NormalMap.vert.glsl"
                                ,
                                #include "Shaders/GBuffer.frag.glsl"
                            }

                        }
                        ,
                        {                               // Pass 1
                            { MARGIN },                     // Viewport
                            BLACK,                           // Background color
                            DISABLE,                        // Depth test
                            {                               // Objects
                                {
                                    QUAD,
                                    MATERIAL_OBJECT
                                }
                            },
                             NONE,                           // Cameras
                            { { { 0.0f, 5.0f, 5.0f } } },   // Lights
                            DEFAULT,                        // Rendering target
                            {                               // Vertex and fragment shaders
                                #include "Shaders/PassThrough.vert.glsl"
                                ,
                                #include "Shaders/DLight&Shading.frag.glsl"
                            }
                        }
                    },
//FocusCallback
[](cg::ManagedWindow* pWindow, int focused)
{
    if (focused) {
        std::cout << std::endl << R"(
    Done by    : Ban Kai Wei Benjamin (k.ban@digipen.edu)
    Instructor : Vadim Surov (vsurov@digipen.edu)
    School     : Digipen Institute of Technology Singapore
            )";
    }
    if (focused) {
        std::cout << std::endl << R"(
        Assignment 11 (Ogres)
            This demo showcases both normal mapping and deferred shading techniques.
            Normal mapping enhances the visual complexity of rendered objects without
            increasing geometric detail. Deferred shading improves rendering
            efficiency by avoiding multiple shading calculations for the same pixel,
            making it particularly effective for scenes with a large number of
            objects, as demonstrated here.
        Controls:
            - Orbit Object: Drag with the left mouse button; scroll to zoom.
            - Orbit Light: Hold Shift and drag; scroll to adjust distance.
            - Rendering Options: Press 1-3 to change the rendering mode.

        Option 1 : BlinnPhong
        Option 2 : Light direction vector
        Option 3 : View direction vector
        )";
    }                           
},
                    //InitCallback
                    [](cg::ManagedWindow* pWindow, int pass) {// InitCallback
                        if (pass == -1) {
                            pWindow->passes[1].renderShader.use();
                            pWindow->passes[1].renderShader.setUniform("TextureMode", TextureMode);
                            pWindow->passes[1].renderShader.use();
                            pWindow->passes[1].renderShader.setUniform("BackgroundColor", BG_color);
                        }
                    },
                    //SizeCallback
                    nullptr,
                    //KeyCallback

                    [](cg::ManagedWindow* pWindow, int key)
                    {
                        if (key == '1')
                        {
                            std::cout << "Option 1 - BlinnPhong shading using gbuffer\n";
                        
                            TextureMode = 1;
                            pWindow->passes[1].renderShader.use();
                            pWindow->passes[1].renderShader.setUniform("TextureMode", TextureMode);

                            return false;
                        }

                        if (key == '2')
                        {
                            TextureMode = 2;
                            std::cout << "Option 2 - light directions of gbuffer\n";
                            pWindow->passes[1].renderShader.use();
                            pWindow->passes[1].renderShader.setUniform("TextureMode", TextureMode);

                            return false;
                        }

                        if (key == '3')
                        {
                            TextureMode = 3;
                            std::cout << "Option 3 - view directions of gbuffer\n";
                            pWindow->passes[1].renderShader.use();
                            pWindow->passes[1].renderShader.setUniform("TextureMode", TextureMode);

                            return false;
                        }



                        return true; // Propagate the event by default
                    },
                    //MousebuttonCallback
                    nullptr,
                    //ScrollCallback
                    [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
                    {
                        if (mods == GLFW_MOD_SHIFT) {
                           // std::cout << "shift press scrollback\n";
                            pWindow->passes[0].lights[0].onCursor(xoffset, yoffset, 0);
                        }
                        else {
                            pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                        }
       

                    },
                    //CursorposCallback
                     [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                    {

                        if (mods == GLFW_MOD_SHIFT) {
                            pWindow->passes[0].lights[0].onCursor(xoffset, yoffset, 0);
                        }
                        else {
                            pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                        }
        


                    },
                    //MouseclickCallback
                    [](cg::ManagedWindow* pWindow, int xpos, int ypos) {
                        
                    },
                    //MousedoubleclickCallback
                    [](cg::ManagedWindow* pWindow, int xpos, int ypos) {
                        cg::WindowManager::getInstance().copyWindow(*pWindow);
                    }


                    }));
        //==============================================================================================
        //==============================END OF ASSIGNMENT 11============================================

                    ShowSelectedTab(0);
                    // 
                    //for (auto* window : tab[0])
                    //glfwShowWindow(window);
                    //for (auto* window : tab[1])
                    //    glfwHideWindow(window);

                    //for (auto* window : tab[2])
                    //    glfwHideWindow(window);
        //==============================END FOR WINDOWS================================
        // Show all windows with SHOW state
        windowManager.show();

        // Set focus to the main window if available
        if (pMainWindow)
            glfwFocusWindow(pMainWindow);

        // Start the rendering loop
        windowManager.render();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        retcode = -1;
    }

    // Explicit cleanup before GLFW termination
    windowManager.cleanup();

    // Clean up and terminate GLFW
    glfwTerminate();

    return retcode;
}