#ifndef LINEAR_OPTIMIZER_H
#define LINEAR_OPTIMIZER_H

#include "abstract_optimizer.h"
#include "abstract_objective.hh"

/**
 * @brief Optimizer to optimize the scaling function of harmonic drives linearly.
 *
 * This optimizer optimizes the scaling function of harmonic drives linearly.
 * Derived classes specify the objective by implementing the `getValues` function. The objective is achieved when all (absolute) values returned by `getValues` are below the specified `max_value_`.
 * This optimizer performs several rounds of linear optimizations until the objective is met.
 * In each round, all harmonic drives present in the model (except for the main component) are optimized one by one. The `constant` or `slope` parameter of the scaling function (depending on the `amplitude` of the harmonic drive) is optimized linearly;
 * the optimizer changes the parameter in small steps and calculates the objective value for each step. The optimizer then fits a linear function to the data points and extracts the root of the function as the new parameter value.
 */
class LinearOptimizer : public AbstractOptimizer
{
public:
    /**
     * @brief Construct a new Linear Optimizer object with suppresed user interaction.
     * @param optimized_value_label Label of the type of values optimized.
     * @param harmonic_drive_prefix Prefix for the names of targeted custom CCT harmonics.
     * @param model_handler ModelHandler object to be used for the optimization.
     * @param max_value Maximum absolute value for the optimized values.
     *
     * The constructor initializes the optimizer with the provided parameters and sets up the model for optimization.
     */
    LinearOptimizer(std::string optimized_value_label, std::string harmonic_drive_prefix, CCTools::ModelHandler &model_handler, double max_value);

    /**
     * @brief Construct a new Linear Optimizer object with user interaction.
     * @param optimized_value_label Label of the type of values optimized.
     * @param harmonic_drive_prefix Prefix for the names of targeted custom CCT harmonics.
     *
     * The constructor initializes the optimizer with the provided parameters and sets up the model for optimization.
     * The user is asked to select a model and enter a maximum absolute value for the optimized values from the command line.
     */
    LinearOptimizer(std::string optimized_value_label, std::string harmonic_drive_prefix);

    /**
     * @brief Get the results of the optimization.
     * @return Vector of doubles with the optimized values.
     *
     * This function returns the optimized values after the optimization has finished.
     */
    std::vector<double> &getResults();

    /**
     * @brief Run the optimization algorithm.
     *
     * This function runs the optimization algorithm. The algorithm optimizes the scaling function of the harmonic drives linearly until the objective is met.
     */
    void optimize() override;

    /**
     * @brief Log the results of the optimization.
     *
     * This function logs the results of the optimization.
     */
    void logResults() override;

    virtual ~LinearOptimizer() {};

protected:
    /**
     * @brief Get the drive value and type for a harmonic drive.
     * @param identifier Identifier of the harmonic drive. Format: "B2"
     * @param current_drive_value Reference to a double to store the current drive value.
     * @param drive_type Reference to a HarmonicDriveParameterType to store the drive type.
     * @throws std::logic_error if the scaling function is neither constant nor linear.
     *
     * This function gets the current drive value (`constant` value for constant drives, `slope` for linear ones) and type (constant or linear) for a harmonic drive.
     * The results are stored in the provided references.
     */
    void getDriveValueAndType(const std::string &identifier, double &current_drive_value, CCTools::HarmonicDriveParameterType &drive_type);

    /**
     * @brief Get the values to be optimized.
     * @param harmonics_handler HarmonicsDataHandler object with the results of the harmonics calculation.
     * @return Vector of doubles with the objective values for all harmonics.
     *
     * This function is pure virtual and needs to be implemented in the derived classes.
     * It returns the values to be optimized for the result of a harmonics calculation. Vector must have length 10 to provide objective values for all harmonics.
     */
    virtual std::vector<double> getValues(CCTools::HarmonicsDataHandler &harmonics_handler) = 0;

    /**
     * @brief Fit a linear function to data and extract the root.
     * @param points Vector of pairs of doubles with the data points.
     * @return The root of the linear function.
     *
     * This function fits a linear function to the data points and extracts the root of the function.
     */
    static double fitLinearGetRoot(const std::vector<std::pair<double, double>> &points);

    /**
     * @brief Label of the type of values optimized.
     */
    std::string optimized_value_label_;

    /**
     * @brief Map with the custom CCT harmonic drive values before the optimization.
     */
    CCTools::HarmonicDriveParameterMap initial_drive_values_;

private:

    /**
     * @brief Setup the optimizer.
     *
     * This function performs some model checks and initializes the calculator.
     */
    void setup(CCTools::ModelHandler &model_handler, double max_value, std::string harmonic_drive_prefix);

    /**
     * @brief Get the maximum harmonic value from the user.
     * @return The maximum harmonic value.
     *
     * This function asks the user for the maximum harmonic value in the terminal.
     */
    double getMaxHarmonicValue();

    /**
     * @brief Maximum absolute value for the optimized values.
     */
    double max_value_;

    /**
     * @brief Vector with the current values to be optimized of the harmonics.
     */
    std::vector<double> current_values_;
};

#endif // LINEAR_OPTIMIZER_H
