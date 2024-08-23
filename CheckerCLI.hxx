/// \file CheckerCLI.hxx
/// \ingroup NTuple ROOT7
/// \author Ida Caspary <ida.caspary@gmail.com>
/// \date 2024-10-14
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback
/// is welcome!

/*************************************************************************
 * Copyright (C) 1995-2023, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef CHECKERCLI_HXX
#define CHECKERCLI_HXX

#include "Checker.hxx"
#include <vector>
#include <string>

namespace Checker {
    struct CheckerConfig {
        std::string fTTreeFile;
        std::string fRNTupleFile;
        std::string fTTreeName;
        std::string fRNTupleName;
        bool fShouldRun = false;
    };

    CheckerConfig ParseArgs(int argc, char* argv[]);
    void RunChecker(const CheckerConfig& config);

    class CheckerCLI {
    public:

        /**
         * @brief Sets the verbosity level of the CheckerCLI.
         *
         * This function sets the internal `fVerbose` flag, which controls whether
         * detailed information should be printed during the comparison process.
         *
         * @param verbose Boolean flag to enable or disable verbose output.
         */
        void SetVerbosity(bool verbose);

        /**
         * @brief Compares two datasets based on the provided configuration.
         *
         * This function orchestrates the comparison of TTree and RNTuple data files
         * based on the given configuration. It compares entry counts, field counts,
         * field names, and field types. Additionally, it generates histograms to
         * visually compare the data distributions.
         *
         * @param config The configuration object containing file paths and other
         *               comparison parameters.
         * @throws std::runtime_error if any inconsistency is detected during the
         *         comparison process, such as misaligned data vectors or file read errors.
         */
        void Compare(const CheckerConfig& config);

        /**
         * @brief Runs the comparison process if the configuration specifies to do so.
         *
         * This function checks the configuration to determine if the comparison
         * process should be executed by calling `Compare()`. If the `fShouldRun`
         * flag is set in the configuration, the comparison process is triggered.
         *
         * @param config The configuration object containing file paths and other
         *               comparison parameters.
         */
        void RunAll(const CheckerConfig& config);

        /**
         * @brief Prints styled text to the console.
         *
         * This function prints text to the console with specified styles (e.g., colors, bold) and manages
         * line breaks. It can be used for formatting output in a more readable and visually distinct manner.
         *
         * @param text The text to be printed.
         * @param styles A list of styles (colors, bold, etc.) to apply to the text.
         * @param firstLineBreak Whether to insert a line break after the text.
         * @param secondLineBreak Whether to insert an additional line break after the first one.
         */
        void PrintStyled(const std::string& text, const std::initializer_list<std::string>& styles, bool firstLineBreak = true, bool secondLineBreak = false);

        /**
         * @brief Prints styled text with alignment and width.
         *
         * This function prints text to the console with specified styles, alignment, and width.
         *
         * @param text The text to be printed.
         * @param styles A list of styles (colors, bold, etc.) to apply to the text.
         * @param width The width to which the text should be aligned.
         * @param firstLineBreak Whether to insert a line break after the text.
         * @param secondLineBreak Whether to insert an additional line break after the first one.
         */
        void PrintStyled(const std::string& text, const std::initializer_list<std::string>& styles, int width, bool firstLineBreak = true, bool secondLineBreak = false);

        /**
         * @brief Compares and prints the entry counts of the datasets.
         *
         * This function compares the entry counts of the TTree and RNTuple datasets.
         * It prints the results, highlighting any discrepancies. If the verbosity
         * is set to false and there are no discrepancies, it will not print anything.
         *
         * @param entries A pair of integers representing the entry counts of TTree and RNTuple.
         * @return True if there are discrepancies or if verbosity is enabled; otherwise, false.
         */
        bool PrintEntryComparison(const std::pair<int, int>& entries);

        /**
         * @brief Compares and prints the field counts of the datasets.
         *
         * This function compares the field counts of the TTree and RNTuple datasets.
         * It prints the results, highlighting any discrepancies. If the verbosity
         * is set to false and there are no discrepancies, it will not print anything.
         *
         * @param fields A pair of integers representing the field counts of TTree and RNTuple, respectively.
         * @return True if there are discrepancies or if verbosity is enabled; otherwise, false.
         */
        bool PrintFieldComparison(const std::pair<int, int>& fields);

        /**
         * @brief Compares and prints the field names of the datasets.
         *
         * This function compares the field names of the TTree and RNTuple datasets.
         * It prints the results, highlighting any discrepancies. If the verbosity
         * is set to false and there are no discrepancies, it will not print anything.
         *
         * @param fieldNames A vector of pairs of strings representing the field names in TTree and RNTuple, respectively.
         * @return True if there are discrepancies or if verbosity is enabled; otherwise, false.
         */
        bool PrintFieldNameComparison(const std::vector<std::pair<std::string, std::string>>& fieldNames);

        /**
         * @brief Compares and prints the field types of the datasets.
         *
         * This function compares the field types of the TTree and RNTuple datasets.
         * It prints the results, highlighting any discrepancies or non-exact matches.
         * If the verbosity is set to false and there are no discrepancies, it will not
         * print anything.
         *
         * @param fieldTypes A vector of tuples, where each tuple contains the field name, the TTree field type, and the RNTuple field type.
         * @return True if there are discrepancies or if verbosity is enabled; otherwise, false.
         */
        bool PrintFieldTypeComparison(const std::vector<std::tuple<std::string, std::string, std::string>>& fieldTypes);

        /**
         * @brief Prints the contents of different vectors from the TTree dataset.
         *
         * This function prints the contents of integer, float, double, and boolean vectors
         * from the TTree dataset. It formats the output in a structured manner.
         *
         * @param intVector The vector of integers to be printed.
         * @param doubleVector The vector of doubles to be printed.
         * @param floatVector The vector of floats to be printed.
         * @param boolVector The vector of booleans to be printed.
         */
        void PrintVectorFromTTree(const std::vector<int>& intVector, const std::vector<double>& doubleVector, const std::vector<float>& floatVector, const std::vector<bool>& boolVector);

        /**
         * @brief Prints the contents of different vectors from the RNTuple dataset.
         *
         * This function prints the contents of integer, float, double, and boolean vectors
         * from the RNTuple dataset. It formats the output in a structured manner.
         *
         * @param intVector The vector of integers to be printed.
         * @param floatVector The vector of floats to be printed.
         * @param doubleVector The vector of doubles to be printed.
         * @param boolVector The vector of booleans to be printed.
         */
        void PrintVectorFromRNTuple(const std::vector<int>& intVector, const std::vector<float>& floatVector, const std::vector<double>& doubleVector, const std::vector<bool>& boolVector);

        /**
         * @brief Creates and compares histograms for integer data.
         *
         * This is the version for integer data - the same works for float, double, bool.
         *
         * This function creates histograms for integer data from the TTree and RNTuple datasets.
         * It draws the histograms and calculates the chi-square test value to
         * determine if the distributions are statistically similar.
         *
         * @param ttreeVector The vector of integer data from the TTree dataset.
         * @param rntupleVector The vector of integer data from the RNTuple dataset.
         */
        void IntHist_ChiSquareComparison(const std::vector<int>& ttreeVector, const std::vector<int>& rntupleVector);

        /**
         * @brief Generates histograms for the provided TTree dataset fields.
         *
         * This function generates histograms for integer, float, double, and boolean fields from the
         * TTree dataset. It returns a vector of tuples containing the number of entries, the mean,
         * and the standard deviation for each field.
         *
         * @param intData Vector containing integer data from the TTree.
         * @param floatData Vector containing float data from the TTree.
         * @param doubleData Vector containing double data from the TTree.
         * @param boolData Vector containing boolean data from the TTree.
         *
         * @return std::vector<std::tuple<int, double, double>> A vector of tuples where each tuple contains:
         *         - An integer representing the number of entries in the histogram.
         *         - A double representing the mean of the histogram.
         *         - A double representing the standard deviation of the histogram.
         *
         * @note If a data vector is empty, the corresponding histogram statistics (count, mean, stddev) are returned as (0, 0.0, 0.0).
         */
        std::vector<std::tuple<int, double, double>> HistTTree(const std::vector<int>& intData,
            const std::vector<float>& floatData,
            const std::vector<double>& doubleData,
            const std::vector<bool>& boolData);

        /**
         * @brief Generates histograms for the provided RNTuple dataset fields.
         *
         * This function generates histograms for integer, float, double, and boolean fields from the
         * RNTuple dataset and displays them. It also saves the combined histograms as a PNG image.
         * The function returns a vector of tuples containing the statistics (count, mean, stddev) for each field.
         *
         * @param intData Vector containing integer data from the RNTuple.
         * @param floatData Vector containing float data from the RNTuple.
         * @param doubleData Vector containing double data from the RNTuple.
         * @param boolData Vector containing boolean data from the RNTuple.
         *
         * @return std::vector<std::tuple<int, double, double>> A vector of tuples where each tuple contains:
         *         - An integer representing the number of entries in the histogram.
         *         - A double representing the mean of the histogram.
         *         - A double representing the standard deviation of the histogram.
         *
         * @note If a data vector is empty, the corresponding histogram statistics (count, mean, stddev) are returned as (0, 0.0, 0.0).
         */
        std::vector<std::tuple<int, double, double>> HistRNTuple(const std::vector<int>& intData,
            const std::vector<float>& floatData,
            const std::vector<double>& doubleData,
            const std::vector<bool>& boolData);

        /**
         * @brief Compares and prints the histogram statistics for two datasets.
         *
         * This function compares the histogram statistics (count, mean, standard deviation) between two datasets:
         * It prints a formatted comparison table highlighting any mismatches in statistics. It may print them.
         * The function also indicates whether all statistics match between the datasets.
         *
         * @param dataT A vector of tuples containing histogram statistics (count, mean, stddev) for the TTree dataset.
         * @param dataR A vector of tuples containing histogram statistics (count, mean, stddev) for the RNTuple dataset.
         *
         * @note If the vectors do not have the expected sizes, an error message is printed.
         * @note The function is only executed if verbose mode is enabled.
         */
        void HistogramDrawStat(const std::vector<std::tuple<int, double, double>>& dataT,
            const std::vector<std::tuple<int, double, double>>& dataR);

        // Colours for PrintStyled
        static constexpr const char* GREEN = "\033[0;32m";
        static constexpr const char* RED = "\033[0;31m";
        static constexpr const char* YELLOW = "\033[0;33m";
        static constexpr const char* BLUE = "\033[0;34m";
        static constexpr const char* WHITE = "\033[0;37m";
        static constexpr const char* BLACK = "\033[0;30m";

        static constexpr const char* MEDIUM_BLUE = "\033[38;5;75m";
        static constexpr const char* DARKER_BLUE = "\033[38;5;18m";

        static constexpr const char* BG_WHITE = "\033[47m";
        static constexpr const char* BG_RED = "\033[41m";
        static constexpr const char* BG_GREEN = "\033[42m";
        static constexpr const char* BG_YELLOW = "\033[43m";

        static constexpr const char* RESET = "\033[0m";
        static constexpr const char* DEFAULT = "\033[39m";

    private:
        bool fVerbose = false;
    };
} // namespace Checker

#endif // CHECKERCLI_HXX
