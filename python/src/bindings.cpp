// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <tuple>

#include "engine/Cloth.hpp"
#include "engine/World.hpp"
#include "physics/Particle.hpp"
#include "physics/Constraint.hpp"
#include "physics/DistanceConstraint.hpp"
#include "physics/BendingConstraint.hpp"
#include "physics/Collider.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/SphereCollider.hpp"
#include "physics/CapsuleCollider.hpp"
#include "physics/Force.hpp"
#include "physics/AerodynamicForce.hpp"
#include "physics/GravityForce.hpp"
#include "physics/Solver.hpp"
#include "engine/ClothMesh.hpp"
#include "io/OBJLoader.hpp"
#include "io/OBJExporter.hpp"
#include "io/ConfigLoader.hpp"
#include "utils/Logger.hpp"
#include "math/Types.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include "io/AlembicExporter.hpp"

namespace py = pybind11;
using namespace ClothSDK;

PYBIND11_MODULE(_cloth_sdk_core, m) {
    m.doc() = "ClothSDK: Professional XPBD Simulation Engine";

    py::class_<Triangle>(m, "Triangle")
        .def(py::init<int, int, int>(), py::arg("a"), py::arg("b"), py::arg("c"))
        .def_readwrite("a", &Triangle::a)
        .def_readwrite("b", &Triangle::b)
        .def_readwrite("c", &Triangle::c);

    py::class_<ClothSDK::ClothMaterial, std::shared_ptr<ClothSDK::ClothMaterial>>(m, "ClothMaterial")
        .def(py::init<double, double, double, double>(),
            py::arg("density"), py::arg("structural"), py::arg("shear"), py::arg("bending"))
        .def_readwrite("density", &ClothMaterial::density)
        .def_readwrite("structural_compliance", &ClothMaterial::structuralCompliance)
        .def_readwrite("shear_compliance", &ClothMaterial::shearCompliance)
        .def_readwrite("bending_compliance", &ClothMaterial::bendingCompliance);

    py::class_<ClothSDK::AeroFace>(m, "AeroFace")
        .def(py::init<int, int, int>(), py::arg("a"), py::arg("b"), py::arg("c"))
        .def_readwrite("a", &AeroFace::a)
        .def_readwrite("b", &AeroFace::b)
        .def_readwrite("c", &AeroFace::c);

    py::class_<ClothSDK::Force, std::shared_ptr<ClothSDK::Force>>(m, "Force");

    py::class_<ClothSDK::GravityForce, ClothSDK::Force, std::shared_ptr<ClothSDK::GravityForce>>(m, "GravityForce")
        .def(py::init<const Eigen::Vector3d&>());

    py::class_<ClothSDK::AerodynamicForce, ClothSDK::Force, std::shared_ptr<ClothSDK::AerodynamicForce>>(m, "AerodynamicForce")
        .def(py::init<const std::vector<AeroFace>&, const Eigen::Vector3d&, double>());

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

    py::class_<CapsuleCollider, Collider, std::unique_ptr<CapsuleCollider>>(m, "CapsuleCollider")
        .def(py::init<double, const Eigen::Vector3d&, const Eigen::Vector3d&, double>(), py::arg("radius"), py::arg("start"), py::arg("end"), py::arg("friction"));

    py::class_<SpatialHash>(m, "SpatialHash")
    .def(py::init<int, double>(), py::arg("table_size"), py::arg("cell_size"))
    .def("build", &SpatialHash::build, py::arg("particles"))
    .def("query", &SpatialHash::query, 
        py::arg("particles"), py::arg("pos"), py::arg("radius"), py::arg("out_neighbors"));

    py::class_<World>(m, "World")
        .def(py::init<>())
        .def("add_cloth", &World::addCloth)
        .def("add_collider", &World::addCollider)
        .def("add_force", &World::addForce)
        .def("clear", &World::clear)
        .def("add_plane_collider", &World::addPlaneCollider)
        .def("add_sphere_collider", &World::addSphereCollider)
        .def("set_gravity", &World::setGravity)
        .def("set_wind", &World::setWind)
        .def("set_air_density", &World::setAirDensity)
        .def("set_thickness", &World::setThickness)
        .def("get_thickness", &World::getThickness)
        .def("get_gravity", &World::getGravity)
        .def("get_wind", &World::getWind)
        .def("get_air_density", &World::getAirDensity);

    py::class_<Solver, std::shared_ptr<ClothSDK::Solver>>(m, "Solver")
        .def(py::init<>())
        .def("update", &Solver::update, py::arg("world"), py::arg("delta_time"))
        .def("clear", &Solver::clear)
        .def("add_particle", &Solver::addParticle)
        .def("get_particles", &Solver::getParticles, py::return_value_policy::reference_internal)
        .def("set_substeps", &Solver::setSubsteps)
        .def("set_iterations", &Solver::setIterations)
        .def("get_iterations", &Solver::getIterations)
        .def("get_substeps", &Solver::getSubsteps)
        .def("add_distance_constraint", &Solver::addDistanceConstraint)
        .def("add_bending_constraint", &Solver::addBendingConstraint)
        .def("add_pin", &Solver::addPin)
        .def("set_collision_compliance", &Solver::setCollisionCompliance);

    py::class_<ClothMesh, std::shared_ptr<ClothSDK::ClothMesh>>(m, "ClothMesh")
        .def(py::init<>())
        .def("init_grid", &ClothMesh::initGrid, 
            py::arg("rows"), py::arg("cols"), py::arg("spacing"), py::arg("out_cloth"), py::arg("solver"))
        .def("build_from_mesh", &ClothMesh::buildFromMesh, 
            py::arg("positions"), py::arg("indices"), py::arg("out_cloth"), py::arg("solver"));

    py::class_<ClothSDK::Cloth, std::shared_ptr<ClothSDK::Cloth>>(m, "Cloth")
        .def(py::init<const std::string&, std::shared_ptr<ClothMaterial>>(), 
            py::arg("name"), py::arg("material"))
        .def("get_name", &Cloth::getName)
        .def("get_particle_id", &Cloth::getParticleID, py::arg("row"), py::arg("col"))
        .def("get_material", &Cloth::getMaterial)
        .def("set_material", &Cloth::setMaterial)
        .def("get_particle_indices", &Cloth::getParticleIndices)
        .def("get_aerofaces", &Cloth::getAeroFaces)
        .def("get_triangles", [](const Cloth& cloth) {
            std::vector<int> flat;
            for (const auto& t : cloth.getTriangles()) {
                flat.push_back(t.a); flat.push_back(t.b); flat.push_back(t.c);
            }
            return flat;
        });

    py::class_<OBJLoader>(m, "OBJLoader")
        .def_static("load", [](const std::string& path) {
        std::vector<Eigen::Vector3d> pos;
        std::vector<int> indices;
        bool success = ClothSDK::OBJLoader::load(path, pos, indices);
        
        return std::make_tuple(success, pos, indices);
    });

    py::class_<ConfigLoader>(m, "ConfigLoader")
        .def_static("load", &ConfigLoader::load)
        .def_static("save", &ConfigLoader::save);

    py::class_<Logger>(m, "Logger")
    .def_static("info", &Logger::info, py::arg("message"))
    .def_static("warn", &Logger::warn, py::arg("message"))
    .def_static("error", &Logger::error, py::arg("message"));

    py::class_<ClothSDK::Viewer::Renderer, std::unique_ptr<ClothSDK::Viewer::Renderer>>(m, "Renderer")
    .def("set_shader_path", &ClothSDK::Viewer::Renderer::setShaderPath, 
        py::arg("path"), "Sets the directory where .vert and .frag files are located.");

    py::class_<Viewer::Application>(m, "Application")
    .def(py::init<>())
    .def("init", &ClothSDK::Viewer::Application::init, 
        py::arg("width"), py::arg("height"), py::arg("title"), py::arg("shader_path"))
    .def("run", &ClothSDK::Viewer::Application::run)
    .def("shutdown", &ClothSDK::Viewer::Application::shutdown)
    .def("sync_visual_topology", &ClothSDK::Viewer::Application::syncVisualTopology)
    .def("set_solver", &ClothSDK::Viewer::Application::setSolver, py::arg("solver"))
    .def("set_cloth", &ClothSDK::Viewer::Application::setCloth)
    .def("set_mesh", &ClothSDK::Viewer::Application::setMesh, py::arg("mesh"))
    .def("get_renderer", &ClothSDK::Viewer::Application::getRenderer, 
        py::return_value_policy::reference_internal);    

    py::class_<ClothSDK::AlembicExporter>(m, "AlembicExporter")
    .def(py::init<>())
    .def("open", &ClothSDK::AlembicExporter::open, py::arg("path"), py::arg("positions"), py::arg("indices"))
    .def("write_frame", &ClothSDK::AlembicExporter::writeFrame, py::arg("positions"), py::arg("time"))
    .def("close", &ClothSDK::AlembicExporter::close);

    py::class_<OBJExporter>(m, "OBJExporter")
    .def(py::init<>())
    .def_static("export_obj", &OBJExporter::exportOBJ, 
        py::arg("filename"), 
        py::arg("cloth"), 
        py::arg("solver"), 
        "Exports a specific cloth instance to an OBJ file"); 
}