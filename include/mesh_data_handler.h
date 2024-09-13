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

class MeshDataHandler
{
public:
    MeshDataHandler();
    MeshDataHandler(std::list<rat::mdl::ShMeshDataPr> &mesh_data);

    std::pair<double, double> getMinMaxZValues();

private:
    rat::mdl::ShMeshDataPr getMeshData(int index);
    int getMeshDataSize();
    void get_z(rat::mdl::ShMeshDataPr mesh_data, arma::field<arma::dmat> &z);


    std::list<rat::mdl::ShMeshDataPr> mesh_data_;
};

#endif // MESH_DATA_HANDLER_H
