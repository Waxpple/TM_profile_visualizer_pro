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
            ImU32 color      = ImGui::GetColorU32(event_type[i] == 1 ? commit : abort);
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
// void PlotTM2(const char* label_id, std::vector<double>& xs, std::vector<double>& xss , std::vector<double>& thread_id, std::vector<double>* event_type, int count, ImVec4 commit, ImVec4 abort) {

//     // get ImGui window DrawList
//     ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    
//     // begin plot item
//     if (ImPlot::BeginItem(label_id)) {
//         // override legend icon color
//         ImPlot::GetCurrentItem()->Color = IM_COL32(64,64,64,255);
//         for (int i = 0; i < count; ++i) {
//             ImVec2 open_pos  = ImPlot::PlotToPixels(xs[i], thread_id->at(i) -0.5);
//             ImVec2 close_pos = ImPlot::PlotToPixels(xss[i], thread_id->at(i)+0.5 );
//             ImU32 color      = ImGui::GetColorU32(event_type->at(i) == 1.0 ? commit : abort);
//             draw_list->AddRectFilled(open_pos, close_pos, color);
//         }

//         // end plot item
//         ImPlot::EndItem();
//     }
// }
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

void vector_Random_t_id(int size,std::vector<double>* arr){
    for(int i=0;i<size;++i){
        arr->push_back(rand() % 4);
    }
}
void vector_Random_xs(int size,std::vector<double>* xs,std::vector<double>* xss, int limit){
    for(int i=0;i<size;++i){
        xs->push_back(rand() % (limit*5) );
        xss->push_back(xs->at(i) + rand() %200 /20.0);
    }
}
void vector_Random_event(int size,std::vector<double>* event){
    for(int i=0;i<size;++i){
        event->push_back(rand() % 2);
        
    }
}

void pairsort(double* a, double* b, double* c, double* d, int n)
{
    std::pair<double, std::pair<double, std:: pair <double,double> > > pairt[n];
  
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
    fileDialog.SetTypeFilters({ ".*", ".txt" });

    int size = 5;
    std::vector<double> thread_id;
    std::vector<double> xs;
    std::vector<double> xss;
    std::vector<double> event_type;
    std::vector<double> analysis_thread_id;
    std::vector<double> analysis_xs;
    std::vector<double> analysis_xss;
    std::vector<double> analysis_event_type;
    bool raw = true;
    bool every_100 = false;
    float every_transactions = 10.0;
    std::vector<double> matrix;


    //Green
    static ImVec4 bullCol = ImVec4(0.000f, 1.000f, 0.441f, 1.000f);
    //Red
    static ImVec4 bearCol = ImVec4(0.853f, 0.050f, 0.310f, 1.000f);
    //test sorting
    // std::vector<double> t_id = {5.0,4.0,3.0,2.0,1.0};
    // std::vector<double> t_time = {15.0,11.0,8.0,7.0,2.0};
    // std::vector<double> t_stop = {16.0,17.0,9.0,8.0,3.0};
    // std::vector<double> t_abort = {1.0,0.0,1.0,0.0,1.0};
    // ImPlot2::pairsort(t_time.data(),t_id.data(),t_stop.data(),t_abort.data(),t_time.size());
    // for(int i =0; i<5;++i)
    // {
    //     std::cout << "thread: " << t_id[i] << "time" << t_time[i] << "stop" << t_stop[i] << "type" << t_abort[i] << std::endl;
    // }



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
            //std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
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
                        switch(place%4)
                        {
                            case 0:
                                thread_id.push_back(atof(data.c_str()));
                                #ifdef DEBUG
                                std::cout<< "thread=" <<data.c_str()<<std::endl;
                                #endif
                                break;
                            case 1:
                                xs.push_back(atof(data.c_str()));
                                #ifdef DEBUG
                                std::cout<< "start=" <<data.c_str()<<std::endl;
                                #endif
                                break;
                            case 2:
                                xss.push_back(atof(data.c_str()));
                                #ifdef DEBUG
                                std::cout<< "stop=" <<data.c_str()<<std::endl;
                                #endif
                                break;
                            case 3:
                                event_type.push_back(atof(data.c_str()));
                                #ifdef DEBUG
                                std::cout<< "event=" <<data.c_str()<<std::endl;
                                #endif
                                break;
                        }
                        place++;
                    }
                }     
            }
            // Close file
            inFile.close(); 
            fileDialog.ClearSelected();

            // Sort time
            ImPlot2::pairsort(xs.data(),thread_id.data(),xss.data(),event_type.data(),xs.size());

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
            ImGui::Checkbox("RAW DATA",&raw);
            // ImGui::SameLine();
            // ImGui::Text("");
            ImGui::Checkbox("every transactions",&every_100);
            ImGui::SameLine();
            ImGui::SliderFloat("x",&every_transactions,10.0,1000.0);
            ImPlot::SetNextPlotLimitsY(-1.0, 6.5);
            ImPlot::SetNextPlotLimitsX(-100, 1000);
            
            if (ImPlot::BeginPlot("Real time visualizer",NULL,NULL,ImVec2(-1,0),0,ImPlotAxisFlags_RangeFit)) 
            {
                // ImPlot::PlotBarsH("My visualizer",x_data,11,1,1,1,1);
                // ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                // ImPlot::PlotLine("x^2", x_data, y_data, 11);
                
                static bool flag = false;

                //initalize the plots
                if(!flag){
                    //thread
                    ImPlot2::vector_Random_t_id(size,&thread_id);
                    //xs and sxx
                    ImPlot2::vector_Random_xs(size,&xs,&xss,10);
                    //event
                    ImPlot2::vector_Random_event(size,&event_type);
                    flag = true;
                }
                
                ImPlot2::PlotTM("Kmeans",xs.data(), xss.data(), thread_id.data(), event_type.data(), event_type.size(), bullCol, bearCol);

                if(every_100)
                {   
                    analysis_thread_id.clear();
                    analysis_xs.clear();
                    analysis_xss.clear();
                    analysis_event_type.clear();

                    for(int i =0; i<floor(xs.back()/every_transactions); ++i )
                    {
                        std::vector<double>::iterator lower,upper;
                        lower = std::lower_bound (xs.begin(), xs.end(), every_transactions*i);
                        upper = std::upper_bound (xs.begin(), xs.end(), every_transactions*(i+1)+0.1);
                        int start = lower-xs.begin();
                        int stop = upper-xs.begin();
                        double every_type[6][2];

                        //std::cout << "Now" << every_transactions << "lower" << start  << " upper" << stop << std::endl;
                        for(int j=start;j<stop;++j)
                        {
                            every_type[(int)thread_id[j] ][1] += event_type[j];
                            every_type[(int)thread_id[j] ][2] += 1;
                            //std::cout << "thread_id" << (int)thread_id[j] << "sum" << every_type[(int)thread_id[j]][2] << std::endl;
                        }
                        for(int j=0;j<6;++j)
                        {
                            every_type[j][1] /= every_type[j][2];
                            analysis_thread_id.push_back( (double)j -6 );
                            analysis_event_type.push_back( every_type[j][1]);
                            analysis_xs.push_back(every_transactions*i);
                            analysis_xss.push_back(every_transactions*(i+1)+0.1);
                            
                            //std::cout << "thread_id" << j << "average" << every_type[j][1] << std::endl;
                        }
                        for(int j=0;j<6;++j)
                        {

                            every_type[j][1] =0;
                            every_type[j][2] =0;
                        }

                    }
                    std::cout<< "the size should be" << floor(event_type.size()/every_transactions) << std::endl;
                    ImPlot2::PlotTM2("Analysis",analysis_xs.data(), analysis_xss.data(), analysis_thread_id.data(), analysis_event_type.data(), analysis_event_type.size(), bullCol, bearCol);
                    
                    // for(int i =0;i<100;++i)
                    // {
                    //     std::cout << "thread: " << thread_id[i] << "time" << xs[i] << "stop" << xss[i] << "type" << event_type[i] << std::endl;
                    // }
                }
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

