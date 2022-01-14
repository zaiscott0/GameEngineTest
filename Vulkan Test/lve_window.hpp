#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace lve {
    
    class LveWindow{
        public:
        LveWindow( int w, int h, std::string name );
        ~LveWindow( );
        
        LveWindow( const LveWindow &) = delete;
        LveWindow &operator = ( const LveWindow & ) = delete;
        
        bool shouldClose( ) { return glfwWindowShouldClose( window ); }
        
        VkExtent2D getExtent ( ) { return { static_cast<uint32_t>(WIDTH), static_cast<uint32_t>(HEIGHT) };}
        
        bool wasWindowResized( ){ return frameBufferResized; };
        void resetWindowResizedFlag( ){ frameBufferResized = false; };
        
        GLFWwindow *getGLFWwindow( ) const { return window; }
        
        void createWindowSurface( VkInstance instance, VkSurfaceKHR *surface);
        
        
        
        
        private:
        static void frameBufferResizeCallback( GLFWwindow *window, int width, int height );
        void initWindow( );
        
        int HEIGHT;
        int WIDTH;
        bool frameBufferResized = false;
        
        std::string windowName;
        
        GLFWwindow *window;
        
        
        
    
    };
}
