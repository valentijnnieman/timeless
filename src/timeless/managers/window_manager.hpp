#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <vector>
#include "../event.hpp"

#include "../systems/geo_rendering_system.hpp"
#include "../systems/rendering_system.hpp"
#include "../systems/text_rendering_system.hpp"
#include "../systems/mouse_input_system.hpp"
#include "../systems/movement_system.hpp"
#include "../systems/npc_ai_system.hpp"

class WindowManager
{
private:
    float quad_verts[24] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

public:
    unsigned int framebuffer;
    unsigned int textureColorbuffer;
    unsigned int rbo;
    unsigned int ScreenVAO, ScreenVBO;
    std::shared_ptr<Shader> screen_shader;

    GLFWwindow *window;

    WindowManager()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        window = glfwCreateWindow(TESettings::SCREEN_X, TESettings::SCREEN_Y, "Timeless", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetWindowUserPointer(window, this);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }
        // glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        // glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        std::cout << "OpenGL Initialized!" << std::endl;
        glViewport(0, 0, TESettings::SCREEN_X, TESettings::SCREEN_Y);
        glfwSetWindowAspectRatio(window, 16, 9);

        glfwSetMouseButtonCallback(window, &mouse_button_callback);
        glfwSetScrollCallback(window, MouseInputSystem::scroll_callback);
    }
    void enable_screen_shader(std::shared_ptr<Shader> shader)
    {
        screen_shader = shader;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // glEnable(GL_BLEND);
        // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TESettings::SCREEN_X, TESettings::SCREEN_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, TESettings::SCREEN_X, TESettings::SCREEN_Y); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);            // now actually attach it

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Could not create Framebuffer!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenVertexArrays(1, &ScreenVAO);
        glGenBuffers(1, &ScreenVBO);
        glBindVertexArray(ScreenVAO);
        glBindBuffer(GL_ARRAY_BUFFER, ScreenVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), &quad_verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

        glViewport(0, 0, TESettings::SCREEN_X, TESettings::SCREEN_Y);
    }
    ~WindowManager()
    {
        glfwDestroyWindow(window);
        glDeleteVertexArrays(1, &ScreenVAO);
        glDeleteBuffers(1, &ScreenVBO);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteFramebuffers(1, &framebuffer);

        // Sound::unload();
    }

    void set_shader_time()
    {
        glUniform1f(glGetUniformLocation(screen_shader->ID, "time"), glfwGetTime());
    }

    void loop(ComponentManager &cm, GeoRenderingSystem &g_rendering_sys, RenderingSystem &rendering_sys, RenderingSystem &ui_rendering_sys, TextRenderingSystem &t_rendering_sys, TextRenderingSystem &ui_t_rendering_sys, MovementSystem &movement_sys, NpcAiSystem &ai_sys)
    {
        double t = 0.0;
        double dt = 1.0 / 60.0;

        double current_time = glfwGetTime();
        while (!glfwWindowShouldClose(window))
        {
            double new_time = glfwGetTime();
            double frame_time = new_time - current_time;
            current_time = new_time;

            while (frame_time > 0.0)
            {
                float delta_time = std::min(frame_time, dt);

                movement_sys.update(cm, window);
                ai_sys.update(cm);
                frame_time -= delta_time;
                t += delta_time;
            }
            /** render scene into framebuffer */
            if (screen_shader != nullptr)
            {
                // glEnable(GL_DEPTH_TEST);
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
                glClearColor(TESettings::SCREEN_COLOR.r, TESettings::SCREEN_COLOR.g, TESettings::SCREEN_COLOR.b, TESettings::SCREEN_COLOR.a);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
            // else
            // {
            //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
            //     glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // }

            g_rendering_sys.render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
            rendering_sys.render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
            ui_rendering_sys.render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
            t_rendering_sys.render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
            ui_t_rendering_sys.render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);

            if (screen_shader != nullptr)
            {
                /** render framebuffer as quad */
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // default buffer
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                screen_shader->use();
                set_shader_time();
                glBindVertexArray(ScreenVAO);
                // glDisable(GL_DEPTH_TEST);
                // glDisable(GL_BLEND);
                glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            // else
            // {
            //     glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    static void window_size_callback(GLFWwindow *window, int width, int height)
    {
        TESettings::rescale_window(width, height);
        WindowManager *wm = static_cast<WindowManager *>(glfwGetWindowUserPointer(window));
        wm->enable_screen_shader(wm->screen_shader);
        // glViewport(0, 0, 1920, 1080);
    }
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        // make sure the viewport matches the new window dimensions; note that width and
        // height will be significantly larger than specified on retina displays.
        // glViewport(0, 0, width, height);
    }
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            double xpos, ypos;
            // getting cursor position
            glfwGetCursorPos(window, &xpos, &ypos);
            MouseInputSystem::mouse_click_handler(new MouseEvent("LeftMousePress", glm::vec2(xpos, ypos)));
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            double xpos, ypos;
            // getting cursor position
            glfwGetCursorPos(window, &xpos, &ypos);
            MouseInputSystem::mouse_click_handler(new MouseEvent("LeftMouseRelease", glm::vec2(xpos, ypos)));
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            double xpos, ypos;
            // getting cursor position
            glfwGetCursorPos(window, &xpos, &ypos);
            MouseInputSystem::mouse_click_handler(new MouseEvent("RightMousePress", glm::vec2(xpos, ypos)));
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            double xpos, ypos;
            // getting cursor position
            glfwGetCursorPos(window, &xpos, &ypos);
            MouseInputSystem::mouse_click_handler(new MouseEvent("RightMouseRelease", glm::vec2(xpos, ypos)));
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
        {
            double xpos, ypos;
            // getting cursor position
            glfwGetCursorPos(window, &xpos, &ypos);
            MouseInputSystem::mouse_click_handler(new MouseEvent("MiddleMousePress", glm::vec2(xpos, ypos)));
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
        {
            double xpos, ypos;
            // getting cursor position
            glfwGetCursorPos(window, &xpos, &ypos);
            MouseInputSystem::mouse_click_handler(new MouseEvent("MiddleMouseRelease", glm::vec2(xpos, ypos)));
        }
    }
};