#pragma once

#include <vector>
#include <Eigen/Dense>

#include "physics/Force.hpp"
#include "physics/Particle.hpp"

namespace ClothSDK {

struct AeroFace {
    int a, b, c;
};

class AerodynamicForce final : public Force {
public:
    AerodynamicForce(
        const std::vector<AeroFace>& faces,
        const Eigen::Vector3d& wind,
        double airDensity
    );

    void apply(std::vector<Particle>& particles, double dt) override;

    inline void setWind(const Eigen::Vector3d& wind);
    inline const Eigen::Vector3d& getWind() const;

    inline void setAirDensity(double density);
    inline double getAirDensity() const;
    inline void setFaces(AeroFace face) { m_faces.push_back(face); }

private:
    std::vector<AeroFace> m_faces;
    Eigen::Vector3d m_wind;
    double m_airDensity;
    double m_time = 0.0;
};

}
