#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <vector>
#include <string>
#include <boost/filesystem.hpp>

void print_harmonic_drive_values(const std::vector<std::pair<int, double>> &harmonic_drive_values);
char getch();
bool askUserToProceed();
double getUserInput(const std::string &prompt, double default_value);
boost::filesystem::path selectJsonFile();
void copyModelWithTimestamp(const boost::filesystem::path &src_path);
void export_vector_to_csv(const std::vector<double>& vector, const std::string& csv_path);
void export_data_to_csv(const std::vector<std::pair<double, double>>& vector, const std::string& csv_path);
void print_vector(const std::vector<double>& data, std::string label);


#endif // INPUT_OUTPUT_H
