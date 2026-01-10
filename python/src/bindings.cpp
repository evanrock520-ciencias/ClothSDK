#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>

#include "physics/Particle.hpp"
#include "physics/Constraint.hpp"
#include "physics/DistanceConstraint.hpp"
#include "physics/BendingConstraint.hpp"
#include "physics/Collider.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/SphereCollider.hpp"
#include "physics/Solver.hpp"
#include "engine/ClothMesh.hpp"
#include "io/OBJLoader.hpp"
#include "io/ConfigLoader.hpp"
#include "utils/Logger.hpp"
#include "math/Types.hpp"

namespace py = pybind11;
using namespace ClothSDK;

PYBIND11_MODULE(cloth_sdk, m) {
    m.doc() = "ClothSDK: Professional XPBD Simulation Engine";

    py::class_<Triangle>(m, "Triangle")
        .def(py::init<int, int, int>(), py::arg("a"), py::arg("b"), py::arg("c"))
        .def_readwrite("a", &Triangle::a)
        .def_readwrite("b", &Triangle::b)
        .def_readwrite("c", &Triangle::c);

    py::class_<Particle>(m, "Particle")
        .def(py::init<const Eigen::Vector3d&>(), py::arg("initial_pos"))
        .def("get_position", &Particle::getPosition)
        .def("set_position", &Particle::setPosition)
        .def("get_inverse_mass", &Particle::getInverseMass)
        .def("set_inverse_mass", &Particle::setInverseMass)
        .def("add_force", &Particle::addForce)
        .def("integrate", &Particle::integrate);

    py::class_<Constraint, std::unique_ptr<Constraint>>(m, "Constraint")
        .def("reset_lambda", &Constraint::resetLambda);

    py::class_<DistanceConstraint, Constraint, std::unique_ptr<DistanceConstraint>>(m, "DistanceConstraint")
        .def(py::init<int, int, double, double>(), py::arg("idA"), py::arg("idB"), py::arg("restLength"), py::arg("compliance"));

    py::class_<BendingConstraint, Constraint, std::unique_ptr<BendingConstraint>>(m, "BendingConstraint")
        .def(py::init<int, int, int, int, double, double>(), py::arg("idA"), py::arg("idB"), py::arg("idC"), py::arg("idD"), py::arg("restAngle"), py::arg("compliance"));

    py::class_<Collider, std::unique_ptr<Collider>>(m, "Collider")
        .def("get_friction", &Collider::getFriction)
        .def("set_friction", &Collider::setFriction);

    py::class_<PlaneCollider, Collider, std::unique_ptr<PlaneCollider>>(m, "PlaneCollider")
        .def(py::init<const Eigen::Vector3d&, const Eigen::Vector3d&, double>(), py::arg("origin"), py::arg("normal"), py::arg("friction"));

    py::class_<SphereCollider, Collider, std::unique_ptr<SphereCollider>>(m, "SphereCollider")
        .def(py::init<const Eigen::Vector3d&, double, double>(), py::arg("center"), py::arg("radius"), py::arg("friction"));

    py::class_<SpatialHash>(m, "SpatialHash")
    .def(py::init<int, double>(), py::arg("table_size"), py::arg("cell_size"))
    .def("build", &SpatialHash::build, py::arg("particles"))
    .def("query", &SpatialHash::query, 
        py::arg("particles"), py::arg("pos"), py::arg("radius"), py::arg("out_neighbors"));

    py::class_<Solver>(m, "Solver")
        .def(py::init<>())
        .def("update", &Solver::update, py::arg("delta_time"))
        .def("clear", &Solver::clear)
        .def("add_particle", &Solver::addParticle)
        .def("get_particles", &Solver::getParticles, py::return_value_policy::reference_internal)
        .def("set_gravity", &Solver::setGravity)
        .def("get_gravity", &Solver::getGravity)
        .def("set_substeps", &Solver::setSubsteps)
        .def("set_iterations", &Solver::setIterations)
        .def("add_distance_constraint", &Solver::addDistanceConstraint)
        .def("add_bending_constraint", &Solver::addBendingConstraint)
        .def("add_plane_collider", &Solver::addPlaneCollider)
        .def("add_sphere_collider", &Solver::addSphereCollider)
        .def("set_wind", &Solver::setWind)
        .def("set_air_density", &Solver::setAirDensity)
        .def("set_thickness", &Solver::setThickness);

    py::class_<ClothMesh>(m, "ClothMesh")
        .def(py::init<>())
        .def("init_grid", &ClothMesh::initGrid)
        .def("build_from_mesh", &ClothMesh::buildFromMesh)
        .def("set_material", &ClothMesh::setMaterial)
        .def("export_to_obj", &ClothMesh::exportToOBJ);

    py::class_<OBJLoader>(m, "OBJLoader")
        .def_static("load", &OBJLoader::load);

    py::class_<ConfigLoader>(m, "ConfigLoader")
        .def_static("load", &ConfigLoader::load)
        .def_static("save", &ConfigLoader::save);

    py::class_<Logger>(m, "Logger")
    .def_static("info", &Logger::info, py::arg("message"))
    .def_static("warn", &Logger::warn, py::arg("message"))
    .def_static("error", &Logger::error, py::arg("message"));
}