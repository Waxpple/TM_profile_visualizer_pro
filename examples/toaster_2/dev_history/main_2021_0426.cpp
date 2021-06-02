// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "implot_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <imfilebrowser.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
// Encapsulates examples for customizing ImPlot.
namespace ImPlot2 {
template <typename T>
int BinarySearch(const T* arr, int l, int r, T x) {
    if (r >= l) {
        int mid = l + (r - l) / 2;
        if (arr[mid] == x)
            return mid;
        if (arr[mid] > x)
            return BinarySearch(arr, l, mid - 1, x);
        return BinarySearch(arr, mid + 1, r, x);
    }
    return -1;
}
void PlotTM(const char* label_id, const double* xs, const double* xss , const double* thread_id, const double* event_type, int count, ImVec4 commit, ImVec4 abort) {

    // get ImGui window DrawList
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    
    // begin plot item
    if (ImPlot::BeginItem(label_id)) {
        // override legend icon color
        ImPlot::GetCurrentItem()->Color = IM_COL32(64,64,64,255);
        for (int i = 0; i < count; ++i) {
            ImVec2 open_pos  = ImPlot::PlotToPixels(xs[i], thread_id[i]-0.5);
            ImVec2 close_pos = ImPlot::PlotToPixels(xss[i], thread_id[i]+0.5);
            ImU32 color      = ImGui::GetColorU32(event_type[i]==1 ? commit : abort);
            draw_list->AddRectFilled(open_pos, close_pos, color);
        }

        // end plot item
        ImPlot::EndItem();
    }
}

void Random_t_id(int size,double* arr){
    for(int i=0;i<size;++i){
        arr[i] = (rand() % 4);
    }
}
void Random_xs(int size,double* xs,double* xss, int limit){
    for(int i=0;i<size;++i){
        xs[i] = rand() % (limit*5) ;
        xss[i] = xs[i] + rand() %200 /20.0;
    }
}
void Random_event(int size,double* event){
    for(int i=0;i<size;++i){
        event[i] = rand() % 2;
        
    }
}


} // namespace MyImPlot



static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**)
{   
    srand(5);
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Transactional Memory Visualizer Pro", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);


    // 0: not enable vsync
    // 1: enable vsync
    glfwSwapInterval(0); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_visualizer = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // float x_data[11] = {0.1,1.2,2.3,3.4,4.5,5.6,6.7,7.8,8.9,1.0,1.10};
    // float y_data[11] = {0,1,0,1,0,1,0,1,0,1,1};
    int size = 5;
    double thread_id[size];
    double xs[size];
    double xss[size];
    double event_type[size];
    bool raw = true;
    bool every_100 = false;
    float x = 0.0;
    std::vector<double> matrix;
        // create a file browser instance
    ImGui::FileBrowser fileDialog;
    
    // (optional) set browser properties
    fileDialog.SetTitle("title");
    fileDialog.SetTypeFilters({ ".h", ".cpp" });
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Select Event File"))
                {
                    
                    fileDialog.Open();
                    fileDialog.SetTypeFilters({".*"});
                    
                }
                if (ImGui::MenuItem("Quit", "Alt+F4")) break;

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools"))
            {
                if( ImGui::MenuItem("Visualizer") ){
                    
                    show_visualizer = true;
                }
                
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(u8"#Tony"))
            {
                if( ImGui::MenuItem("Tony is God"))
                {
                    
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        //render file dialog
        fileDialog.Display();
        //select function
        if(fileDialog.HasSelected())
        {
            //std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
            std::ifstream inFile(fileDialog.GetSelected().string());
            if(!inFile)
            {
                std::cout<<"Couldn't open the file"<<std::endl;
                break;
            }
            std::string line;
            while (!inFile.eof())
            {
                if(getline(inFile,line))
                {
                    std::istringstream templine(line);
                    std::string data;
                    while (getline(templine,data,','))
                    {
                        matrix.push_back(atof(data.c_str()));
                    }
                }
                
            }
            for(int i=0;i<5;++i)
            {
                std::cout<< matrix[i] << std::endl;
            }
            
            // std::string line;   
            // if ( getline( inFile, line )) {
        
            //     std::stringstream str(line);
            //     float num;
            //     str >> num;
            //     x = num;
            // }
            // std::string line;
            // std::string arr[5];
            // int loop = 0;
            // while (! inFile.eof())
            // {
            //     getline(inFile,line);
            //     arr[loop] = line;
            //     std::cout<< arr[loop] << std::endl;
            //     loop++;
            // }
            inFile.close();

            fileDialog.ClearSelected();
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            ImGui::Begin("TM Visualizer option");                          // Create a window called "Hello, world!" and append into it.

            // ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            // ImGui::Text("Try to add something.");
            // ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &show_another_window);
            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::Checkbox("TM Visualizer",&show_visualizer);
            ImGui::Text("Change a color you like!");
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
            // if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //     counter++;
            // ImGui::SameLine();
            // ImGui::Text("counter = %d", counter);

            
            ImGui::End();
        }
        //ImPlot::ShowDemoWindow();
        // if(ImGui::Begin("dummy window"))
        // {
        //     // open file dialog when user clicks this button
        //     if(ImGui::Button("open file dialog"))
        //         fileDialog.Open();
        // }
        // ImGui::End();

        if (show_visualizer)
        {
            ImGui::Begin("TM Visualizer", &show_visualizer);
            ImGui::SliderFloat("x",&x,0.f,1080.f);
            ImGui::Checkbox("RAW DATA",&raw);
            ImGui::Checkbox("every 100 transactions",&every_100);
            ImPlot::SetNextPlotLimitsY(-0.5, 3.5);
            ImPlot::SetNextPlotLimitsX(0, 1000);
            
            if (ImPlot::BeginPlot("Real time visualizer",NULL,NULL,ImVec2(-1,0),0,ImPlotAxisFlags_RangeFit)) 
            {
                // ImPlot::PlotBarsH("My visualizer",x_data,11,1,1,1,1);
                // ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                // ImPlot::PlotLine("x^2", x_data, y_data, 11);
                
                static bool flag = false;
                if(!flag){
                    //thread
                    ImPlot2::Random_t_id(size,thread_id);
                    //xs and sxx
                    ImPlot2::Random_xs(size,xs,xss,10000);
                    //event
                    ImPlot2::Random_event(size,event_type);
                    flag = true;
                }

                static ImVec4 bullCol = ImVec4(0.000f, 1.000f, 0.441f, 1.000f);
                static ImVec4 bearCol = ImVec4(0.853f, 0.050f, 0.310f, 1.000f);
                
                ImPlot2::PlotTM("Kmeans",xs, xss, thread_id, event_type, size, bullCol, bearCol);
                
                ImPlot::EndPlot();
            }


            
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

