#include "harmonics_data_handler.h"

// class for handling the result of a harmonics calculation

// dummy constructor
HarmonicsDataHandler::HarmonicsDataHandler() {}

HarmonicsDataHandler::HarmonicsDataHandler(rat::mdl::ShHarmonicsDataPr harmonics_data)
{
    if (harmonics_data)
    {
        harmonics_data_ = harmonics_data;
        // extract an and bn
        extract_an_bn(harmonics_data);
        // extract the ell (= length) and all the Bn data
        std::tie(ell_, Bn_per_component_) = extract_ell_Bn(harmonics_data);
    }
}

// constructor for manually injecting data - only used for testing
HarmonicsDataHandler::HarmonicsDataHandler(std::vector<double> ell, std::vector<std::vector<double>> Bn_per_component)
{
    ell_ = ell;
    Bn_per_component_ = Bn_per_component;
}

// Function for getting the Bn and ell data for a specific component. The Bn data indicates the strength [T] of the component at certain points along the magnet (see ell). Only returns data within the set ell bounds
std::vector<std::pair<double, double>> HarmonicsDataHandler::get_Bn(int component)
{

    std::vector<double> Bn_data = get_Bn_(component);
    std::vector<double> ell = get_ell_();

    if (ell.size() != Bn_data.size())
    {
        throw std::logic_error("Ell and Bn data must have the same length.");
    }

    // I do not now why the RAT library does this, but it is necessary to get the correct data
    if (component % 2 == 1)
    {
        for (double &value : Bn_data)
        {
            value *= -1;
        }
    }

    std::vector<std::pair<double, double>> data = combinePoints(ell, Bn_data);

    return data;
}

// function for getting the ell data in [mm]. The ell data contains the x-coordinates (length along the magnet) for the y-values Bn
std::vector<double> HarmonicsDataHandler::get_ell_()
{
    std::vector<double> modified_ell = ell_;
    ;
    for (double &value : modified_ell)
    {
        // convert from m to mm
        value *= 1000;
    }

    return modified_ell;
}

// function for getting ALL the Bn data for a B component. The Bn data indicates the strength of the component at certain points along the magnet (see ell)
std::vector<double> HarmonicsDataHandler::get_Bn_(int component)
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
std::vector<double> HarmonicsDataHandler::get_bn()
{
    return bn_;
}

// function for getting the an data for all components. 
std::vector<double> HarmonicsDataHandler::get_an(){
    return an_;
}

// function to extract and set the an and bn values from harmonics data
void HarmonicsDataHandler::extract_an_bn(rat::mdl::ShHarmonicsDataPr harmonics_data)
{
    arma::Row<rat::fltp> An, Bn;
    harmonics_data->get_harmonics(An, Bn);
    const arma::uword idx = arma::index_max(arma::max(arma::abs(An), arma::abs(Bn)));
    const rat::fltp ABmax = std::max(std::abs(An(idx)), std::abs(Bn(idx)));
    const arma::Row<rat::fltp> an = 1e4 * An / ABmax;
    const arma::Row<rat::fltp> bn = 1e4 * Bn / ABmax;
    an_ = convert_bn_to_vector(an);
    bn_ = convert_bn_to_vector(bn);
}

// function to convert an arma::Row of harmonics data to a vector, omitting the first value
std::vector<double> HarmonicsDataHandler::convert_bn_to_vector(const arma::Row<rat::fltp> &bn)
{
    std::vector<double> bn_values;
    for (arma::uword i = 1; i <= std::min(10u, (unsigned int)(bn.n_elem - 1)); ++i)
    {
        bn_values.push_back(bn(i));
    }
    return bn_values;
}

// converts an arma::Row to a std::vector
std::vector<double> HarmonicsDataHandler::convert_row_to_vector(const arma::Row<rat::fltp> &row)
{
    return std::vector<double>(row.begin(), row.end());
}

// Convert column to vector
std::vector<double> HarmonicsDataHandler::convert_col_to_vector(const arma::Col<rat::fltp> &col)
{
    return std::vector<double>(col.begin(), col.end());
}

// function for extracting the ell and all the Bn data. ell includes the length along the magnet and Bn is a matrix with the strength of each component at each of these ell locations.
std::tuple<std::vector<double>, std::vector<std::vector<double>>> HarmonicsDataHandler::extract_ell_Bn(rat::mdl::ShHarmonicsDataPr harmonics_data)
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