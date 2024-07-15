#include "mesh_data_handler.h"

// class for handling the result of a mesh calculation

// dummy constructor
MeshDataHandler::MeshDataHandler() {}

MeshDataHandler::MeshDataHandler(rat::mdl::ShMeshDataPr mesh_data)
{
    if (mesh_data)
    {
        mesh_data_ = mesh_data;
    }
}
