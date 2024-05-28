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


#endif // INPUT_OUTPUT_H
