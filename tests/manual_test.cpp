#include "physics/Particle.hpp"
#include <iostream>

int main() {
    Eigen::Vector3d pos(10.0, 20.0, 30.0);
    Eigen::Vector3d force(0, -9.8, 0);
    ClothSDK::Particle p(pos);

    std::cout << "Get Position: " << p.getPosition().transpose() << std::endl;
    std::cout << "Get Old Position: " << p.getOldPosition().transpose() << std::endl;
    std::cout << "Get Inverse Mass: " << p.getInverseMass() << std::endl;
    std::cout << "Get Acceleration: " << p.getAcceleration().transpose() << std::endl;

    for (int i = 0; i < 10; i++)
        p.addForce(force);

    std::cout << "Get New Acceleration: " << p.getAcceleration().transpose() << std::endl;

    p.integrate(1);

    std::cout << "Get Position After 1 Second: " << p.getPosition().transpose() << std::endl;
    std::cout << "Get Old Position After 1 Second: " << p.getOldPosition().transpose() << std::endl;

    return 0;
}