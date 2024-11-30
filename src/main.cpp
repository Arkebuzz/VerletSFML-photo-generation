#include <filesystem>
#include <iostream>

#include "engine/window_context_handler.hpp"
#include "engine/common/color_utils.hpp"

#include "physics/physics.hpp"
#include "thread_pool/thread_pool.hpp"
#include "renderer/renderer.hpp"
#include "generators.hpp"

std::vector<GravityFN>   gravities {gravity_normal, gravity_central, gravity_central_n};
std::vector<std::string> gr_names  {"normal", "uneven_central", "uniform_central"};
std::vector<func>        presets   {single, single, dual, dual_rev, quadruple, quadruple_rev};
std::vector<std::string> pr_names  {"single", "dual", "quadruple"};


int main(int argc, char **argv) {
    // Setting starting parameters
    const char *FFMPEG = "ffmpeg.exe";
    const char *IMG = "";
    int WORLD_WIDTH = 200;
    int WORLD_HEIGHT = 200;
    int CIRCLES_LIMIT = 26000;
    int DELAY_DESTROY = 120;
    int DELAY_REVERSE = 60;
    int PRESET = 4;
    int NUM = 15;
    float STEP = 0.0f;
    int GRAVITY = 1;
    float GR_FORCE = 1.0f;
    int FPS = 30;
    int WINDOWS_HEIGHT, WINDOWS_WIDTH;
    bool DISABLE_BORDERS = true;
    
    for (int i = 1; i < argc; i++) {
        std::string str = argv[i];
        if (str == "-ff")
            FFMPEG = argv[i + 1];
        else if (str == "-i")
            IMG = argv[i + 1];
        else if (str == "-ww")
            WORLD_WIDTH = atoi(argv[i+1]);
        else if (str == "-wh")
            WORLD_HEIGHT = atoi(argv[i+1]);
        else if (str == "-c")
            CIRCLES_LIMIT = atoi(argv[i+1]);
        else if (str == "-d")
            DISABLE_BORDERS = false;
        else if (str == "-dd")
            DELAY_DESTROY = atoi(argv[i+1]);
        else if (str == "-dr")
            DELAY_REVERSE = atoi(argv[i+1]);
        else if (str == "-p")
            PRESET = atoi(argv[i+1]) * 2;
        else if (str == "-n")
            NUM = atoi(argv[i+1]);
        else if (str == "-s")
            STEP = atof(argv[i+1]);
        else if (str == "-g")
            GRAVITY = atoi(argv[i+1]);
        else if (str == "-gf")
            GR_FORCE = atof(argv[i+1]);
        else if (str == "-f")
            FPS = atoi(argv[i+1]);
    }
    
    WORLD_WIDTH  = WORLD_WIDTH  > 0 ? WORLD_WIDTH : 150;
    WORLD_HEIGHT = WORLD_HEIGHT > 0 ? WORLD_HEIGHT : 150;
    CIRCLES_LIMIT = CIRCLES_LIMIT > 0 ? CIRCLES_LIMIT : WORLD_WIDTH * WORLD_HEIGHT;
    DELAY_DESTROY = DELAY_DESTROY > 0 ? DELAY_DESTROY : 120;
    DELAY_REVERSE = DELAY_REVERSE > 0 ? DELAY_REVERSE : 60;
    PRESET  = PRESET >= 0 && PRESET < presets.size() ? PRESET : 0;
    NUM  = NUM  > 0 ? NUM  : 15;
    STEP = STEP > 0 ? STEP : 0.0f;
    GRAVITY = GRAVITY >= 0 && GRAVITY < gravities.size() ? GRAVITY : 0;
    FPS = FPS > 0 ? FPS : 30;

    if (WORLD_HEIGHT > WORLD_WIDTH) {
        WINDOWS_HEIGHT = 1000;
        WINDOWS_WIDTH = 1000 * WORLD_HEIGHT / WORLD_WIDTH;
    }
    else {
        WINDOWS_WIDTH = 1000;
        WINDOWS_HEIGHT = 1000 * WORLD_HEIGHT / WORLD_WIDTH;
    }
    
    std::cout << "Starting settings:\n";
    std::cout << "world size     " << WORLD_WIDTH << "x" << WORLD_HEIGHT << "\n";
    std::cout << "circles limit  " << CIRCLES_LIMIT << "\n";
    std::cout << "preset         " << pr_names[PRESET / 2] << "\n";
    std::cout << "number emit    " << NUM << "\n";
    std::cout << "step           " << STEP << "\n";
    std::cout << "gravity type   " << gr_names[GRAVITY] << "\n";
    std::cout << "gravity force  " << GR_FORCE << "\n";
    std::cout << "destroy image  " << (DISABLE_BORDERS ? "true" : "false") << "\n";
    std::cout << "delay destruct " << DELAY_DESTROY << "\n";
    std::cout << "delay reverse  " << DELAY_REVERSE << "\n";
    std::cout << "FPS            " << FPS << "\n";
    // std::cout << "window size    " << WINDOWS_WIDTH << "x" << WINDOWS_HEIGHT << "\n";
    std::cout << "image          " << IMG << "\n";
    std::cout << "ffmpeg         " << FFMPEG << "\n";
    
    // Loading an image and preparing the output directory
    sf::Image img;
    if (!img.loadFromFile(IMG)) {
        return 1;
    }

    float kx = (float) img.getSize().x / WORLD_WIDTH;
    float ky = (float) img.getSize().y / WORLD_HEIGHT;
    
    std::filesystem::remove_all("images");
    std::filesystem::create_directory("images");

    // Initialize solver and renderer
    WindowContextHandler app("Verlet-MultiThread", sf::Vector2u(WINDOWS_WIDTH, WINDOWS_HEIGHT), sf::Style::Default);
    RenderContext &render_context = app.getRenderContext();
    
    tp::ThreadPool thread_pool(10);
    const IVec2 world_size{WORLD_WIDTH, WORLD_HEIGHT};
    PhysicSolver solver{world_size, thread_pool, gravities[GRAVITY], GR_FORCE};
    Renderer renderer(solver, thread_pool);

    const float margin = 1.5f;
    const float zoom = (WINDOWS_HEIGHT - margin) / world_size.y;
    render_context.setZoom(zoom);
    render_context.setFocus({world_size.x * 0.5f, world_size.y * 0.5f});

    // Preparation for the main loop
    float num = NUM;
    bool emit = true;
    bool reverse = false;
    bool borders_collision = true;
    bool recording = false;
    int delay_reverse = DELAY_REVERSE;
    int delay_destroy = DELAY_DESTROY;
    uint64_t screen_name = 0;
    sf::Color *colors = new sf::Color[CIRCLES_LIMIT + 500];

    // Main loop
    const uint32_t fps_cap = 60;
    const float dt = 1.0f / static_cast<float>(fps_cap);
    while (app.run()) {
        // Change the direction of the circles release and delay
        if (delay_reverse > 0 && solver.objects.size() > CIRCLES_LIMIT * 0.7f) {
            reverse = true;
            delay_reverse -= 1;
            num = NUM;
        }
        // Creating new circles
        else if (emit && solver.objects.size() < CIRCLES_LIMIT) {
            presets[PRESET + (int) reverse](solver, colors, num);
            num += STEP;
        }
        // Setting colors from an image
        else if (delay_destroy == 0) {
            for (int i = 0; i < solver.objects.size(); i++) {
                int x = solver.objects[i].position.x * kx;
                int y = solver.objects[i].position.y * ky;
                colors[i] = img.getPixel(x, y);
            }
        
            img.~Image();
            solver.objects.clear();

            num = NUM;
            delay_destroy -= 1;
            delay_reverse = DELAY_REVERSE;
            recording = true;
            reverse = false;
        }
        // Start destroying the image
        else if (DISABLE_BORDERS && delay_destroy == -DELAY_DESTROY - 1) {
            borders_collision = emit = false;
            solver.gravity_force = -1;
            solver.gravity = gravity_central;
            delay_destroy -= 1;
        }
        // Completion of the program
        else if (delay_destroy == -DELAY_DESTROY - 1 || solver.objects.size() == 0) {
            app.~WindowContextHandler();
            std::string command = FFMPEG;
            command += " -r " + std::to_string(FPS) + " -i images\\%d.jpg res.mp4";
            system(command.c_str());
        }
        // Increasing gravity when the image is destroyed
        else if (!borders_collision && !emit) {
            solver.gravity_force *= 1.1f;
        }
        // Reducing the delay
        else if (solver.objects.size() >= CIRCLES_LIMIT && delay_destroy > -DELAY_DESTROY - 1) {
            delay_destroy -= 1;
        }

        solver.update(dt, borders_collision);

        render_context.clear();
        renderer.render(render_context);
        render_context.display();

        if (recording) {
            app.copyScreen().saveToFile("images\\" + std::to_string(screen_name) + ".jpg");
            screen_name += 1;
        }
    }

    return 0;
}
