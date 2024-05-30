#include "harmonics_handler.h"

// class for handling the result of a harmonics calculation

// dummy constructor
HarmonicsHandler::HarmonicsHandler(){}

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

// function for getting the ell data. The ell data contains the x-coordinates (length along the magnet) for the y-values Bn
std::vector<double> HarmonicsHandler::get_ell(){
    return ell_;
}

// function for getting the Bn data for a B component. The Bn data indicates the strength of the component at certain points along the magnet (see ell)
std::vector<double> HarmonicsHandler::get_Bn(int component){
    // Bn_per_component is 0-indexed
    if (component > 0 && component <= Bn_per_component_.size()) {
        return Bn_per_component_[component-1];
    } else {
        return std::vector<double>();
    }
}

// function for getting the bn data for all components. The bn value for a components is computed using a fourier transformation on the Bn data. It describes the strength of the component along the whole magnet.
std::vector<double> HarmonicsHandler::get_bn(){
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
std::vector<double> HarmonicsHandler::convert_row_to_vector(const arma::Row<rat::fltp> &row){
    std::vector<double> vector;
    for (arma::uword i = 0; i <= row.n_elem - 1; ++i)
    {
        vector.push_back(row(i));
    }
    return vector;
}

// function for extracting the ell and all the Bn data. ell includes the length along the magnet and Bn is a matrix with the strength of each component at each of these ell locations.
std::tuple<std::vector<double>, std::vector<std::vector<double>>> HarmonicsHandler::extract_ell_Bn(rat::mdl::ShHarmonicsDataPr harmonics_data){
    arma::Row<rat::fltp> ell;
    arma::Mat<rat::fltp> An, Bn;
    harmonics_data->get_harmonics(ell, An, Bn);

    std::vector<double> ell_vector = convert_row_to_vector(ell);

    // stores a vector with the Bn values for all B components
    std::vector<std::vector<double>> all_Bn;

    // skip the first column (dummy 0 values) 
    for (arma::uword i = 1; i < Bn.n_cols; ++i)
    {
        arma::Row<rat::fltp> column = Bn.col(i);
        std::vector<double> bn_values = convert_row_to_vector(column);
        all_Bn.push_back(bn_values);
    }

    return {ell_vector, all_Bn};
}

// function for exporting all the component's Bn values to CSV files
void HarmonicsHandler::export_Bns_to_csv(const std::string& dir_path){
    // Create the directory if it doesn't exist
    if (!std::filesystem::exists(dir_path)){
        std::filesystem::create_directory(dir_path);
    } else {
        std::filesystem::remove_all(dir_path);
        std::filesystem::create_directory(dir_path);
    }
    
    // Export each component's Bn values to a separate CSV file
    for (int i = 1; i <= Bn_per_component_.size(); ++i){
        std::string file_path = dir_path + "/Bn_component_" + std::to_string(i) + ".csv";
        
        // export the file
        export_vector_to_csv(get_Bn(i), file_path);
    }
}
