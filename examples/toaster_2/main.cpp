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
#include <bits/stdc++.h>
#include <algorithm>
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

// USER DEFINE!
#define NUMBER_OF_CORE 4
#define RECORD_N 4

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
// Encapsulates examples for customizing ImPlot.
namespace ImPlot2 {


int BinarySearch(unsigned long long int* arr, unsigned long long int* arrs, double* thread_id, int l, int r, double x, double y) {
    if (r >= l) {
        int mid = l + (r - l) / 2;
        #ifdef DEBUG
            printf("the x now is %f\n",x);
            printf("the mid now is %d\n",mid);
        #endif
        if (arrs[mid] > x && arr[mid] < x && thread_id[mid] == y )
            return mid;
        else
            for (int i=-NUMBER_OF_CORE;i<NUMBER_OF_CORE;++i)
            {   
                if(mid+i>-1 && mid+i<l+1)
                    if (arrs[mid+i] > x && arr[mid+i] < x && thread_id[mid+i] == y )
                        return mid+i;
            }
        if (arr[mid] > x)
            return BinarySearch(arr, arrs, thread_id, l, mid - 1, x, y);
        return BinarySearch(arr, arrs, thread_id, mid + 1, r, x, y);
    }
    return -1;
}
void PlotTM(char* label_id, unsigned long long int* xs, unsigned long long int* xss , double* thread_id, int* event_type , int count, ImVec4 rollback, ImVec4 delay, ImVec4 commit) {

    // get ImGui window DrawList
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    // ImPlot::FitPointAxis(GImPlot->CurrentPlot->XAxis,GImPlot->ExtentsX, (double) xs[0]);
    // ImPlot::FitPointAxis(GImPlot->CurrentPlot->XAxis,GImPlot->ExtentsX, (double) xs[count-1]);
    //tool
    if (ImPlot::IsPlotHovered())
    {
        ImPlotPoint mouse   = ImPlot::GetPlotMousePos();
        int idx =0;
        idx = BinarySearch(xs, xss, thread_id, 0, count-1, mouse.x, round(mouse.y));
        #ifdef DEBUG
            printf("The idx is %d \n\n",idx);
        #endif
        if (idx!=-1){
            ImGui::BeginTooltip();
            
            ImGui::Text("Thread:    %f",thread_id[idx]);
            ImGui::Text("Start:     %lld ns",xs[idx]);
            ImGui::Text("Stop:     %lld ns",xss[idx]);
            ImGui::Text("Duration:  %lld ns",xss[idx]-xs[idx]);
            ImGui::Text("Event:     %s", (event_type[idx]==0)? "rollback" : (event_type[idx]==1)? "delay" : "commit");
            
            ImGui::EndTooltip();
        }
        //printf("The mouse is currently at x:%f\n\n",mouse.x);
    }
    // begin plot item
    if (ImPlot::BeginItem(label_id)) {
        // override legend icon color
        ImPlot::GetCurrentItem()->Color = IM_COL32(64,64,64,255);
        for (int i = 0; i < count; ++i) {
            ImVec2 open_pos  = ImPlot::PlotToPixels(xs[i], thread_id[i]-0.5);
            ImVec2 close_pos = ImPlot::PlotToPixels(xss[i], thread_id[i]+0.5);
            ImU32 color      = ImGui::GetColorU32(event_type[i] == 0 ? rollback : (event_type[i]==1)? delay : commit );
            draw_list->AddRectFilled(open_pos, close_pos, color);
        }

        // end plot item
        ImPlot::EndItem();
    }
}
void PlotTM2(const char* label_id, const double* xs, const double* xss , const double* thread_id, const double* event_type, int count, ImVec4 commit, ImVec4 abort) {

    // get ImGui window DrawList
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    
    // begin plot item
    if (ImPlot::BeginItem(label_id)) {
        // override legend icon color
        ImPlot::GetCurrentItem()->Color = IM_COL32(64,64,64,255);
        for (int i = 0; i < count; ++i) {
            ImVec2 open_pos  = ImPlot::PlotToPixels(xs[i], thread_id[i]-0.5);
            ImVec2 close_pos = ImPlot::PlotToPixels(xss[i], thread_id[i]+0.5);
            ImU32 color      = ImGui::GetColorU32(ImVec4(0.853f * (1-event_type[i]), 0.050f + 0.95 * event_type[i], 0.310f + 0.131* event_type[i], 1.000f));
            draw_list->AddRectFilled(open_pos, close_pos, color);
        }

        // end plot item
        ImPlot::EndItem();
    }
}
template<typename T>
void find_min_max(std::vector<T>* xs,std::vector<T>* xss,T* min, T* max)
{
    *min = *min_element(xs->begin(),xs->end());
    *max = *max_element(xss->begin(),xss->end());
    
}

void pairsort(unsigned long long int* a, double* b, unsigned long long int* c, int* d, int n)
{
    std::pair<unsigned long long int, std::pair<double, std:: pair <unsigned long long int, int > > > pairt[n];
  
    // Storing the respective array
    // elements in pairs.
    for (int i = 0; i < n; i++) 
    {
        pairt[i].first = a[i];
        pairt[i].second.first = b[i];
        pairt[i].second.second.first = c[i];
        pairt[i].second.second.second = d[i];
        
    }
  
    // Sorting the pair array.
    std::sort(pairt, pairt + n);
      
    // Modifying original arrays
    for (int i = 0; i < n; i++) 
    {
        a[i] = pairt[i].first;
        b[i] = pairt[i].second.first;
        c[i] = pairt[i].second.second.first;
        d[i] = pairt[i].second.second.second;

    }
}

void pairsort_on_core(double* a, unsigned long long int* b, unsigned long long int* c, int* d, int n)
{
    std::pair<double, std::pair<unsigned long long int, std:: pair <unsigned long long int, int > > > pairt[n];
  
    // Storing the respective array
    // elements in pairs.
    for (int i = 0; i < n; i++) 
    {
        pairt[i].first = a[i];
        pairt[i].second.first = b[i];
        pairt[i].second.second.first = c[i];
        pairt[i].second.second.second = d[i];
        
    }
  
    // Sorting the pair array.
    std::sort(pairt, pairt + n);
      
    // Modifying original arrays
    for (int i = 0; i < n; i++) 
    {
        a[i] = pairt[i].first;
        b[i] = pairt[i].second.first;
        c[i] = pairt[i].second.second.first;
        d[i] = pairt[i].second.second.second;

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

    // create a file browser instance
    ImGui::FileBrowser fileDialog;
    
    // (optional) set browser properties
    fileDialog.SetTitle("Choose event file");
    fileDialog.SetTypeFilters({ ".txt", ".log",".*" });

    int size = 5;
    std::vector<double> thread_id;
    std::vector<unsigned long long int> xs;
    std::vector<unsigned long long int> xss;
    std::vector<int> event_type;
    // std::vector<long long int> memory_addr;
    std::vector<double> core_0_analysis_thread_id;
    std::vector<unsigned long long int> core_0_analysis_xs;
    std::vector<unsigned long long int> core_0_analysis_xss;
    std::vector<double> core_0_analysis_event_type;
    std::vector<double> core_0_center_point;
    std::vector<double> core_0_abort_rate;
    std::vector<double> core_0_commit_rate;
    std::vector<double> core_0_average_duration;

    std::vector<double> core_1_analysis_thread_id;
    std::vector<unsigned long long int> core_1_analysis_xs;
    std::vector<unsigned long long int> core_1_analysis_xss;
    std::vector<double> core_1_analysis_event_type;
    std::vector<double> core_1_center_point;
    std::vector<double> core_1_abort_rate;
    std::vector<double> core_1_commit_rate;
    std::vector<double> core_1_average_duration;


    std::vector<double> core_2_analysis_thread_id;
    std::vector<unsigned long long int> core_2_analysis_xs;
    std::vector<unsigned long long int> core_2_analysis_xss;
    std::vector<double> core_2_analysis_event_type;
    std::vector<double> core_2_center_point;
    std::vector<double> core_2_abort_rate;
    std::vector<double> core_2_commit_rate;
    std::vector<double> core_2_average_duration;
    
    std::vector<double> core_3_analysis_thread_id;
    std::vector<unsigned long long int> core_3_analysis_xs;
    std::vector<unsigned long long int> core_3_analysis_xss;
    std::vector<double> core_3_analysis_event_type;
    std::vector<double> core_3_center_point;
    std::vector<double> core_3_abort_rate;
    std::vector<double> core_3_commit_rate;
    std::vector<double> core_3_average_duration;
    //
    // std::vector<double> center_point;
    // std::vector<double> abort_rate;

    //
    std::vector<double> core_list;
    std::vector<int> core_index;
    
    // std::vector<double> FGL_thread_id;
    // std::vector<double> FGL_xs;
    // std::vector<double> FGL_xss;
    // std::vector<double> FGL_event_type;
    // std::vector<long long int> FGL_memory_addr;

    //bool
    bool readfile = false;


    bool raw = true;
    bool every_ns = false;
    bool calculate_0 = false;


    bool FGL = false;
    bool flag2 = true;
    float every_transactions = 10.0;
    std::vector<double> matrix;
    static bool initialized_plot = false;

    //Green
    static ImVec4 commit = ImVec4(0.000f, 1.000f, 0.000f, 1.000f);
    //Red
    static ImVec4 rollback = ImVec4(1.000f, 0.000f, 0.000f, 1.000f);
    //delay
    static ImVec4 delay = ImVec4(0.000f, 0.000f, 1.000f, 1.000f);

    //statics
    long long unsigned int *time_min = (long long unsigned int*) malloc(sizeof(long long unsigned int));
    long long unsigned int *time_max= (long long unsigned int*) malloc(sizeof(long long unsigned int));
    *time_min = 0;
    *time_max = 2200;
    double *core_min = (double*) malloc(sizeof(double));
    double *core_max = (double*) malloc(sizeof(double));
    *core_min = -2.0;
    *core_max = 10.0;

    // initial smilie face
    thread_id.push_back(5);
    thread_id.push_back(5);
    thread_id.push_back(5);
    thread_id.push_back(5);
    thread_id.push_back(2);
    thread_id.push_back(1);
    thread_id.push_back(0);
    thread_id.push_back(1);
    thread_id.push_back(2);

    xs.push_back(1100);
    xs.push_back(1150);
    xs.push_back(1300);
    xs.push_back(1350);
    xs.push_back(1150);
    xs.push_back(1100);
    xs.push_back(1150);
    xs.push_back(1350);
    xs.push_back(1400);

    xss.push_back(1150);
    xss.push_back(1200);
    xss.push_back(1350);
    xss.push_back(1400);
    xss.push_back(1100);
    xss.push_back(1150);
    xss.push_back(1350);
    xss.push_back(1400);
    xss.push_back(1450);

    event_type.push_back(0);
    event_type.push_back(0);
    event_type.push_back(0);
    event_type.push_back(0);
    event_type.push_back(0);
    event_type.push_back(1);
    event_type.push_back(1);
    event_type.push_back(1);
    event_type.push_back(1);

    

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
                }
                if (ImGui::MenuItem("Quit", "Alt+F4")) break;

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools"))
            {
                if( ImGui::MenuItem("Visualizer",show_visualizer?"O":"X") ){
                    
                    show_visualizer = !show_visualizer;
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
            
            //new FGL
            flag2 = true;

            #ifdef DEBUG
            std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
            #endif

            std::ifstream inFile(fileDialog.GetSelected().string());
            if(!inFile)
            {
                std::cout<<"Couldn't open the file"<<std::endl;
                break;
            }
            std::string line;
            // clear our vector
            thread_id.clear();
            xs.clear();
            xss.clear();
            event_type.clear();
            while (!inFile.eof())
            {
                if(getline(inFile,line))
                {
                    std::istringstream templine(line);
                    std::string data;
                    int place=0;
                    while (getline(templine,data,','))
                    {   
                        switch(place%RECORD_N)
                        {
                            case 0:
                                thread_id.push_back(atof(data.c_str()));
                                #ifdef DEBUG
                                std::cout<< "thread=" <<data.c_str()<<std::endl;
                                #endif
                                break;
                            case 1:
                                xs.push_back(atoll(data.c_str()));
                                #ifdef DEBUG
                                std::cout<< "start=" <<data.c_str()<<std::endl;
                                #endif
                                break;
                            case 2:
                                xss.push_back(atoll(data.c_str()));
                                #ifdef DEBUG
                                std::cout<< "stop=" <<data.c_str()<<std::endl;
                                #endif
                                break;
                            case 3:
                                event_type.push_back(atoi(data.c_str()));
                                #ifdef DEBUG
                                std::cout<< "event=" <<data.c_str()<<std::endl;
                                #endif
                                break;                           
                            default:
                                break;
                        }
                        place++;
                    }
                    if(place%RECORD_N!=0)
                    {
                        std::cout<< "Error[1] File format misfit." << std::endl;
                        thread_id.clear();
                        xs.clear();
                        xss.clear();
                        event_type.clear();
                        thread_id.push_back(6);
                        thread_id.push_back(5);
                        thread_id.push_back(6);
                        thread_id.push_back(4);
                        thread_id.push_back(4);
                        thread_id.push_back(6);
                        thread_id.push_back(5);
                        thread_id.push_back(6);
                        thread_id.push_back(4);
                        thread_id.push_back(4);
                        thread_id.push_back(0);
                        thread_id.push_back(1);
                        thread_id.push_back(2);
                        thread_id.push_back(1);
                        thread_id.push_back(0);
                        xs.push_back(50);
                        xs.push_back(100);
                        xs.push_back(150);
                        xs.push_back(150);
                        xs.push_back(50);
                        xs.push_back(300);
                        xs.push_back(350);
                        xs.push_back(400);
                        xs.push_back(400);
                        xs.push_back(300);
                        xs.push_back(50);
                        xs.push_back(100);
                        xs.push_back(150);
                        xs.push_back(350);
                        xs.push_back(400);
                        xss.push_back(100);
                        xss.push_back(150);
                        xss.push_back(200);
                        xss.push_back(200);
                        xss.push_back(100);
                        xss.push_back(350);
                        xss.push_back(400);
                        xss.push_back(450);
                        xss.push_back(450);
                        xss.push_back(350);
                        xss.push_back(100);
                        xss.push_back(150);
                        xss.push_back(350);
                        xss.push_back(400);
                        xss.push_back(450);
                        event_type.push_back(0);
                        event_type.push_back(0);
                        event_type.push_back(0);
                        event_type.push_back(0);
                        event_type.push_back(0);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        event_type.push_back(1);
                        break;
                        
                    }
                    else
                    {
                        calculate_0 = false;
                    }
                    
                }     
            }
            // Close file
            inFile.close(); 
            fileDialog.ClearSelected();

            ImPlot2::find_min_max(&xs,&xss,time_min,time_max);
            ImPlot2::find_min_max(&thread_id,&thread_id,core_min,core_max);
            
            for (int i=0;i<(int)xs.size();++i)
            {
                xs[i] -= *time_min;
                xss[i] -= *time_min;
                thread_id[i] -= *core_min;
            }
            
            for (int i=0;i<=(int)(*core_max-*core_min);++i)
            {
                if(std::find(thread_id.begin(),thread_id.end(),(double)i) != thread_id.end()) 
                {
                    core_list.push_back((double)i);
                }
            }
            std::ofstream output_file("./core_list.txt");
            for (const auto &e :core_list) output_file << e << "\n";




            // Sort time
            ImPlot2::pairsort_on_core(thread_id.data(),xs.data(),xss.data(),event_type.data(),xs.size());

            // Put them into different thread file
            std::vector<double>::iterator upper;
            for (int i=0;i<core_list.size();++i)
            {
                upper = std::upper_bound (thread_id.begin(), thread_id.end(), core_list[i]);
                int ans = upper-thread_id.begin();
                //printf("The end of core %0f is:%d\n\n", core_list[i] ,ans );
                core_index.push_back(ans);
            }
            for (int i=0;i<core_index.size();++i)
            {
                int start_point;
                if(i==0)
                    start_point = 0;
                else
                    start_point = core_index[i-1];
                int stop_point;
                stop_point = core_index[i];
                for(int j=start_point;j<stop_point;++j)
                {
                    switch (i)
                    {
                    case 0:
                        core_0_analysis_thread_id.push_back(thread_id[j]);
                        core_0_analysis_xs.push_back(xs[j]);
                        core_0_analysis_xss.push_back(xss[j]);
                        core_0_analysis_event_type.push_back(event_type[j]);
                        break;
                    case 1:
                        core_1_analysis_thread_id.push_back(thread_id[j]);
                        core_1_analysis_xs.push_back(xs[j]);
                        core_1_analysis_xss.push_back(xss[j]);
                        core_1_analysis_event_type.push_back(event_type[j]);
                        break;
                    case 2:
                        core_2_analysis_thread_id.push_back(thread_id[j]);
                        core_2_analysis_xs.push_back(xs[j]);
                        core_2_analysis_xss.push_back(xss[j]);
                        core_2_analysis_event_type.push_back(event_type[j]);
                        break;
                    case 3:
                        core_3_analysis_thread_id.push_back(thread_id[j]);
                        core_3_analysis_xs.push_back(xs[j]);
                        core_3_analysis_xss.push_back(xss[j]);
                        core_3_analysis_event_type.push_back(event_type[j]);
                        break;
                    default:
                        printf("[Error] Distribute thread into void!");
                        break;
                    }
                }
            }

            
        }
        ImPlot::ShowDemoWindow();
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            ImGui::Begin("TM Visualizer option");                          // Create a window called "Hello, world!" and append into it.
            ImGui::Checkbox("TM Visualizer",&show_visualizer);
            ImGui::Text("Change a color you like!");
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color            
            ImGui::End();
        }

        if (show_visualizer)
        {
            ImGui::Begin("TM Visualizer", &show_visualizer);
            ImPlot::SetNextPlotLimitsY(*core_min, *core_max ,ImGuiCond_Once);
            ImPlot::SetNextPlotLimitsX(*time_min, *time_max ,ImGuiCond_Once);


            ImGui::Checkbox("RAW DATA",&raw);
            ImGui::Checkbox("Calculate every",&every_ns);
            ImGui::SameLine();
            ImGui::SliderFloat("x",&every_transactions,1000.0,10000000.0);
            if (ImGui::IsItemActive())
            {
                calculate_0 = false;
            }   
            ImGui::SameLine();
            ImGui::Text(" ns");
                        
            if (ImPlot::BeginPlot("Real time visualizer","X","Y",ImVec2(-1,-1),0,ImPlotAxisFlags_RangeFit,ImPlotAxisFlags_RangeFit)) 
            {
                if(raw){
                    ImPlot2::PlotTM("Event",xs.data(), xss.data(), thread_id.data(), event_type.data(), event_type.size(), rollback, delay, commit);
                }

                if(every_ns)
                {   
                    
                   
                    if(!calculate_0)
                    {

                        calculate_0 = true;
                        core_0_center_point.clear();
                        core_0_abort_rate.clear();
                        core_0_commit_rate.clear();
                        core_0_average_duration.clear();

                        core_1_center_point.clear();
                        core_1_abort_rate.clear();
                        core_1_commit_rate.clear();
                        core_1_average_duration.clear();

                        core_2_center_point.clear();
                        core_2_abort_rate.clear();
                        core_2_commit_rate.clear();
                        core_2_average_duration.clear();
                        
                        core_3_center_point.clear();
                        core_3_abort_rate.clear();
                        core_3_commit_rate.clear();
                        core_3_average_duration.clear();
                        double abort_temp,commit_temp,center_temp,average_duration_temp;
                        
                        for(int j=0;j<core_list.size();++j)
                        {
                            switch (j)
                            {
                            case 0:
                                for(int i =0; i<floor(core_0_analysis_xs.back()/every_transactions); ++i )
                                {
                                    std::vector<long long unsigned int>::iterator lower,upper;
                                    lower = std::lower_bound (core_0_analysis_xs.begin(), core_0_analysis_xs.end(), every_transactions*i);
                                    upper = std::upper_bound (core_0_analysis_xs.begin(), core_0_analysis_xs.end(), every_transactions*(i+1)+0.1);
                                    int start = lower-core_0_analysis_xs.begin();
                                    int stop = upper-core_0_analysis_xs.begin();
                                    
                                    center_temp = (double)(core_0_analysis_xs[start] + core_0_analysis_xs[stop-1]) /2.0;
                                    abort_temp = 0.0;
                                    commit_temp =0.0;
                                    average_duration_temp = 0.0;
                                    for(int j=start;j<stop;j++)
                                    {
                                        if(core_0_analysis_event_type[j]==0)
                                        {
                                            abort_temp += 1.0;
                                        }
                                        if(core_0_analysis_event_type[j]==2)
                                        {
                                            commit_temp += 1.0;
                                        }
                                        average_duration_temp += core_0_analysis_xss[j] - core_0_analysis_xs[j];

                                    }
                                    if(stop!=start){
                                        abort_temp /= (double)(stop-start);
                                        commit_temp /= (double)(stop-start);
                                        average_duration_temp /= (double)(stop-start);
                                        core_0_center_point.push_back(center_temp);
                                        core_0_commit_rate.push_back(commit_temp);
                                        core_0_abort_rate.push_back(abort_temp);
                                        core_0_average_duration.push_back(average_duration_temp);
                                    }
                                }
                                break;
                            case 1:
                                for(int i =0; i<floor(core_1_analysis_xs.back()/every_transactions); ++i )
                                {
                                    std::vector<long long unsigned int>::iterator lower,upper;
                                    lower = std::lower_bound (core_1_analysis_xs.begin(), core_1_analysis_xs.end(), every_transactions*i);
                                    upper = std::upper_bound (core_1_analysis_xs.begin(), core_1_analysis_xs.end(), every_transactions*(i+1)+0.1);
                                    int start = lower-core_1_analysis_xs.begin();
                                    int stop = upper-core_1_analysis_xs.begin();
                                    
                                    center_temp = (double)(core_1_analysis_xs[start] + core_1_analysis_xs[stop-1]) /2.0;
                                    abort_temp = 0.0;
                                    commit_temp =0.0;
                                    average_duration_temp = 0.0;
                                    for(int j=start;j<stop;j++)
                                    {
                                        if(core_1_analysis_event_type[j]==0)
                                        {
                                            abort_temp += 1.0;
                                        }
                                        if(core_1_analysis_event_type[j]==2)
                                        {
                                            commit_temp += 1.0;
                                        }
                                        average_duration_temp += core_1_analysis_xss[j] - core_1_analysis_xs[j];

                                    }
                                    if(stop!=start){
                                        abort_temp /= (double)(stop-start);
                                        commit_temp /= (double)(stop-start);
                                        average_duration_temp /= (double)(stop-start);
                                        core_1_center_point.push_back(center_temp);
                                        core_1_commit_rate.push_back(commit_temp);
                                        core_1_abort_rate.push_back(abort_temp);
                                        core_1_average_duration.push_back(average_duration_temp);
                                    }
                                }
                                break;
                            case 2:
                                for(int i =0; i<floor(core_2_analysis_xs.back()/every_transactions); ++i )
                                {
                                    std::vector<long long unsigned int>::iterator lower,upper;
                                    lower = std::lower_bound (core_2_analysis_xs.begin(), core_2_analysis_xs.end(), every_transactions*i);
                                    upper = std::upper_bound (core_2_analysis_xs.begin(), core_2_analysis_xs.end(), every_transactions*(i+1)+0.1);
                                    int start = lower-core_2_analysis_xs.begin();
                                    int stop = upper-core_2_analysis_xs.begin();
                                    
                                    center_temp = (double)(core_2_analysis_xs[start] + core_2_analysis_xs[stop-1]) /2.0;
                                    abort_temp = 0.0;
                                    commit_temp =0.0;
                                    average_duration_temp = 0.0;
                                    for(int j=start;j<stop;j++)
                                    {
                                        if(core_2_analysis_event_type[j]==0)
                                        {
                                            abort_temp += 1.0;
                                        }
                                        if(core_2_analysis_event_type[j]==2)
                                        {
                                            commit_temp += 1.0;
                                        }
                                        average_duration_temp += core_2_analysis_xss[j] - core_2_analysis_xs[j];

                                    }
                                    if(stop!=start){
                                        abort_temp /= (double)(stop-start);
                                        commit_temp /= (double)(stop-start);
                                        average_duration_temp /= (double)(stop-start);
                                        core_2_center_point.push_back(center_temp);
                                        core_2_commit_rate.push_back(commit_temp);
                                        core_2_abort_rate.push_back(abort_temp);
                                        core_2_average_duration.push_back(average_duration_temp);
                                    }
                                }
                                break;
                            case 3:
                                for(int i =0; i<floor(core_3_analysis_xs.back()/every_transactions); ++i )
                                {
                                    std::vector<long long unsigned int>::iterator lower,upper;
                                    lower = std::lower_bound (core_3_analysis_xs.begin(), core_3_analysis_xs.end(), every_transactions*i);
                                    upper = std::upper_bound (core_3_analysis_xs.begin(), core_3_analysis_xs.end(), every_transactions*(i+1)+0.1);
                                    int start = lower-core_3_analysis_xs.begin();
                                    int stop = upper-core_3_analysis_xs.begin();
                                    
                                    center_temp = (double)(core_3_analysis_xs[start] + core_3_analysis_xs[stop-1]) /2.0;
                                    abort_temp = 0.0;
                                    commit_temp =0.0;
                                    average_duration_temp = 0.0;
                                    for(int j=start;j<stop;j++)
                                    {
                                        if(core_3_analysis_event_type[j]==0)
                                        {
                                            abort_temp += 1.0;
                                        }
                                        if(core_3_analysis_event_type[j]==2)
                                        {
                                            commit_temp += 1.0;
                                        }
                                        average_duration_temp += core_3_analysis_xss[j] - core_3_analysis_xs[j];

                                    }
                                    if(stop!=start){
                                        abort_temp /= (double)(stop-start);
                                        commit_temp /= (double)(stop-start);
                                        average_duration_temp /= (double)(stop-start);
                                        core_3_center_point.push_back(center_temp);
                                        core_3_commit_rate.push_back(commit_temp);
                                        core_3_abort_rate.push_back(abort_temp);
                                        core_3_average_duration.push_back(average_duration_temp);
                                    }
                                }
                                break;

                            default:
                                break;
                            }
                            
                        }
                        
                        // std::copy_if(thread_id.begin(),thread_id.end(),std::back_inserter(analysis_thread_id),[](double i){return i==0.0;});
                        // std::ofstream output_file("./example.txt");
                        // for (const auto &e :analysis_thread_id) output_file << e << "\n";
                    }else{
                        #ifdef DEBUG
                            printf("%d \n\n",abort_rate.size());
                        #endif
                        for(int j=0;j<core_list.size();++j)
                        {
                            switch (j)
                            {
                            case 0:
                                ImPlot::PlotLine("Core_0_abort_rate", core_0_center_point.data(), core_0_abort_rate.data(), core_0_abort_rate.size());
                                ImPlot::PlotLine("Core_0_commit_rate",core_0_center_point.data(),core_0_commit_rate.data(), core_0_commit_rate.size());
                                ImPlot::PlotLine("Core_0_average_duration",core_0_center_point.data(),core_0_average_duration.data(), core_0_commit_rate.size());
                                break;
                            case 1:
                                ImPlot::PlotLine("Core_1_abort_rate", core_1_center_point.data(), core_1_abort_rate.data(), core_1_abort_rate.size());
                                ImPlot::PlotLine("Core_1_commit_rate",core_1_center_point.data(),core_1_commit_rate.data(), core_1_commit_rate.size());
                                ImPlot::PlotLine("Core_1_average_duration",core_1_center_point.data(),core_1_average_duration.data(), core_1_commit_rate.size());
                                break;
                            case 2:
                                ImPlot::PlotLine("Core_2_abort_rate", core_2_center_point.data(), core_2_abort_rate.data(), core_2_abort_rate.size());
                                ImPlot::PlotLine("Core_2_commit_rate",core_2_center_point.data(),core_2_commit_rate.data(), core_2_commit_rate.size());
                                ImPlot::PlotLine("Core_2_average_duration",core_2_center_point.data(),core_2_average_duration.data(), core_2_commit_rate.size());
                                break;
                            case 3:
                                ImPlot::PlotLine("Core_3_abort_rate", core_3_center_point.data(), core_3_abort_rate.data(), core_3_abort_rate.size());
                                ImPlot::PlotLine("Core_3_commit_rate",core_3_center_point.data(),core_3_commit_rate.data(), core_3_commit_rate.size());
                                ImPlot::PlotLine("Core_3_average_duration",core_3_center_point.data(),core_3_average_duration.data(), core_3_commit_rate.size());
                                break;
                            default:
                                printf("[Error] cannot draw analysis at abort_rate.");
                                break;
                            }
                        }
                        
                    }
                    //ImPlot2::PlotTM("Analysis",analysis_xs.data(), analysis_xss.data(), analysis_thread_id.data(), analysis_event_type.data(), analysis_event_type.size(), rollback, delay, commit);
                }
            //     if(FGL)
            //     {

            //         if(flag2){

            //             FGL_xs.clear();
            //             FGL_xss.clear();
            //             FGL_thread_id.clear();
            //             FGL_event_type.clear();
            //             //FGL_memory_addr.clear();


            //             for(int j=0;j<(int)event_type.size();j++)
            //             {
            //                 FGL_xs.push_back(xs[j]);
            //                 FGL_xss.push_back(xss[j]);
            //                 FGL_thread_id.push_back(thread_id[j]-6);
            //                 FGL_event_type.push_back(event_type[j]);
            //                 //FGL_memory_addr.push_back(memory_addr[j]);
            //             }

                        
            //             // double temp_xs = FGL_xs[0];
            //             // double temp_thread = FGL_thread_id[0];
            //             // long long int temp_memory_addr= FGL_memory_addr[0];
            //             // FGL_thread_id[0] = FGL_thread_id[0] -6;

            //             double record_xss[NUMBER_OF_CORE];
            //             for (int i=0;i<NUMBER_OF_CORE;++i)
            //             {
            //                 record_xss[i] = 0;
            //             }

            //             for(int i=1;i<(int)FGL_event_type.size();++i)
            //             {   
            //                 if(FGL_thread_id[i] != FGL_thread_id[i-1])
            //                 {
            //                     if(FGL_memory_addr[i] != FGL_memory_addr[i-1])
            //                     {
            //                         if(FGL_xs[i-1] > record_xss[(int)FGL_thread_id[i]])
            //                         {
            //                             FGL_xss[i] = FGL_xss[i]-(FGL_xs[i]-FGL_xs[i-1]);
            //                             FGL_xs[i] = FGL_xs[i-1];
            //                         }
            //                         else
            //                         {
            //                             FGL_xss[i] = FGL_xss[i]-(FGL_xs[i]-record_xss[(int)FGL_thread_id[i]]);
            //                             FGL_xs[i] = record_xss[(int)FGL_thread_id[i]];
            //                         }
            //                     }
            //                 }
            //                 record_xss[(int)FGL_thread_id[i]] = FGL_xss[i];
            //                 printf("Thread ID [%f] : start: %f stop: %f \n\n",FGL_thread_id[i],FGL_xs[i],FGL_xss[i]);
            //                 // temp_xs = FGL_xs[i];
            //                 // temp_thread = FGL_thread_id[i];
            //                 // temp_memory_addr = FGL_memory_addr[i];
            //                 // FGL_thread_id[i] = FGL_thread_id[i]-6;
            //             }
            //             flag2 = false;
            //         }
            //         ImPlot2::PlotTM("FGL",FGL_xs.data(),FGL_xss.data(),FGL_thread_id.data(),FGL_event_type.data(),FGL_memory_addr.data(),FGL_event_type.size(), bullCol, bearCol);
            //     }
                ImPlot::EndPlot();
            }

            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
    
    //free
    free(core_min);
    free(core_max);
    free(time_min);
    free(time_max);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

