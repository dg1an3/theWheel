// Python bindings for THEWHEEL_MODEL using pybind11
// Copyright (C) 2025 - Python bindings for theWheel semantic network
// Original theWheel code: Copyright (C) 1999-2002 Derek Graham Lane

// IMPORTANT: Include Windows/MFC headers BEFORE pybind11 to avoid type conflicts
#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

#include <afx.h>
#include <afxwin.h>
#include <windows.h>
#include <mmsystem.h>  // For multimedia types like WAVEFORMATEX, HMMIO
#include <dsound.h>    // For DirectSound types

// Include XMLLogging first (needed by other headers)
#include "XMLLogging.h"
#include "XMLLogFile.h"
#include "XMLElement.h"

// Include the core theWheel model headers (which depend on MFC)
#include "Space.h"
#include "Node.h"
#include "NodeLink.h"
#include "Observer.h"

// Undefine Windows macros that conflict with pybind11 and C++ standard library
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef DrawText
#undef DrawText
#endif

// Now include pybind11 headers (after Windows types are defined and conflicting macros removed)
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/operators.h>

namespace py = pybind11;

// Helper function to convert CString to std::string
std::string CStringToStdString(const CString& cstr) {
    return std::string(CT2A(cstr));
}

// Helper function to convert std::string to CString
CString StdStringToCString(const std::string& str) {
    return CString(str.c_str());
}

// Wrapper class for CNode to handle Python-friendly interfaces
class PyNode {
public:
    CNode* node;

    PyNode(CNode* n) : node(n) {}

    std::string getName() const {
        return CStringToStdString(node->GetName());
    }

    void setName(const std::string& name) {
        node->SetName(StdStringToCString(name));
    }

    std::string getDescription() const {
        return CStringToStdString(node->GetDescription());
    }

    void setDescription(const std::string& desc) {
        node->SetDescription(StdStringToCString(desc));
    }

    REAL getActivation() const {
        return node->GetActivation();
    }

    int getLinkCount() const {
        return node->GetLinkCount();
    }

    PyNode* getChildAt(int index) {
        CNode* child = node->GetChildAt(index);
        return child ? new PyNode(child) : nullptr;
    }

    int getChildCount() const {
        return node->GetChildCount();
    }

    void linkTo(PyNode* target, REAL weight, bool reciprocal = true) {
        if (target && target->node) {
            node->LinkTo(target->node, weight, reciprocal);
        }
    }
};

// Wrapper class for CSpace
class PySpace {
public:
    CSpace* space;
    bool ownsSpace;

    PySpace() : ownsSpace(true) {
        space = new CSpace();
    }

    PySpace(CSpace* s, bool owns = false) : space(s), ownsSpace(owns) {}

    ~PySpace() {
        if (ownsSpace && space) {
            delete space;
        }
    }

    PyNode* getRootNode() {
        CNode* root = space->GetRootNode();
        return root ? new PyNode(root) : nullptr;
    }

    int getNodeCount() const {
        return space->GetNodeCount();
    }

    PyNode* getNodeAt(int index) {
        CNode* node = space->GetNodeAt(index);
        return node ? new PyNode(node) : nullptr;
    }

    PyNode* getCurrentNode() {
        CNode* current = space->GetCurrentNode();
        return current ? new PyNode(current) : nullptr;
    }

    void setCurrentNode(PyNode* node) {
        if (node && node->node) {
            space->SetCurrentNode(node->node);
        }
    }

    void activateNode(PyNode* node, REAL scale) {
        if (node && node->node) {
            space->ActivateNode(node->node, scale);
        }
    }

    void normalizeNodes(REAL sum = 1.0) {
        space->NormalizeNodes(sum);
    }

    REAL getTotalActivation() const {
        return space->GetTotalActivation(TRUE);
    }

    void layoutNodes() {
        space->LayoutNodes();
    }

    bool createSimpleSpace() {
        return space->CreateSimpleSpace() != 0;
    }
};

// Module definition
PYBIND11_MODULE(thewheel, m) {
    m.doc() = "Python bindings for theWheel semantic network system";

    // Expose PyNode class
    py::class_<PyNode>(m, "Node")
        .def(py::init<CNode*>())
        .def_property("name", &PyNode::getName, &PyNode::setName,
            "Get or set the node's name")
        .def_property("description", &PyNode::getDescription, &PyNode::setDescription,
            "Get or set the node's description")
        .def_property_readonly("activation", &PyNode::getActivation,
            "Get the node's current activation level")
        .def_property_readonly("link_count", &PyNode::getLinkCount,
            "Get the number of links from this node")
        .def_property_readonly("child_count", &PyNode::getChildCount,
            "Get the number of children")
        .def("get_child_at", &PyNode::getChildAt,
            py::arg("index"),
            py::return_value_policy::take_ownership,
            "Get child node at given index")
        .def("link_to", &PyNode::linkTo,
            py::arg("target"), py::arg("weight"), py::arg("reciprocal") = true,
            "Create a weighted link to another node");

    // Expose PySpace class
    py::class_<PySpace>(m, "Space")
        .def(py::init<>(),
            "Create a new semantic space")
        .def_property_readonly("root_node", &PySpace::getRootNode,
            py::return_value_policy::take_ownership,
            "Get the root node of the space")
        .def_property_readonly("node_count", &PySpace::getNodeCount,
            "Get the total number of nodes in the space")
        .def("get_node_at", &PySpace::getNodeAt,
            py::arg("index"),
            py::return_value_policy::take_ownership,
            "Get node at given index (sorted by activation)")
        .def_property("current_node", &PySpace::getCurrentNode, &PySpace::setCurrentNode,
            py::return_value_policy::take_ownership,
            "Get or set the current active node")
        .def("activate_node", &PySpace::activateNode,
            py::arg("node"), py::arg("scale"),
            "Activate a node with given scale factor")
        .def("normalize_nodes", &PySpace::normalizeNodes,
            py::arg("sum") = 1.0,
            "Normalize all node activations to sum")
        .def_property_readonly("total_activation", &PySpace::getTotalActivation,
            "Get total activation across all nodes")
        .def("layout_nodes", &PySpace::layoutNodes,
            "Run force-directed layout algorithm on the graph")
        .def("create_simple_space", &PySpace::createSimpleSpace,
            "Create a simple test space with example nodes");

    // Expose constants
    m.attr("TOTAL_ACTIVATION") = TOTAL_ACTIVATION;

    // Version info
    m.attr("__version__") = "0.1.0";
    m.attr("__author__") = "Derek Graham Lane (original), Python bindings 2025";
}
