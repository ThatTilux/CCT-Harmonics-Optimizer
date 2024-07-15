#ifndef MESH_DATA_HANDLER_H
#define MESH_DATA_HANDLER_H

#include <armadillo>
#include <boost/filesystem.hpp>
#include <rat/common/log.hh>
#include <rat/models/serializer.hh>
#include <rat/models/modelroot.hh>
#include <rat/models/calcgroup.hh>
#include <rat/models/calcmesh.hh>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <input_output.h>
#include "constants.h"

class MeshDataHandler
{
public:
    MeshDataHandler();
    MeshDataHandler(rat::mdl::ShMeshDataPr mesh_data);

private:
    rat::mdl::ShMeshDataPr mesh_data_;
   
};


#endif // MESH_DATA_HANDLER_H
