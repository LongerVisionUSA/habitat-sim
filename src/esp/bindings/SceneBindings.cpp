// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "esp/bindings/bindings.h"

#include <Magnum/Magnum.h>
#include <Magnum/SceneGraph/SceneGraph.h>

#include <Magnum/Python.h>
#include <Magnum/SceneGraph/Python.h>

#include "esp/scene/Mp3dSemanticScene.h"
#include "esp/scene/ObjectControls.h"
#include "esp/scene/SceneConfiguration.h"
#include "esp/scene/SceneGraph.h"
#include "esp/scene/SceneManager.h"
#include "esp/scene/SceneNode.h"
#include "esp/scene/SemanticScene.h"
#include "esp/scene/SuncgSemanticScene.h"

namespace py = pybind11;
using py::literals::operator""_a;

namespace esp {
namespace scene {

void initSceneBindings(py::module& m) {
  // ==== SceneConfiguration ====
  py::class_<SceneConfiguration, SceneConfiguration::ptr>(m,
                                                          "SceneConfiguration")
      .def(py::init(&SceneConfiguration::create<>))
      .def_readwrite("dataset", &SceneConfiguration::dataset)
      .def_readwrite("id", &SceneConfiguration::id)
      .def_readwrite("filepaths", &SceneConfiguration::filepaths)
      .def_readwrite("scene_up_dir", &SceneConfiguration::sceneUpDir)
      .def_readwrite("scene_front_dir", &SceneConfiguration::sceneFrontDir)
      .def_readwrite("scene_scale_unit", &SceneConfiguration::sceneScaleUnit)
      .def(
          "__eq__",
          [](const SceneConfiguration& self,
             const SceneConfiguration& other) -> bool { return self == other; })
      .def("__neq__",
           [](const SceneConfiguration& self, const SceneConfiguration& other)
               -> bool { return self != other; });

  // ==== SceneGraph ====
  py::class_<SceneGraph>(m, "SceneGraph")
      .def(py::init())
      .def("get_root_node",
           py::overload_cast<>(&SceneGraph::getRootNode, py::const_),
           R"(
            Get the root node of the scene graph.

            User can specify transformation of the root node w.r.t. the world
            frame. (const function) PYTHON DOES NOT GET OWNERSHIP)",
           pybind11::return_value_policy::reference)
      .def("get_root_node", py::overload_cast<>(&SceneGraph::getRootNode),
           R"(
            Get the root node of the scene graph.

            User can specify transformation of the root node w.r.t. the world
            frame. PYTHON DOES NOT GET OWNERSHIP)",
           pybind11::return_value_policy::reference)
      .def("set_default_render_camera_parameters",
           &SceneGraph::setDefaultRenderCamera,
           R"(
            Set transformation and the projection matrix to the default render camera.

            The camera will have the same absolute transformation as the target
            scene node after the operation.)",
           "targetSceneNode"_a)
      .def("get_default_render_camera", &SceneGraph::getDefaultRenderCamera,
           R"(
            Get the default camera stored in scene graph for rendering.

            PYTHON DOES NOT GET OWNERSHIP)",
           pybind11::return_value_policy::reference);

  // ==== SceneManager ====
  py::class_<SceneManager>(m, "SceneManager")
      .def("init_scene_graph", &SceneManager::initSceneGraph,
           R"(
          Initialize a new scene graph, and return its ID.)")
      .def("get_scene_graph",
           py::overload_cast<int>(&SceneManager::getSceneGraph),
           R"(
             Get the scene graph by scene graph ID.

             PYTHON DOES NOT GET OWNERSHIP)",
           "sceneGraphID"_a, pybind11::return_value_policy::reference)
      .def("get_scene_graph",
           py::overload_cast<int>(&SceneManager::getSceneGraph, py::const_),
           R"(
             Get the scene graph by scene graph ID.

             PYTHON DOES NOT GET OWNERSHIP)",
           "sceneGraphID"_a, pybind11::return_value_policy::reference);

  // !!Warning!!
  // CANNOT apply smart pointers to "SceneNode" or ANY its descendant classes,
  // namely, any class whose instance can be a node in the scene graph. Reason:
  // Memory will be automatically handled in simulator (C++ backend). Using
  // smart pointers on scene graph node from Python code, will claim the
  // ownership and eventually free its resources, which leads to "duplicated
  // deallocation", and thus memory corruption.

  // ==== enum SceneNodeType ====
  py::enum_<SceneNodeType>(m, "SceneNodeType")
      .value("EMPTY", SceneNodeType::EMPTY)
      .value("SENSOR", SceneNodeType::SENSOR)
      .value("AGENT", SceneNodeType::AGENT)
      .value("CAMERA", SceneNodeType::CAMERA);

  // ==== SceneNode ====
  py::class_<SceneNode, Magnum::SceneGraph::PyObject<SceneNode>, MagnumObject,
             Magnum::SceneGraph::PyObjectHolder<SceneNode>>(m, "SceneNode", R"(
      SceneNode: a node in the scene graph.

      Cannot apply a smart pointer to a SceneNode object.
      You can "create it and forget it".
      Simulator backend will handle the memory.)")
      .def(py::init_alias<std::reference_wrapper<SceneNode>>(),
           R"(Constructor: creates a scene node, and sets its parent.)")
      .def_property("type", &SceneNode::getType, &SceneNode::setType)
      .def(
          "create_child", [](SceneNode& self) { return &self.createChild(); },
          R"(Creates a child node, and sets its parent to the current node.)")
      .def_property_readonly("absolute_translation",
                             &SceneNode::absoluteTranslation);

  // ==== SemanticCategory ====
  py::class_<SemanticCategory, SemanticCategory::ptr>(m, "SemanticCategory")
      .def("index", &SemanticCategory::index, "mapping"_a = "")
      .def("name", &SemanticCategory::name, "mapping"_a = "");

  // === Mp3dObjectCategory ===
  py::class_<Mp3dObjectCategory, SemanticCategory, Mp3dObjectCategory::ptr>(
      m, "Mp3dObjectCategory")
      .def("index", &Mp3dObjectCategory::index, "mapping"_a = "")
      .def("name", &Mp3dObjectCategory::name, "mapping"_a = "");

  // === Mp3dRegionCategory ===
  py::class_<Mp3dRegionCategory, SemanticCategory, Mp3dRegionCategory::ptr>(
      m, "Mp3dRegionCategory")
      .def("index", &Mp3dRegionCategory::index, "mapping"_a = "")
      .def("name", &Mp3dRegionCategory::name, "mapping"_a = "");

  // === SuncgObjectCategory ===
  py::class_<SuncgObjectCategory, SemanticCategory, SuncgObjectCategory::ptr>(
      m, "SuncgObjectCategory")
      .def("index", &SuncgObjectCategory::index, "mapping"_a = "")
      .def("name", &SuncgObjectCategory::name, "mapping"_a = "");

  // === SuncgRegionCategory ===
  py::class_<SuncgRegionCategory, SemanticCategory, SuncgRegionCategory::ptr>(
      m, "SuncgRegionCategory")
      .def("index", &SuncgRegionCategory::index, "mapping"_a = "")
      .def("name", &SuncgRegionCategory::name, "mapping"_a = "");

  // These two are (cyclically) referenced by multiple classes below, define
  // the classes first so pybind has the type definition available when binding
  // functions
  py::class_<SemanticObject, SemanticObject::ptr> semanticObject(
      m, "SemanticObject");
  py::class_<SemanticRegion, SemanticRegion::ptr> semanticRegion(
      m, "SemanticRegion");

  // ==== SemanticLevel ====
  py::class_<SemanticLevel, SemanticLevel::ptr>(m, "SemanticLevel")
      .def_property_readonly("id", &SemanticLevel::id)
      .def_property_readonly("aabb", &SemanticLevel::aabb)
      .def_property_readonly("regions", &SemanticLevel::regions)
      .def_property_readonly("objects", &SemanticLevel::objects);

  // ==== SemanticRegion ====
  semanticRegion
      .def_property_readonly(
          "id", &SemanticRegion::id,
          "The ID of the region, of the form ``<level_id>_<region_id>``")
      .def_property_readonly("level", &SemanticRegion::level)
      .def_property_readonly("aabb", &SemanticRegion::aabb)
      .def_property_readonly("category", &SemanticRegion::category,
                             "The semantic category of the region")
      .def_property_readonly("objects", &SemanticRegion::objects,
                             "All objects in the region");

  // ==== SuncgSemanticRegion ====
  py::class_<SuncgSemanticRegion, SemanticRegion, SuncgSemanticRegion::ptr>(
      m, "SuncgSemanticRegion")
      .def_property_readonly("id", &SuncgSemanticRegion::id)
      .def_property_readonly("level", &SuncgSemanticRegion::level)
      .def_property_readonly("aabb", &SuncgSemanticRegion::aabb)
      .def_property_readonly("category", &SuncgSemanticRegion::category)
      .def_property_readonly("objects", &SuncgSemanticRegion::objects);

  // ==== SemanticObject ====
  semanticObject
      .def_property_readonly("id", &SemanticObject::id,
                             "The ID of the object, of the form "
                             "``<level_id>_<region_id>_<object_id>``")
      .def_property_readonly("region", &SemanticObject::region)
      .def_property_readonly("aabb", &SemanticObject::aabb)
      .def_property_readonly("obb", &SemanticObject::obb)
      .def_property_readonly("category", &SemanticObject::category);

  // ==== SuncgSemanticObject ====
  py::class_<SuncgSemanticObject, SemanticObject, SuncgSemanticObject::ptr>(
      m, "SuncgSemanticObject")
      .def_property_readonly("id", &SuncgSemanticObject::id)
      .def_property_readonly("region", &SuncgSemanticObject::region)
      .def_property_readonly("aabb", &SuncgSemanticObject::aabb)
      .def_property_readonly("obb", &SuncgSemanticObject::obb)
      .def_property_readonly("category", &SuncgSemanticObject::category,
                             "The semantic category of the object.");

  // ==== SemanticScene ====
  py::class_<SemanticScene, SemanticScene::ptr>(m, "SemanticScene")
      .def(py::init(&SemanticScene::create<>))
      .def_static(
          "load_mp3d_house",
          [](const std::string& filename, SemanticScene& scene,
             const vec4f& rotation) {
            // numpy doesn't have a quaternion equivalent, use vec4
            // instead
            return SemanticScene::loadMp3dHouse(
                filename, scene, Eigen::Map<const quatf>(rotation.data()));
          },
          R"(
        Loads a SemanticScene from a Matterport3D House format file into passed
        `SemanticScene`.
      )",
          "file"_a, "scene"_a, "rotation"_a)
      .def_property_readonly("aabb", &SemanticScene::aabb)
      .def_property_readonly("categories", &SemanticScene::categories,
                             "All semantic categories in the house")
      .def_property_readonly("levels", &SemanticScene::levels,
                             "All levels in the house")
      .def_property_readonly("regions", &SemanticScene::regions,
                             "All regions in the house")
      .def_property_readonly("objects", &SemanticScene::objects,
                             "All object in the house")
      .def_property_readonly("semantic_index_map",
                             &SemanticScene::getSemanticIndexMap)
      .def("semantic_index_to_object_index",
           &SemanticScene::semanticIndexToObjectIndex);

  // ==== ObjectControls ====
  py::class_<ObjectControls, ObjectControls::ptr>(m, "ObjectControls")
      .def(py::init(&ObjectControls::create<>))
      .def("action", &ObjectControls::action, R"(
        Take action using this :py:class:`ObjectControls`.
      )",
           "object"_a, "name"_a, "amount"_a, "apply_filter"_a = true);
}

}  // namespace scene
}  // namespace esp
