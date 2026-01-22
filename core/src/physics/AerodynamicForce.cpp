#include "physics/AerodynamicForce.hpp"
#include <cmath>
#include <omp.h>

namespace ClothSDK {

AerodynamicForce::AerodynamicForce(
    const std::vector<AeroFace>& faces,
    const Eigen::Vector3d& wind,
    double airDensity
)
    : m_faces(faces),
        m_wind(wind),
        m_airDensity(airDensity) {}

void AerodynamicForce::apply(std::vector<Particle>& particles, double dt) {
    if (dt < 1e-6)
        return;

    m_time += dt;

    double gust = std::sin(m_time * 5.0) * 0.5 + 0.5;
    Eigen::Vector3d currentWind = m_wind * (1.0 + gust);

    #pragma omp parallel for
    for (int i = 0; i < (int)m_faces.size(); i++) {
        const auto& face = m_faces[i];

        Particle& pA = particles[face.a];
        Particle& pB = particles[face.b];
        Particle& pC = particles[face.c];

        Eigen::Vector3d vFace =
            (pA.getVelocity(dt) +
                pB.getVelocity(dt) +
                pC.getVelocity(dt)) / 3.0;

        Eigen::Vector3d vRel = vFace - currentWind;
        double vMag = vRel.norm();

        if (vMag < 1e-4)
            continue;

        Eigen::Vector3d edge1 = pB.getPosition() - pA.getPosition();
        Eigen::Vector3d edge2 = pC.getPosition() - pA.getPosition();

        Eigen::Vector3d n = edge1.cross(edge2);
        double area = 0.5 * n.norm();

        if (area < 1e-6)
            continue;

        Eigen::Vector3d normal = n.normalized();

        double pressure = vRel.dot(normal) / vMag;

        Eigen::Vector3d force =
            -0.5 * m_airDensity * vMag * vMag * area * pressure * normal;

        Eigen::Vector3d f = force / 3.0;

        #pragma omp critical
        {
            pA.addForce(f);
            pB.addForce(f);
            pC.addForce(f);
        }
    }
}

} 
