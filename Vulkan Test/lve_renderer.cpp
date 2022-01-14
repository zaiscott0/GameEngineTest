#include "lve_renderer.hpp"

// std
#include <stdexcept>
#include <array>

namespace lve{

LveRenderer::LveRenderer( LveWindow &window, LveDevice &device ) : lveWindow{window}, lveDevice{device} {
    
    recreateSwapChain();
    createCommandBuffers();
}

LveRenderer::~LveRenderer( ){ freeCommandBuffers(); }


void LveRenderer::recreateSwapChain(){
    
    auto extent = lveWindow.getExtent();
    while ( extent.width == 0 | extent.height == 0 ){
        
        extent = lveWindow.getExtent();
        glfwWaitEvents();
        
    }
    
    vkDeviceWaitIdle(lveDevice.device());
    
    if ( lveSwapChain == nullptr ){
        
        lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
    }
    else{
        std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
        lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);
        
        if ( !oldSwapChain->compareSwapFormats( *lveSwapChain.get() ) ){
            throw std::runtime_error( "Swap Chain image (or depth) format has changed!" );
        }
    }
    
    // come back to here
 
}


void LveRenderer::createCommandBuffers( ){
    
    commandBuffers.resize( LveSwapChain::MAX_FRAMES_IN_FLIGHT );
    
    VkCommandBufferAllocateInfo allocInfo{ };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = lveDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    
    if (vkAllocateCommandBuffers( lveDevice.device(), &allocInfo, commandBuffers.data() ) != VK_SUCCESS ){
        
        throw std::runtime_error( "Failed to Allocate Command Buffers!" );
    }
    
}

void LveRenderer::freeCommandBuffers(){
    
    vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<float>( commandBuffers.size() ), commandBuffers.data());
    commandBuffers.clear();
    
}


VkCommandBuffer LveRenderer::beginFrame( ){
    assert(!isFrameStarted && "Can't call beginFrame() while already in progress!" );
    
    auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
    
    if( result == VK_ERROR_OUT_OF_DATE_KHR){
        
        recreateSwapChain();
        return nullptr;
        
    }
    
    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR ){
        
        throw std::runtime_error( "Failed to aquire next swap chain image!" );
        
    }
    
    isFrameStarted = true;
    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{ };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if ( vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS ){
        throw std::runtime_error( "Failed to Beign Recording Command Buffer!" );
    }
    
    return commandBuffer;
    
}


void LveRenderer::endFrame( ){
    assert( isFrameStarted && "Can't call endFrame() while frame is not in progress!" );
    auto commandBuffer = getCurrentCommandBuffer();
    if ( vkEndCommandBuffer(commandBuffer) != VK_SUCCESS ){
        throw std::runtime_error( "Failed to Record Command Buffer" );
    }
    
    auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()){
        
        lveWindow.resetWindowResizedFlag();
        recreateSwapChain();
    }
    
    else if ( result != VK_SUCCESS ){
        throw std::runtime_error( "Failed to present swap chain image!" );
    }
    
    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
    
}


void LveRenderer::beginSwapChainRenderPass( VkCommandBuffer commandBuffer ){
    assert( isFrameStarted && "Can't call beginSwapChainRenderPass() while frame is not in progress!" );
    assert( commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame!" );
    
    VkRenderPassBeginInfo renderPassInfo{ };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = lveSwapChain->getRenderPass();
    renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);
    
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();
    
    std::array<VkClearValue, 2> clearValues{ };
    clearValues[0].color = { 0.01f, 0.01f, 0.01f, 0.1f }; // can change these numbers at will
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    
    vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport{ };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
    viewport.height =static_cast<float>(lveSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{ 0,0 }, lveSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

}


void LveRenderer::endSwapChainRenderPass( VkCommandBuffer commandBuffer ){
    assert( isFrameStarted && "Can't call endSwapChainRenderPass() while frame is not in progress!" );
    assert( commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame!" );
    
    vkCmdEndRenderPass(commandBuffer);

}

} // namespace lve ---- end

