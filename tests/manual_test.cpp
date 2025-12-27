#include "physics/Particle.hpp"
#include "physics/Solver.hpp"
#include <iostream>

int main() {
    ClothSDK::Solver solver;

    ClothSDK::Particle particle1(Eigen::Vector3d(0, 10, 0)); 
    
    ClothSDK::Particle particle2(Eigen::Vector3d(5, 10, 0));

    solver.addParticle(particle1);
    solver.addParticle(particle2);

    std::cout << "Fall simulation" << std::endl;

    for(int i = 0; i < 10; ++i) {
        solver.update(0.1);
        
        const auto& particles = solver.getParticles();
        std::cout << "Paso " << i << " | P1 Y: " << particles[0].getPosition().y() 
                  << " | P2 Y: " << particles[1].getPosition().y() << std::endl;
    }

    return 0;
}