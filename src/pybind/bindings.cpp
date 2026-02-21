// Python bindings for theWheel using pybind11
// This provides Python access to core theWheelModel classes for testing

// Define these before any headers
#define _AFXDLL
#define NOMINMAX

// Include MFC/Windows headers BEFORE pybind11 to avoid conflicts
#include <afx.h>
#include <afxwin.h>

// Undefine problematic macros that conflict with Python
#ifdef _DEBUG
#undef _DEBUG
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#define _DEBUG
#else
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#endif

// Include utility macros first
#include "UtilMacros.h"

// Now include theWheel headers
#include "Node.h"
#include "Space.h"
#include "NodeLink.h"
#include "VectorD.h"

namespace py = pybind11;

// Helper to convert CString to std::string
std::string CStringToStdString(const CString& cstr) {
    return std::string(CT2A(cstr));
}

// Helper to convert std::string to CString
CString StdStringToCString(const std::string& str) {
    return CString(str.c_str());
}

PYBIND11_MODULE(pythewheel, m) {
    m.doc() = "Python bindings for theWheel semantic visualization system";

    // Bind CVectorD<3> (3D position vector)
    py::class_<CVectorD<3>>(m, "Vector3D")
        .def(py::init<>())
        .def(py::init<REAL, REAL, REAL>())
        .def("__getitem__", [](const CVectorD<3>& v, int i) -> REAL {
            if (i < 0 || i >= 3) throw py::index_error();
            return v[i];
        })
        .def("__setitem__", [](CVectorD<3>& v, int i, REAL value) {
            if (i < 0 || i >= 3) throw py::index_error();
            v[i] = value;
        })
        .def("get_length", &CVectorD<3>::GetLength)
        .def("normalize", &CVectorD<3>::Normalize)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * REAL())
        .def(REAL() * py::self)
        .def("__repr__", [](const CVectorD<3>& v) {
            return "Vector3D(" + std::to_string(v[0]) + ", " +
                   std::to_string(v[1]) + ", " + std::to_string(v[2]) + ")";
        });

    // Bind CNodeLink
    py::class_<CNodeLink>(m, "NodeLink")
        .def(py::init<CNode*, REAL>(),
             py::arg("target") = nullptr,
             py::arg("weight") = 0.0)
        .def("get_weight", &CNodeLink::GetWeight)
        .def("get_gain", &CNodeLink::GetGain)
        .def("set_gain", &CNodeLink::SetGain)
        .def("get_gain_weight", &CNodeLink::GetGainWeight)
        .def("get_target", &CNodeLink::GetTarget, py::return_value_policy::reference)
        .def("set_target", &CNodeLink::SetTarget)
        .def("is_stabilizer", &CNodeLink::GetIsStabilizer)
        .def("set_is_stabilizer", &CNodeLink::SetIsStabilizer);

    // Bind CNode
    py::class_<CNode>(m, "Node")
        .def(py::init<CSpace*, const CString&, const CString&>(),
             py::arg("space") = nullptr,
             py::arg("name") = CString(_T("")),
             py::arg("description") = CString(_T("")))
        // Hierarchy
        .def("get_space", &CNode::GetSpace, py::return_value_policy::reference)
        .def("get_parent", &CNode::GetParent, py::return_value_policy::reference)
        .def("set_parent", &CNode::SetParent)
        .def("get_child_count", &CNode::GetChildCount)
        .def("get_child_at", &CNode::GetChildAt, py::return_value_policy::reference)
        .def("get_descendant_count", &CNode::GetDescendantCount)
        // Attributes - wrap CString conversions
        .def("get_name", [](CNode* node) {
            return CStringToStdString(node->GetName());
        })
        .def("set_name", [](CNode* node, const std::string& name) {
            node->SetName(StdStringToCString(name));
        })
        .def("get_description", [](CNode* node) {
            return CStringToStdString(node->GetDescription());
        })
        .def("set_description", [](CNode* node, const std::string& desc) {
            node->SetDescription(StdStringToCString(desc));
        })
        .def("get_class", [](CNode* node) {
            return CStringToStdString(node->GetClass());
        })
        .def("set_class", [](CNode* node, const std::string& cls) {
            node->SetClass(StdStringToCString(cls));
        })
        // Links
        .def("get_link_count", &CNode::GetLinkCount)
        .def("get_link_at", &CNode::GetLinkAt, py::return_value_policy::reference)
        .def("get_link_to", &CNode::GetLinkTo, py::return_value_policy::reference)
        .def("get_link_weight", &CNode::GetLinkWeight)
        .def("link_to", &CNode::LinkTo,
             py::arg("to_node"),
             py::arg("weight"),
             py::arg("reciprocal") = true)
        .def("unlink", &CNode::Unlink,
             py::arg("node"),
             py::arg("reciprocal") = true)
        // Activation
        .def("get_activation", &CNode::GetActivation)
        .def("set_activation", &CNode::SetActivation)
        .def("get_primary_activation", &CNode::GetPrimaryActivation)
        .def("set_primary_activation", &CNode::SetPrimaryActivation)
        .def("get_secondary_activation", &CNode::GetSecondaryActivation)
        .def("set_secondary_activation", &CNode::SetSecondaryActivation)
        // Position
        .def("get_position", &CNode::GetPosition, py::return_value_policy::reference)
        .def("set_position", &CNode::SetPosition);

    // Bind CSpace
    py::class_<CSpace>(m, "Space")
        .def(py::init<>())
        // Hierarchy
        .def("get_root_node", &CSpace::GetRootNode, py::return_value_policy::reference)
        .def("set_root_node", &CSpace::SetRootNode)
        .def("get_node_count", &CSpace::GetNodeCount)
        .def("get_node_at", &CSpace::GetNodeAt, py::return_value_policy::reference)
        .def("add_node", &CSpace::AddNode,
             py::arg("new_node"),
             py::arg("parent_node") = nullptr)
        .def("remove_node", &CSpace::RemoveNode)
        // Current node
        .def("get_current_node", &CSpace::GetCurrentNode, py::return_value_policy::reference)
        .def("set_current_node", &CSpace::SetCurrentNode)
        // Path
        .def("get_path_name", [](CSpace* space) {
            return CStringToStdString(space->GetPathName());
        })
        .def("set_path_name", [](CSpace* space, const std::string& path) {
            space->SetPathName(StdStringToCString(path));
        })
        // Activation
        .def("activate_node", &CSpace::ActivateNode)
        .def("get_prim_sec_ratio", [](CSpace* space) { return space->GetPrimSecRatio(); })
        .def("set_prim_sec_ratio", [](CSpace* space, REAL ratio) { space->SetPrimSecRatio(ratio); })
        .def("normalize_nodes", &CSpace::NormalizeNodes, py::arg("sum") = 1.0)
        .def("sort_nodes", &CSpace::SortNodes)
        .def("get_total_activation", &CSpace::GetTotalActivation, py::arg("compute") = false)
        .def("get_total_primary_activation", &CSpace::GetTotalPrimaryActivation, py::arg("compute") = false)
        .def("get_total_secondary_activation", &CSpace::GetTotalSecondaryActivation, py::arg("compute") = false)
        // Layout
        .def("get_center", [](CSpace* space) -> const CVectorD<3>& { return space->GetCenter(); }, py::return_value_policy::reference)
        .def("set_center", [](CSpace* space, const CVectorD<3>& center) { space->SetCenter(center); })
        .def("get_spring_const", [](CSpace* space) { return space->GetSpringConst(); })
        .def("set_spring_const", [](CSpace* space, REAL k) { space->SetSpringConst(k); });
}
