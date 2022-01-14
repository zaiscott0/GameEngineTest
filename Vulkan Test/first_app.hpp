#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include "lve_game_object.hpp"
#include "lve_descriptors.hpp"

// std
#include <memory>
#include <vector>

namespace lve {

class FirstApp{
    
    public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    
    FirstApp( );
    ~FirstApp( );
    
    FirstApp( const FirstApp &) = delete;
    FirstApp &operator = ( const FirstApp & ) = delete;
    
    void run( );
    
    
    private:
    void loadGameObjects( );
    
    LveWindow lveWindow{ WIDTH, HEIGHT, "VULKAN 3D ENGINE"};
    LveDevice lveDevice{lveWindow};
    LveRenderer lveRenderer{ lveWindow, lveDevice };
    
    // note: order of declarations matter
    std::unique_ptr<LveDescriptorPool> globalPool{ };
    LveGameObject::Map gameObjects;
    
};

} // namespace lve ---- end
