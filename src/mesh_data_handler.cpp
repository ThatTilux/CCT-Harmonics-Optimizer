#include "mesh_data_handler.h"

// class for handling the result of a mesh calculation

// dummy constructor
MeshDataHandler::MeshDataHandler() {}

MeshDataHandler::MeshDataHandler(std::list<rat::mdl::ShMeshDataPr> &mesh_data)
{
    if (!mesh_data.empty())
    {
        mesh_data_ = mesh_data;
    } else {
        throw std::runtime_error("Mesh data is empty");
    }
}

// Function to get the min and max z values of the mesh data
std::pair<double, double> MeshDataHandler::getMinMaxZValues()
{
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::min();

    for (int i = 0; i < getMeshDataSize(); i++)
    {
        auto mesh_data = getMeshData(i);
        arma::field<arma::dmat> z;
        get_z(mesh_data, z);

        // assert that z only has 0 or 1 element(s)
        if (z.size() > 1)
        {
            throw std::runtime_error("z has more than 1 element. Size: " + std::to_string(z.size()));
        }
        // this corresponds to a mesh calculation for an object that is not a magnet. In this case, z is empty
        if (z.size() == 0)
        {
            continue;
        }


        double temp_min = z(0).min();
        double temp_max = z(0).max();

        if (temp_min < min)
        {
            min = temp_min;
        }
        if (temp_max > max)
        {
            max = temp_max;
        }
    }
    return std::make_pair(min, max);
}

// Function to get the z values of the mesh data
void MeshDataHandler::get_z(rat::mdl::ShMeshDataPr mesh_data, arma::field<arma::dmat> &z)
{
    arma::field<arma::dmat> x;
    arma::field<arma::dmat> y;
    arma::uword num_edges;

    mesh_data->create_xyz(x, y, z, num_edges);

}

rat::mdl::ShMeshDataPr MeshDataHandler::getMeshData(int index)
{
    if (index < 0 || index >= mesh_data_.size())
    {
        throw new std::runtime_error("Index out of bounds for Mesh Data access");
    }

    auto it = mesh_data_.begin();
    std::advance(it, index);

    return *it;
}

int MeshDataHandler::getMeshDataSize()
{
    return mesh_data_.size();
}
