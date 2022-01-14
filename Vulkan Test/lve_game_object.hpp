#pragma once

#include "lve_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace lve{

struct Transform3dComponent{
    glm::vec3 translation { }; // position offset --- up, down, left, right
    glm::vec3 scale { 1.f, 1.f, 1.f };
    glm::vec3 rotation{ };
    
    // Optimized Rotation Code -------
    
    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
     // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
     // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 mat4();
    
    glm::mat3 normalMatrix( );
};

class LveGameObject{
    
    public: // ----- public methods -------
    
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, LveGameObject>;
    
    static LveGameObject createGameObject( ){
        
        static id_t currentId = 0;
        return LveGameObject{ currentId++ };
    }
    
    LveGameObject( const LveGameObject & ) = delete;
    LveGameObject &operator=( const LveGameObject & ) = delete;
    LveGameObject( LveGameObject && ) = default;
    LveGameObject &operator=( LveGameObject && ) = default;
    
    
    id_t getId( ) { return id; };
    
    std::shared_ptr<LveModel> model{ };
    glm::vec3 color{ };
    Transform3dComponent transform3d { };
    
    
    private: // ------- private methods --------
    
    LveGameObject( id_t objId ) : id{ objId } {}
    id_t id;
};

} // namespace lve ---- end
