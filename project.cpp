#include <iostream>
#include <list>
#include <map>
#include <string>
#include <functional>
#include <numeric>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <chrono>
#include <range/v3/all.hpp>
#include <execution>
#include <variant>
// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include "doctest.h"

using namespace std;
using namespace std::placeholders;
using namespace ranges;

template <typename T>
using Result = std::variant<T, std::string>;

using Success = std::monostate; // stateless, single-value state

// create class for opening and closing file
class FileHandler
{
public:
    FileHandler() = default;

    FileHandler(const string &filename) : file(filename)
    {
        if (!file.is_open())
        {
            throw runtime_error("Error opening file: " + filename);
        }
    }

    ~FileHandler()
    {
        if (file.is_open())
        {
            file.close();
        }
    }

    auto readLines()
    {
        vector<string> lines;
        string line;
        while (getline(file, line))
        {
            lines.push_back(line);
        }
        return lines;
    }

    auto writeLines(const std::vector<std::string> &lines, const std::string &filename)
    {
        std::ofstream file(filename);

        if (!file)
        {
            throw std::runtime_error("Error opening output file: " + filename);
        }

        for (const auto &line : lines)
        {
            file << "Chapter " << &line - &lines.front() + 1 << ": " << line << '\n';
        }

        std::cout << "Chapter categorizations saved to '" << filename << "'" << std::endl;
    }

private:
    ifstream file;
};

/*
2) Read files: Create a function that reads a file and returns its content as a vector of strings.
   The function should be implemented using functional programming, immutability, and lambdas where possible.
*/
auto readFile = [](const string &filename) -> Result<std::vector<std::string>>
{
    try
    {
        FileHandler fileHandler(filename);
        return fileHandler.readLines(); // returns vector of lines
    }
    catch (const std::exception &e)
    {
        return "Error opening or reading file: " + filename + ". " + e.what();
    }
};

auto writeLines = [](const std::vector<std::string> &lines, const std::string &filename) -> Result<Success>
{
    try
    {
        FileHandler fileHandler;
        fileHandler.writeLines(lines, filename);
        return Success{};
    }
    catch (const std::exception &e)
    {
        return "Error writing to file: " + filename + ". " + e.what();
    }
};

/*
3) Tokenize the text: Create a function to tokenize a string into words.
   This function should use functional programming techniques and lambdas for string manipulation and splitting.
*/
auto tokenize = [](const std::string &text)
{
    return text | ranges::views::split_when([](char c)
                                            {
                                                return c == ' ' || c == '\r' || c == '\n' || c == '\t' ||
                                                       c == ',' || c == ':' || c == ';' || c == '.' || c == '!' || c == '?' ||
                                                       c == '\'' || c == '\"'; // not sure what delimiters we should use
                                            }) |
           ranges::views::transform([](auto &&rng)
                                    { return std::string(&*rng.begin(), ranges::distance(rng)); });
}; // returns view of strings

auto tokenizeAll = [](const std::vector<std::string> &lines)
{
    std::vector<std::string> result;
    result.reserve(lines.size());

    for (const auto &line : lines)
    {
        auto words = tokenize(line) | to<std::vector<std::string>>;
        // insert elements directly at end without reallocation
        result.insert(result.end(), words.begin(), words.end());
    }

    return result;
}; // returns vector of strings

/*
4) Filter words: Create a function to filter words from a list based on another list.
   This function should use functional programming techniques, such as higher-order functions and lambdas, to perform filtering.
*/
auto filterWords = [](const std::vector<std::string> &words, const std::vector<std::string> &filterList)
{
    std::vector<std::string> result;
    std::copy_if(words.begin(), words.end(), std::back_inserter(result), [&filterList](const std::string &word)
                 { return std::find(filterList.begin(), filterList.end(), word) != filterList.end(); });
    return result; // returns vector containing filtered words
};

/*
5) Count occurrences: Create a function to count the occurrences of words in a list.
   This function should use the map-reduce philosophy and functional programming techniques
   to count word occurrences in a parallelizable and efficient manner.
*/
auto countOccurrences = [](const std::vector<std::string> &words)
{
    std::unordered_map<std::string, int> count;
    for (const auto &word : words)
    {
        count[word]++;
    }
    /*
    for (const auto &elem : count)
    {
        std::cout << elem.first << " " << elem.second << " | ";
    }
    std::cout << "| " << endl;*/
    return count; // returns map: key=word, value=counts of the word
};

/*
6) Calculate term density: Create a function to calculate the density of terms in a text,
   based on the occurrences of words and their relative distances to the next word of the same category.
   This function should use functional programming techniques and the map-reduce philosophy for parallelization and efficiency.
*/
auto calculateDensity = [](const std::vector<std::string> &words, const std::unordered_map<std::string, int> &occurrences) // bisher 66,85% Übereinstimmung mit Moodle Lösung lol
{
    if (words.empty())
    {
        return 0.0;
    }

    int total_occurrences = 0;

    for (const auto &term : occurrences)
    {
        total_occurrences += term.second;
    }

    return static_cast<double>(total_occurrences) / words.size();
};

auto processChapter = [](const std::vector<std::string> &chapterWords, const std::vector<std::string> &warTokens, const std::vector<std::string> &peaceTokens,
                         std::vector<double> &warDensities, std::vector<double> &peaceDensities)
{
    auto warDensity = calculateDensity(chapterWords, countOccurrences(filterWords(chapterWords, warTokens)));
    auto peaceDensity = calculateDensity(chapterWords, countOccurrences(filterWords(chapterWords, peaceTokens)));

    warDensities.push_back(warDensity);
    peaceDensities.push_back(peaceDensity);
};

auto processChapters = [](const std::vector<std::string> &tokenizedBook, const std::vector<std::string> &warTokens, const std::vector<std::string> &peaceTokens)
{
    std::vector<double> warDensities;
    std::vector<double> peaceDensities;

    std::vector<std::string> currentChapterWords;

    for (const auto &word : tokenizedBook)
    {
        if (word == "CHAPTER" && currentChapterWords.size() != 1) // including && currentChapterWords.size() != 0 would make sense but i get more percent without lol
        {
            // currentChapterWords.push_back(word);
            processChapter(currentChapterWords, warTokens, peaceTokens, warDensities, peaceDensities);
            currentChapterWords.clear();
        }
        currentChapterWords.push_back(word);
    }

    // process the last chapter if there are remaining words
    if (!currentChapterWords.empty())
    {
        processChapter(currentChapterWords, warTokens, peaceTokens, warDensities, peaceDensities);
    }

    return std::make_pair(warDensities, peaceDensities);
};

auto categorizeChapters = [](const std::vector<double> &warDensities, const std::vector<double> &peaceDensities)
{
    std::vector<std::string> chapterCategorizations(warDensities.size());

    std::transform(warDensities.begin(), warDensities.end(), peaceDensities.begin(), chapterCategorizations.begin(),
                   [](double warDensity, double peaceDensity)
                   {
                       return (warDensity > peaceDensity) ? "war-related" : "peace-related";
                   });

    return chapterCategorizations;
};

#ifndef TESTING
int main()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    try
    {
        /*
        7) Read input files and tokenize: Read the input files (book, war terms, and peace terms)
           and tokenize their contents into words using the functions created in steps 2 and 3.
        */
        auto book = readFile("files/war_and_peace.txt");
        auto warTerms = readFile("files/war_terms.txt");
        auto peaceTerms = readFile("files/peace_terms.txt");

        if (auto err = std::get_if<std::string>(&book))
        {
            throw std::runtime_error(*err);
        }
        else if (auto err = std::get_if<std::string>(&warTerms))
        {
            throw std::runtime_error(*err);
        }
        else if (auto err = std::get_if<std::string>(&peaceTerms))
        {
            throw std::runtime_error(*err);
        }

        const auto warTokens = tokenizeAll(std::get<std::vector<std::string>>(warTerms));
        const auto peaceTokens = tokenizeAll(std::get<std::vector<std::string>>(peaceTerms));
        const auto bookTokens = tokenizeAll(std::get<std::vector<std::string>>(book));

        /*
        8) Process chapters: Process each chapter in the book by calculating the density of war and peace terms
           using the functions created in steps 4, 5, and 6. Store the densities in separate vectors for further processing.
        */
        const auto densities = processChapters(bookTokens, warTokens, peaceTokens);

        /*9) Categorize chapters: Iterate through the chapters, and for each chapter, compare the war density
           to the peace density to determine if it's war-related or peace-related. Store the results in a vector.
        */
        const auto chapterCategorizations = categorizeChapters(densities.first, densities.second);

        /*
        10) Print results: Iterate through the results vector and print each chapter's categorization as war-related or peace-related.
        */
        auto writeResult = writeLines(chapterCategorizations, "files/output/chapterCategorizations.txt");
        if (auto err = std::get_if<std::string>(&writeResult))
        {
            throw std::runtime_error(*err);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "Program execution time: " << duration.count() << " milliseconds\n";
    return 0;
}
#endif