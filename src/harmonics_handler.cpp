#include "harmonics_handler.h"

// class for handling the result of a harmonics calculation

// dummy constructor
HarmonicsHandler::HarmonicsHandler() {}

HarmonicsHandler::HarmonicsHandler(rat::mdl::ShHarmonicsDataPr harmonics_data)
{
    if (harmonics_data)
    {
        harmonics_data_ = harmonics_data;
        // extract bn
        bn_ = extract_bn(harmonics_data);
        // extract the ell (= length) and all the Bn data
        std::tie(ell_, Bn_per_component_) = extract_ell_Bn(harmonics_data);
    }
}

// constructor for manually injecting data - only used for testing
HarmonicsHandler::HarmonicsHandler(std::vector<double> ell, std::vector<std::vector<double>> Bn_per_component)
{
    ell_ = ell;
    Bn_per_component_ = Bn_per_component;
}

// Function for getting the Bn and ell data for a specific component. The Bn data indicates the strength [T] of the component at certain points along the magnet (see ell). Only returns data within the set ell bounds
std::vector<std::pair<double, double>> HarmonicsHandler::get_Bn(int component)
{

    std::vector<double> Bn_data = get_Bn_(component);
    std::vector<double> ell = get_ell_();


    if (ell.size() != Bn_data.size()){
        throw std::logic_error("Ell and Bn data must have the same length.");
    }

    // TODO TEMP REMOVE THIS!!
    if(component % 2 == 1){
        for (double& value : Bn_data)
        {
            value *= -1;
        }
    }

    std::vector<std::pair<double, double>> data = combinePoints(ell, Bn_data);

    // remove all the pairs where the ell value is not inside the set bounds
    data.erase(std::remove_if(data.begin(), data.end(), [](const std::pair<double, double> &pair)
                              { return pair.first < MAG_START_POS || pair.first > MAG_END_POS; }),
               data.end());


    // shift the ell data point so that the first point is at 0
    double shift = data[0].first;
    for (std::pair<double, double> &pair : data)
    {
        pair.first -= shift;
    }

    return data;
}

// TODO only paply these shifts when getting data for chisquared
// function for getting the ell data in [mm]. The ell data contains the x-coordinates (length along the magnet) for the y-values Bn
std::vector<double> HarmonicsHandler::get_ell_()
{
    std::vector<double> modified_ell = ell_;;
    for (double& value : modified_ell)
    {
        // convert from m to mm
        value *= 1000;
    }
    
    return modified_ell;
}

// function for getting ALL the Bn data for a B component. The Bn data indicates the strength of the component at certain points along the magnet (see ell)
std::vector<double> HarmonicsHandler::get_Bn_(int component)
{
    // Bn_per_component is 0-indexed
    if (component > 0 && component <= Bn_per_component_.size())
    {
        return Bn_per_component_[component - 1];
    }
    else
    {
        return std::vector<double>();
    }
}

// function for getting the bn data for all components. The bn value for a components is computed using a fourier transformation on the Bn data. It describes the strength of the component along the whole magnet. The resulting vector is 0-indexed
std::vector<double> HarmonicsHandler::get_bn()
{
    return bn_;
}

// function to extract the bn values from harmonics data
std::vector<double> HarmonicsHandler::extract_bn(rat::mdl::ShHarmonicsDataPr harmonics_data)
{
    arma::Row<rat::fltp> An, Bn;
    harmonics_data->get_harmonics(An, Bn);
    const arma::uword idx = arma::index_max(arma::max(arma::abs(An), arma::abs(Bn)));
    const rat::fltp ABmax = std::max(std::abs(An(idx)), std::abs(Bn(idx)));
    const arma::Row<rat::fltp> an = 1e4 * An / ABmax;
    const arma::Row<rat::fltp> bn = 1e4 * Bn / ABmax;
    return convert_bn_to_vector(bn);
}

// function to convert an arma::Row of harmonics data to a vector, omitting the first value
std::vector<double> HarmonicsHandler::convert_bn_to_vector(const arma::Row<rat::fltp> &bn)
{
    std::vector<double> bn_values;
    for (arma::uword i = 1; i <= std::min(10u, (unsigned int)(bn.n_elem - 1)); ++i)
    {
        bn_values.push_back(bn(i));
    }
    return bn_values;
}

// converts an arma::Row to a std::vector
std::vector<double> HarmonicsHandler::convert_row_to_vector(const arma::Row<rat::fltp> &row)
{
    return std::vector<double>(row.begin(), row.end());
}

// Convert column to vector
std::vector<double> HarmonicsHandler::convert_col_to_vector(const arma::Col<rat::fltp> &col)
{
    return std::vector<double>(col.begin(), col.end());
}

// function for extracting the ell and all the Bn data. ell includes the length along the magnet and Bn is a matrix with the strength of each component at each of these ell locations.
std::tuple<std::vector<double>, std::vector<std::vector<double>>> HarmonicsHandler::extract_ell_Bn(rat::mdl::ShHarmonicsDataPr harmonics_data)
{
    arma::Row<rat::fltp> ell;
    arma::Mat<rat::fltp> An, Bn;
    harmonics_data->get_harmonics(ell, An, Bn);

    std::vector<double> ell_vector = convert_row_to_vector(ell);

    // stores a vector with the Bn values for all B components
    std::vector<std::vector<double>> all_Bn;

    // skip the first column (dummy 0 values)
    for (arma::uword i = 1; i < Bn.n_cols; ++i)
    {
        arma::Col<rat::fltp> column = Bn.col(i);
        std::vector<double> bn_values = convert_col_to_vector(column);
        all_Bn.push_back(bn_values);
    }

    return {ell_vector, all_Bn};
}

// function for exporting all the component's Bn values to CSV files
void HarmonicsHandler::export_Bns_to_csv(const std::string &dir_path)
{
    // Create the directory if it doesn't exist
    if (!std::filesystem::exists(dir_path))
    {
        std::filesystem::create_directory(dir_path);
    }
    else
    {
        std::filesystem::remove_all(dir_path);
        std::filesystem::create_directory(dir_path);
    }

    // Export each component's Bn values to a separate CSV file
    for (int i = 1; i <= Bn_per_component_.size(); ++i)
    {
        std::string file_path = dir_path + "/Bn_component_" + std::to_string(i) + ".csv";

        std::vector<std::pair<double, double>> data = get_Bn(i);

        // export the Bn values
        export_data_to_csv(data, file_path);
    }
}

// Function to combine x and y points into a vector of pairs
std::vector<std::pair<double, double>> combinePoints(const std::vector<double> &x, const std::vector<double> &y)
{
    if (x.size() != y.size())
    {
        throw std::runtime_error("Vectors x and y must have the same length.");
    }

    std::vector<std::pair<double, double>> points;
    for (size_t i = 0; i < x.size(); ++i)
    {
        points.emplace_back(x[i], y[i]);
    }
    return points;
}