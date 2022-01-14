#include "first_app.hpp"
#include "simple_render_system.hpp"
#include "lve_camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <chrono>
#include <numeric>

namespace lve{

struct GlobalUbo{
    glm::mat4 projection{ 1.f };
    glm::mat4 view{ 1.f };
    glm::vec4 ambientLightColor { 1.f, 1.f, 1.f, .02f }; // w is in intensity
    glm::vec3 lightPosition{ -1.f };
    alignas(16) glm::vec4 lightColor { 1.f }; // w is light intensity
};

FirstApp::FirstApp( ){
    globalPool = LveDescriptorPool::Builder( lveDevice )
    .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)

    .build();
    
    loadGameObjects();
}

FirstApp::~FirstApp( ){}
    

void FirstApp::run(){
    
    std::vector<std::unique_ptr<LveBuffer>> uboBuffers( LveSwapChain::MAX_FRAMES_IN_FLIGHT );
    for ( int i = 0; i < uboBuffers.size(); i++ ){
        uboBuffers[i] = std::make_unique<LveBuffer>(
        lveDevice,
         sizeof(GlobalUbo),
         1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
        
        uboBuffers[i]->map();
    };
    
    auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
    .build();
    
    std::vector<VkDescriptorSet> globalDescriptorSets( LveSwapChain::MAX_FRAMES_IN_FLIGHT );
    for ( int i = 0; i < globalDescriptorSets.size(); i++ ){
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        LveDescriptorWriter( *globalSetLayout, *globalPool )
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
    }
    
    SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
    LveCamera camera{};
    camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
    
    auto viewerObject = LveGameObject::createGameObject();
    viewerObject.transform3d.translation.z = -2.5f;
    KeyboardMovementController cameraController { };
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    while ( !lveWindow.shouldClose( ) ) {
        
        glfwPollEvents();
        
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>( newTime - currentTime ).count();
        currentTime = newTime;
        
        //frameTime = glm::min(frameTime, MAX_FRAME_TIME);
        
        cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform3d.translation, viewerObject.transform3d.rotation);
        
        float aspect = lveRenderer.getAspectRation();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
        
        if ( auto commandBuffer = lveRenderer.beginFrame() ) {
            
            int frameIndex = lveRenderer.getFrameIndex();
            FrameInfo frameInfo{
               frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                gameObjects
            };
            
            // update
            GlobalUbo ubo{ };
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();

            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            
            // render
            lveRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(frameInfo);
            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
        }
        
    }
    
    vkDeviceWaitIdle(lveDevice.device());
}


void FirstApp::loadGameObjects( ){
    
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "/Users/zai/Desktop/Vulkan 3D Engine/Vulkan Test/models/flat_vase.obj");
    auto flatVase = LveGameObject::createGameObject();
    flatVase.model = lveModel;
    flatVase.transform3d.translation = { -.5f, .5f, 0.f };
    flatVase.transform3d.scale = glm::vec3{3.f, 1.5f, 3.f}; //{ .5f, .5f, .5f };
    gameObjects.emplace(flatVase.getId(), std::move(flatVase));
    
    
    lveModel = LveModel::createModelFromFile(lveDevice, "/Users/zai/Desktop/Vulkan 3D Engine/Vulkan Test/models/smooth_vase.obj");
    auto gameObj = LveGameObject::createGameObject();
    gameObj.model = lveModel;
    gameObj.transform3d.translation = { .5f, .5f, 0.f};
    gameObj.transform3d.scale = glm::vec3{3.f, 1.5f, 3.f}; //{ .5f, .5f, .5f };
    gameObjects.emplace(gameObj.getId(), std::move(gameObj));
    
    lveModel = LveModel::createModelFromFile(lveDevice, "/Users/zai/Desktop/Vulkan 3D Engine/Vulkan Test/models/quad.obj");
    auto floor = LveGameObject::createGameObject();
    floor.model = lveModel;
    floor.transform3d.translation = { 0.f, .5f, 0.f};
    floor.transform3d.scale = glm::vec3{3.f, 1.f, 3.f}; //{ .5f, .5f, .5f };
    gameObjects.emplace(floor.getId(), std::move(floor));
    
}


} // namespace lve ---- end
