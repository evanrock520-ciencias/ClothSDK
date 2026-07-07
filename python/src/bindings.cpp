// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#ifdef _MSC_VER
#include <cstddef>
using ssize_t = std::ptrdiff_t;
#endif

#include <memory>
#include <tuple>

#include "Application.hpp"
#include "Eigen/src/Geometry/Quaternion.h"
#include "Renderer.hpp"
#include "data-structures/SpatialHash.hpp"
#include "engine/Cloth.hpp"
#include "engine/ClothMesh.hpp"
#include "engine/World.hpp"
#include "io/AlembicExporter.hpp"
#include "io/ConfigLoader.hpp"
#include "io/OBJExporter.hpp"
#include "io/OBJLoader.hpp"
#include "io/SceneExporter.hpp"
#include "io/SceneLoader.hpp"
#include "io/StateSerializer.hpp"
#include "math/Types.hpp"
#include "physics/AerodynamicForce.hpp"
#include "physics/BendingConstraint.hpp"
#include "physics/CapsuleCollider.hpp"
#include "physics/Collider.hpp"
#include "physics/AttachmentConstraint.hpp"
#include "physics/Constraint.hpp"
#include "physics/DistanceConstraint.hpp"
#include "physics/Force.hpp"
#include "physics/GravityForce.hpp"
#include "physics/MeshCollider.hpp"
#include "physics/Particle.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/Solver.hpp"
#include "physics/SphereCollider.hpp"
#include "physics/VolumeConstraint.hpp"
#include "utils/Logger.hpp"

namespace py = pybind11;
using namespace Tissu;

PYBIND11_MODULE(_cloth_sdk_core, m) {
    m.doc() = "Tissu: Professional XPBD Simulation Engine";

    py::class_<Triangle>(m, "Triangle")
        .def(py::init<int, int, int>(), py::arg("a"), py::arg("b"),
             py::arg("c"))
        .def_readwrite("a", &Triangle::a)
        .def_readwrite("b", &Triangle::b)
        .def_readwrite("c", &Triangle::c);

    py::enum_<PinMode>(m, "PinMode")
        .value("TOP_CORNERS", PinMode::TOP_CORNERS)
        .value("BY_HEIGHT", PinMode::BY_HEIGHT)
        .value("NONE", PinMode::NONE);

    py::class_<Pin>(m, "Pin")
        .def(py::init<const PinMode&, double, double>(), py::arg("pin_mode"),
             py::arg("compliance"), py::arg("threshold"))
        .def("get_pin_mode", &Pin::getPinMode)
        .def("get_compliance", &Pin::getCompliance)
        .def("get_threshold", &Pin::getThreshold)
        .def("set_mode", &Pin::setPinMode)
        .def("set_compliance", &Pin::setCompliance)
        .def("set_threshold", &Pin::setThreshold);

    py::class_<Tissu::ClothMaterial, std::shared_ptr<Tissu::ClothMaterial>>(
        m, "ClothMaterial")
        .def(py::init<double, double, double, double>(), py::arg("density"),
             py::arg("structural"), py::arg("shear"), py::arg("bending"))
        .def_property("density", &ClothMaterial::getDensity,
                      &ClothMaterial::setDensity)
        .def_property("structural_compliance",
                      &ClothMaterial::getStructuralCompliance,
                      &ClothMaterial::setStructuralCompliance)
        .def_property("shear_compliance", &ClothMaterial::getShearCompliance,
                      &ClothMaterial::setShearCompliance)
        .def_property("bending_compliance",
                      &ClothMaterial::getBendingCompliance,
                      &ClothMaterial::setBendingCompliance);

    py::class_<Tissu::AeroFace>(m, "AeroFace")
        .def(py::init<int, int, int>(), py::arg("a"), py::arg("b"),
             py::arg("c"))
        .def_readwrite("a", &AeroFace::a)
        .def_readwrite("b", &AeroFace::b)
        .def_readwrite("c", &AeroFace::c);

    py::class_<SceneHeader::FabricInfo>(m, "FabricInfo")
        .def_readonly("name", &SceneHeader::FabricInfo::name)
        .def_readonly("type", &SceneHeader::FabricInfo::type)
        .def_readonly("rows", &SceneHeader::FabricInfo::rows)
        .def_readonly("cols", &SceneHeader::FabricInfo::cols)
        .def_readonly("spacing", &SceneHeader::FabricInfo::spacing)
        .def_readonly("source", &SceneHeader::FabricInfo::source)
        .def_readonly("material", &SceneHeader::FabricInfo::material)
        .def_readonly("pin_mode", &SceneHeader::FabricInfo::pin_mode);

    py::class_<SceneHeader::ColliderInfo>(m, "ColliderInfo")
        .def_readonly("name", &SceneHeader::ColliderInfo::name)
        .def_readonly("type", &SceneHeader::ColliderInfo::type)
        .def_readonly("summary", &SceneHeader::ColliderInfo::summary);

    py::class_<SceneHeader>(m, "SceneHeader")
        .def_readonly("version", &SceneHeader::version)
        .def_readonly("name", &SceneHeader::name)
        .def_readonly("physics_preset", &SceneHeader::physics_preset)
        .def_readonly("fabrics", &SceneHeader::fabrics)
        .def_readonly("colliders", &SceneHeader::colliders);

    py::class_<Tissu::StateInfo>(m, "StateInfo")
        .def_property_readonly(
            "version",
            [](const StateInfo& si) { return static_cast<int>(si.version); })
        .def_readonly("frame", &StateInfo::frame)
        .def_readonly("timestamp", &StateInfo::timestamp)
        .def_readonly("particle_count", &StateInfo::particleCount);

    py::class_<Tissu::Force, std::shared_ptr<Tissu::Force>>(m, "Force");

    py::class_<Tissu::GravityForce, Tissu::Force,
               std::shared_ptr<Tissu::GravityForce>>(m, "GravityForce")
        .def(py::init<const Eigen::Vector3d&>())
        .def("set_gravity", &GravityForce::setGravity)
        .def("get_gravity", &GravityForce::getGravity);

    py::class_<Tissu::AerodynamicForce, Tissu::Force,
               std::shared_ptr<Tissu::AerodynamicForce>>(m, "AerodynamicForce")
        .def(py::init<const std::vector<AeroFace>&, const Eigen::Vector3d&,
                      double>());

    py::class_<Particle>(m, "Particle")
        .def(py::init<const Eigen::Vector3d&>(), py::arg("initial_pos"))
        .def("get_position", &Particle::getPosition)
        .def("set_position", &Particle::setPosition)
        .def("get_old_position", &Particle::getOldPosition)
        .def("set_old_position", &Particle::setOldPosition)
        .def("get_inverse_mass", &Particle::getInverseMass)
        .def("set_inverse_mass", &Particle::setInverseMass)
        .def("get_velocity", &Particle::getVelocity)
        .def("add_force", &Particle::addForce)
        .def("integrate", &Particle::integrate);

    py::class_<Constraint, std::unique_ptr<Constraint>>(m, "Constraint")
        .def("reset_lambda", &Constraint::resetLambda);

    py::class_<DistanceConstraint, Constraint,
               std::unique_ptr<DistanceConstraint>>(m, "DistanceConstraint")
        .def(py::init<int, int, double, double>(), py::arg("idA"),
             py::arg("idB"), py::arg("restLength"), py::arg("compliance"));

    py::class_<BendingConstraint, Constraint,
               std::unique_ptr<BendingConstraint>>(m, "BendingConstraint")
        .def(py::init<int, int, int, int, double, double>(), py::arg("idA"),
             py::arg("idB"), py::arg("idC"), py::arg("idD"),
             py::arg("restAngle"), py::arg("compliance"));

    py::class_<VolumeConstraint, Constraint, std::unique_ptr<VolumeConstraint>>(
        m, "VolumeConstraint")
        .def(py::init<const std::vector<Triangle>&,
                      const std::vector<Particle>&, double>(),
             py::arg("triangles"), py::arg("particles"), py::arg("compliance"))
        .def("get_rest_volume", &VolumeConstraint::getRestVolume);

    py::class_<AttachmentConstraint, Constraint,
               std::unique_ptr<AttachmentConstraint>>(m, "AttachmentConstraint")
        .def(py::init<int, std::shared_ptr<Collider>, int, double, double>(),
             py::arg("particle_id"), py::arg("collider"),
             py::arg("target_vertex_id"), py::arg("compliance") = 0.0,
             py::arg("rest_length") = 0.0)
        .def(py::init<int, std::shared_ptr<Collider>, const Eigen::Vector3d&,
                      double, double>(),
             py::arg("particle_id"), py::arg("collider"),
             py::arg("local_anchor"), py::arg("compliance") = 0.0,
             py::arg("rest_length") = 0.0)
        .def("get_particle_id", &AttachmentConstraint::getParticleId);

    py::class_<Collider, std::shared_ptr<Collider>>(m, "Collider")
        .def("get_friction", &Collider::getFriction)
        .def("set_friction", &Collider::setFriction)
        .def("get_name", &Collider::getName)
        .def("set_name", &Collider::setName)
        .def("get_position", &Collider::getPosition)
        .def("get_rotation", &Collider::getRotation);

    py::class_<PlaneCollider, Collider, std::shared_ptr<PlaneCollider>>(
        m, "PlaneCollider")
        .def(py::init<const Eigen::Vector3d&, const Eigen::Vector3d&, double>(),
             py::arg("origin"), py::arg("normal"), py::arg("friction"));

    py::class_<SphereCollider, Collider, std::shared_ptr<SphereCollider>>(
        m, "SphereCollider")
        .def(py::init<const Eigen::Vector3d&, double, double>(),
             py::arg("center"), py::arg("radius"), py::arg("friction"));

    py::class_<CapsuleCollider, Collider, std::shared_ptr<CapsuleCollider>>(
        m, "CapsuleCollider")
        .def(py::init<double, const Eigen::Vector3d&, const Eigen::Vector3d&,
                      double>(),
             py::arg("radius"), py::arg("start"), py::arg("end"),
             py::arg("friction"));

    py::class_<MeshCollider, Collider, std::shared_ptr<MeshCollider>>(
        m, "MeshCollider")
        .def(py::init<const std::string&, double>(), py::arg("mesh_path"),
             py::arg("friction"))
        .def(py::init<const std::vector<Eigen::Vector3d>&,
                      const std::vector<std::array<int, 3>>&, double>(),
             py::arg("vertices"), py::arg("triangles"), py::arg("friction"))
        .def("get_mesh_path", &MeshCollider::getMeshPath)
        .def("get_world_vertices", &MeshCollider::getWorldVertices);

    py::class_<SpatialHash>(m, "SpatialHash")
        .def(py::init<int, double>(), py::arg("table_size"),
             py::arg("cell_size"))
        .def("build", &SpatialHash::build, py::arg("particles"))
        .def("query", &SpatialHash::query, py::arg("particles"), py::arg("pos"),
             py::arg("radius"), py::arg("out_neighbors"));

    py::class_<World, std::shared_ptr<World>>(m, "World")
        .def(py::init<>())
        .def("add_cloth", &World::addCloth)
        .def("add_collider", &World::addCollider)
        .def("add_force", &World::addForce)
        .def("clear", &World::clear)
        .def("add_plane_collider", &World::addPlaneCollider)
        .def("add_sphere_collider", &World::addSphereCollider)
        .def("add_capsule_collider", &World::addCapsuleCollider)
        .def("add_mesh_collider", &World::addMeshCollider)
        .def("set_gravity", &World::setGravity)
        .def("set_wind", &World::setWind)
        .def("set_air_density", &World::setAirDensity)
        .def("set_thickness", &World::setThickness)
        .def("get_cloths", &World::getCloths)
        .def("get_colliders", &World::getColliders,
             py::return_value_policy::reference_internal)
        .def("get_thickness", &World::getThickness)
        .def("get_gravity", &World::getGravity)
        .def("get_wind", &World::getWind)
        .def("get_air_density", &World::getAirDensity)
        .def(
            "move_collider",
            [](World& self, size_t index, Eigen::Vector3d newPosition,
               Eigen::Vector4d newRotation) // <-- Vector4d, no Quaterniond
            {
                Eigen::Quaterniond quat(newRotation[3], newRotation[0],
                                        newRotation[1], newRotation[2]);
                self.moveCollider(index, newPosition, quat);
            },
            py::arg("index"), py::arg("new_position"), py::arg("new_rotation"));

    py::class_<Solver, std::shared_ptr<Tissu::Solver>>(m, "Solver")
        .def(py::init<>())
        .def("update", &Solver::update, py::arg("world"), py::arg("delta_time"))
        .def("clear", &Solver::clear)
        .def("add_particle", &Solver::addParticle)
        .def("get_particles",
             static_cast<const std::vector<Particle>& (Solver::*)() const>(
                 &Solver::getParticles),
             py::return_value_policy::reference_internal)
        .def("set_substeps", &Solver::setSubsteps)
        .def("set_iterations", &Solver::setIterations)
        .def("get_iterations", &Solver::getIterations)
        .def("get_substeps", &Solver::getSubsteps)
        .def("get_time", &Solver::getCurrentTime)
        .def("get_frame", &Solver::getCurrentFrame)
        .def("add_distance_constraint", &Solver::addDistanceConstraint)
        .def("add_bending_constraint", &Solver::addBendingConstraint)
        .def("add_volume_constraint", &Solver::addVolumeConstraint)
        .def("add_pin", &Solver::addPin)
        .def("unpin", &Solver::removePin)
        .def("add_stitch", &Solver::addStitch)
        .def("add_attach", &Solver::addAttach)
        .def("remove_attach", &Solver::removeAttach)
        .def("add_attachment", &Solver::addAttachment, py::arg("particle_id"),
             py::arg("collider"), py::arg("target_vertex_id"),
             py::arg("compliance") = 0.0, py::arg("rest_length") = 0.0)
        .def("add_attachment_local", &Solver::addAttachmentLocal,
             py::arg("particle_id"), py::arg("collider"),
             py::arg("local_anchor"), py::arg("compliance") = 0.0,
             py::arg("rest_length") = 0.0)
        .def("remove_attachment", &Solver::removeAttachment,
             py::arg("particle_id"))
        .def("soft_reset", &Solver::softReset)
        .def("set_collision_compliance", &Solver::setCollisionCompliance);

    py::class_<ClothMesh, std::shared_ptr<Tissu::ClothMesh>>(m, "ClothMesh")
        .def(py::init<>())
        .def(
            "init_grid",
            [](ClothMesh& self, int rows, int cols, double spacing,
               Cloth& out_cloth, Solver& solver,
               const Eigen::Vector3d& translation, Eigen::Vector4d rotation) {
                const Eigen::Quaterniond quat(rotation[3], rotation[0],
                                              rotation[1], rotation[2]);
                self.initGrid(rows, cols, spacing, out_cloth, solver,
                              translation, quat);
            },
            py::arg("rows"), py::arg("cols"), py::arg("spacing"),
            py::arg("out_cloth"), py::arg("solver"),
            py::arg("translation") = Eigen::Vector3d::Zero(),
            py::arg("rotation") = Eigen::Vector4d(0.0, 0.0, 0.0, 1.0))
        .def(
            "build_from_mesh",
            [](ClothMesh& self, const std::vector<Eigen::Vector3d>& positions,
               const std::vector<int>& indices, Cloth& out_cloth,
               Solver& solver, const std::string& mesh_path,
               const Eigen::Vector3d& translation, Eigen::Vector4d rotation) {
                const Eigen::Quaterniond quat(rotation[3], rotation[0],
                                              rotation[1], rotation[2]);
                self.buildFromMesh(positions, indices, out_cloth, solver,
                                   mesh_path, translation, quat);
            },
            py::arg("positions"), py::arg("indices"), py::arg("out_cloth"),
            py::arg("solver"), py::arg("mesh_path"),
            py::arg("translation") = Eigen::Vector3d::Zero(),
            py::arg("rotation") = Eigen::Vector4d(0.0, 0.0, 0.0, 1.0));

    py::class_<Tissu::Cloth, std::shared_ptr<Tissu::Cloth>>(m, "Cloth")
        .def(py::init<const std::string&, std::shared_ptr<ClothMaterial>>(),
             py::arg("name"), py::arg("material"))
        .def("get_name", &Cloth::getName)
        .def("get_particle_id", &Cloth::getParticleID, py::arg("row"),
             py::arg("col"))
        .def("get_material", &Cloth::getMaterial)
        .def("is_closed", &Cloth::isClosed)
        .def("get_rest_volume", &Cloth::getRestVolume)
        .def("get_pin", &Cloth::getPin)
        .def("set_pin", &Cloth::setPin)
        .def("set_rest_volume", &Cloth::setRestVolume)
        .def("set_material", &Cloth::setMaterial)
        .def("get_particle_indices", &Cloth::getParticleIndices)
        .def("get_aerofaces", &Cloth::getAeroFaces)
        .def("get_triangles",
             [](const Cloth& cloth) {
                 std::vector<int> flat;
                 for (const auto& t : cloth.getTriangles()) {
                     flat.push_back(t.a);
                     flat.push_back(t.b);
                     flat.push_back(t.c);
                 }
                 return flat;
             })
        .def("get_triangles_native",
             [](const Cloth& cloth) { return cloth.getTriangles(); });

    py::class_<OBJLoader>(m, "OBJLoader")
        .def_static("load", [](const std::string& path) {
            std::vector<Eigen::Vector3d> pos;
            std::vector<int> indices;
            bool success = Tissu::OBJLoader::load(path, pos, indices);

            return std::make_tuple(success, pos, indices);
        });

    py::class_<ConfigLoader>(m, "ConfigLoader")
        .def_static("load_material", &ConfigLoader::loadMaterial)
        .def_static("load_physics", &ConfigLoader::loadPhysics)
        .def_static("save_material", &ConfigLoader::saveMaterial)
        .def_static("save_physics", &ConfigLoader::savePhysics);

    py::class_<SceneLoader>(m, "SceneLoader")
        .def_static("load_scene", &SceneLoader::loadScene)
        .def_static("get_scene_header", &SceneLoader::getSceneHeader);

    py::class_<SceneExporter>(m, "SceneExporter")
        .def_static("save_scene", &SceneExporter::saveScene);

    py::class_<StateSerializer>(m, "StateSerializer")
        .def_static("load", &StateSerializer::load)
        .def_static("save", &StateSerializer::save)
        .def_static("get_state_info", &StateSerializer::getStateInfo);

    py::class_<Logger>(m, "Logger")
        .def_static("info", &Logger::info, py::arg("message"))
        .def_static("warn", &Logger::warn, py::arg("message"))
        .def_static("error", &Logger::error, py::arg("message"));

    py::class_<Tissu::Viewer::Renderer,
               std::unique_ptr<Tissu::Viewer::Renderer>>(m, "Renderer")
        .def("set_shader_path", &Tissu::Viewer::Renderer::setShaderPath,
             py::arg("path"),
             "Sets the directory where .vert and .frag files are located.");

    py::class_<Viewer::Application>(m, "Application")
        .def(py::init<>())
        .def("init", &Tissu::Viewer::Application::init, py::arg("width"),
             py::arg("height"), py::arg("title"), py::arg("shader_path"))
        .def("run", &Tissu::Viewer::Application::run)
        .def("shutdown", &Tissu::Viewer::Application::shutdown)
        .def("sync_visual_topology",
             &Tissu::Viewer::Application::syncVisualTopology)
        .def("set_solver", &Tissu::Viewer::Application::setSolver,
             py::arg("solver"))
        .def("set_cloth", &Tissu::Viewer::Application::setCloth)
        .def("set_mesh", &Tissu::Viewer::Application::setMesh, py::arg("mesh"))
        .def("set_aero_force", &Tissu::Viewer::Application::setAeroForce)
        .def("set_world", &Tissu::Viewer::Application::setWorld)
        .def("get_renderer", &Tissu::Viewer::Application::getRenderer,
             py::return_value_policy::reference_internal);

    py::class_<Tissu::AlembicExporter>(m, "AlembicExporter")
        .def(py::init<>())
        .def(
            "open",
            [](Tissu::AlembicExporter& self, const std::string& path,
               const std::vector<std::string>& names,
               py::array_t<double> global_positions,
               py::list global_indices_list, py::list particle_indices_list) {
                if (names.size() != global_indices_list.size() ||
                    names.size() != particle_indices_list.size()) {
                    throw std::runtime_error(
                        "names, global_indices_list, and particle_indices_list "
                        "must have the same length");
                }
                auto gp = global_positions.unchecked<2>();
                if (gp.shape(1) != 3) {
                    throw std::runtime_error(
                        "global_positions must have shape (N, 3)");
                }
                std::vector<Eigen::Vector3d> all_positions(gp.shape(0));
                for (ssize_t j = 0; j < gp.shape(0); ++j) {
                    all_positions[j] =
                        Eigen::Vector3d(gp(j, 0), gp(j, 1), gp(j, 2));
                }

                std::vector<std::vector<int>> all_global_indices;
                std::vector<std::vector<int>> all_particle_indices;
                all_global_indices.reserve(names.size());
                all_particle_indices.reserve(names.size());

                for (size_t i = 0; i < names.size(); ++i) {
                    auto indices =
                        global_indices_list[i].cast<py::array_t<int32_t>>();
                    auto idx = indices.unchecked<1>();
                    std::vector<int> iv(idx.shape(0));
                    for (ssize_t j = 0; j < idx.shape(0); ++j) {
                        iv[j] = idx(j);
                    }
                    all_global_indices.push_back(std::move(iv));

                    auto part_indices =
                        particle_indices_list[i].cast<py::array_t<int32_t>>();
                    auto pi = part_indices.unchecked<1>();
                    std::vector<int> pv(pi.shape(0));
                    for (ssize_t j = 0; j < pi.shape(0); ++j) {
                        pv[j] = pi(j);
                    }
                    all_particle_indices.push_back(std::move(pv));
                }
                return self.open(path, names, all_positions, all_global_indices,
                                 all_particle_indices);
            },
            py::arg("path"), py::arg("names"), py::arg("global_positions"),
            py::arg("global_indices"), py::arg("particle_indices"))
        .def(
            "write_frame",
            [](Tissu::AlembicExporter& self,
               py::array_t<double> global_positions, double time) {
                auto gp = global_positions.unchecked<2>();
                if (gp.shape(1) != 3) {
                    throw std::runtime_error(
                        "global_positions must have shape (N, 3)");
                }
                std::vector<Eigen::Vector3d> all_positions(gp.shape(0));
                for (ssize_t j = 0; j < gp.shape(0); ++j) {
                    all_positions[j] =
                        Eigen::Vector3d(gp(j, 0), gp(j, 1), gp(j, 2));
                }
                self.writeFrame(all_positions, time);
            },
            py::arg("global_positions"), py::arg("time"))
        .def("close", &Tissu::AlembicExporter::close);

    py::class_<OBJExporter>(m, "OBJExporter")
        .def(py::init<>())
        .def_static("export_obj", &OBJExporter::exportOBJ, py::arg("filename"),
                    py::arg("cloth"), py::arg("solver"),
                    "Exports a specific cloth instance to an OBJ file");
}