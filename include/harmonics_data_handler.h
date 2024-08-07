#ifndef HARMONICS_DATA_HANDLER_H
#define HARMONICS_DATA_HANDLER_H

#include <armadillo>
#include <boost/filesystem.hpp>
#include <rat/common/log.hh>
#include <rat/models/serializer.hh>
#include <rat/models/modelroot.hh>
#include <rat/models/calcgroup.hh>
#include <rat/models/calcharmonics.hh>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <input_output.h>
#include "constants.h"

class HarmonicsDataHandler
{
public:
    HarmonicsDataHandler();
    HarmonicsDataHandler(rat::mdl::ShHarmonicsDataPr harmonics_data);
    HarmonicsDataHandler(std::vector<double> ell, std::vector<std::vector<double>> Bn_per_component);
    std::vector<double> get_bn();
    void export_Bns_to_csv(const std::string &dir_path);
    std::vector<std::pair<double, double>> get_Bn(int component);

private:
    std::vector<double> convert_bn_to_vector(const arma::Row<rat::fltp> &bn);
    std::vector<double> extract_bn(rat::mdl::ShHarmonicsDataPr harmonics_data);
    std::vector<double> convert_row_to_vector(const arma::Row<rat::fltp> &row);
    std::vector<double> convert_col_to_vector(const arma::Col<rat::fltp> &col);
    std::tuple<std::vector<double>, std::vector<std::vector<double>>> extract_ell_Bn(rat::mdl::ShHarmonicsDataPr harmonics_data);
    std::vector<double> get_ell_();
    std::vector<double> get_Bn_(int component);

    rat::mdl::ShHarmonicsDataPr harmonics_data_;
    std::vector<double> bn_;
    std::vector<double> ell_;
    std::vector<std::vector<double>> Bn_per_component_;
};

std::vector<std::pair<double, double>> combinePoints(const std::vector<double> &x, const std::vector<double> &y);

#endif // HARMONICS_DATA_HANDLER_H
