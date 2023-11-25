/*TODO: 
    genereal optimizations
    error handling/monads
    better density calculation
    maybe use ranges library more
    test cases
    run.bat
    powerpoint
    readme
    maybe move some lambdas out of main idk
*/
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <functional>
#include <numeric>
#include <fstream>
#include <iterator>
#include <algorithm>
// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include "doctest.h"
#include <range/v3/all.hpp>

using namespace std;
using namespace std::placeholders;
using namespace ranges;

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

    auto writeLines(const std::vector<std::string>& lines, const std::string& filename)
    {
        std::ofstream file(filename);

        if (file.is_open())
        {
            for (size_t i = 0; i < lines.size(); ++i)
            {
                file << "Chapter " << i + 1 << ": " << lines[i] << '\n';
            }

            std::cout << "Chapter categorizations saved to '" << filename << "'" << std::endl;
        }
        else
        {
            throw std::runtime_error("Error opening output file: " + filename);
        }
    }

private:
    ifstream file;
};

/*
2) Read files: Create a function that reads a file and returns its content as a vector of strings.
   The function should be implemented using functional programming, immutability, and lambdas where possible.
*/
auto readFile = [](const string &filename)
{
    FileHandler fileHandler(filename);
    return fileHandler.readLines(); // returns vector of lines
};

/*
3) Tokenize the text: Create a function to tokenize a string into words.
   This function should use functional programming techniques and lambdas for string manipulation and splitting.
*/
auto tokenize = [](const std::string &text)
{
    return text | ranges::views::split(' ') | ranges::views::transform([](auto &&rng)
                                                                       { return std::string(&*rng.begin(), ranges::distance(rng)); });
}; // returns view
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
    std::map<std::string, int> count;
    for (const auto &word : words)
    {
        count[word]++;
    }
    return count; // returns map: key=word, value=counts of the word
};

/*
6) Calculate term density: Create a function to calculate the density of terms in a text,
   based on the occurrences of words and their relative distances to the next word of the same category.
   This function should use functional programming techniques and the map-reduce philosophy for parallelization and efficiency.
*/
auto calculateDensity = [](const std::vector<std::string> &words, const std::map<std::string, int> &occurrences) //bisher 63,84% Übereinstimmung mit Moodle Lösung lol
{
    if (words.empty())
      return 0.0;

    int total_occurrences = 0;
    //int total_distance = 0;

    for (const auto &word : words)
    {
        auto it = occurrences.find(word);
        if (it != occurrences.end())
        {
            total_occurrences += it->second;
            //total_distance++;
        }
    }
    
    //return static_cast<double>(total_occurrences) / total_distance;
    

    return static_cast<double>(total_occurrences) / words.size();
};


int main()
{
    try
    {
        /*
        7) Read input files and tokenize: Read the input files (book, war terms, and peace terms)
           and tokenize their contents into words using the functions created in steps 2 and 3.
        */
        auto book = readFile("files/war_and_peace.txt");
        auto warTerms = readFile("files/war_terms.txt");
        auto peaceTerms = readFile("files/peace_terms.txt");

        auto tokenizeAll = [&](const std::vector<std::string> &lines)
        {
            std::cout << "tokenizing file..." << endl;

            // Calculate the total number of words to reserve space
            size_t totalWords = std::accumulate(lines.begin(), lines.end(), 0, [](size_t acc, const std::string &line) {
                return acc + std::distance(tokenize(line).begin(), tokenize(line).end());
            });

            std::vector<std::string> result;
            result.reserve(totalWords);

            for (const auto &line : lines)
            {
                auto words = tokenize(line) | to<std::vector<std::string>>;
                std::cout << "inserting..." << std::endl;

                // Insert elements directly at the end without reallocation
                result.insert(result.end(), words.begin(), words.end());
            }

            return result;
        }; //returns vector of strings

        const auto warTokens = tokenizeAll(warTerms);
        const auto peaceTokens = tokenizeAll(peaceTerms);
        const auto bookTokens = tokenizeAll(book);
        

        //-----TODO------
        /*
        8) Process chapters: Process each chapter in the book by calculating the density of war and peace terms
           using the functions created in steps 4, 5, and 6. Store the densities in separate vectors for further processing.
        */
        //1 vector for war density (each element  = war density for each chapter) ?
        //1 vector for peace density (each element  = peace density for each chapter) ?

        auto processChapters = [&](const std::vector<std::string> &tokenizedBook, const std::vector<std::string> &warTokens, const std::vector<std::string> &peaceTokens)
        {
            std::cout << "processing chapters..." << endl;
            std::vector<double> warDensities;
            std::vector<double> peaceDensities;

            std::vector<std::string> currentChapterWords;

            for (const auto &word : tokenizedBook)
            {
                if (word == "CHAPTER" && !currentChapterWords.empty())
                {
                    // Calculate density for the current chapter and store in vectors
                    auto warDensity = calculateDensity(currentChapterWords, countOccurrences(filterWords(currentChapterWords, warTokens)));
                    auto peaceDensity = calculateDensity(currentChapterWords, countOccurrences(filterWords(currentChapterWords, peaceTokens)));

                    warDensities.push_back(warDensity);
                    peaceDensities.push_back(peaceDensity);

                    // Clear the words for the next chapter
                    currentChapterWords.clear();
                }
                else
                {
                    // collect words within the current chapter
                    currentChapterWords.push_back(word);
                }
            }

            // Process the last chapter if there are remaining words
            if (!currentChapterWords.empty())
            {
                auto warDensity = calculateDensity(currentChapterWords, countOccurrences(filterWords(currentChapterWords, warTerms)));
                auto peaceDensity = calculateDensity(currentChapterWords, countOccurrences(filterWords(currentChapterWords, peaceTerms)));

                warDensities.push_back(warDensity);
                peaceDensities.push_back(peaceDensity);
            }

            return std::make_pair(warDensities, peaceDensities);
        };

        const auto densities = processChapters(bookTokens, warTokens, peaceTokens);

        /*9) Categorize chapters: Iterate through the chapters, and for each chapter, compare the war density
           to the peace density to determine if it's war-related or peace-related. Store the results in a vector.
        */
        auto categorizeChapters = [&](const std::vector<double> &warDensities, const std::vector<double> &peaceDensities)
        {
            std::cout << "categorizing chapters..." << endl;
            std::vector<std::string> chapterCategorizations;

            for (size_t i = 0; i < warDensities.size(); ++i)
            {
                std::string categorization = (warDensities[i] > peaceDensities[i]) ? "war-related" : "peace-related";
                chapterCategorizations.push_back(categorization);
            }

            return chapterCategorizations;
        };

       const auto chapterCategorizations = categorizeChapters(densities.first, densities.second);
        /*
        10) Print results: Iterate through the results vector and print each chapter's categorization as war-related or peace-related.
        */
        for (size_t i = 0; i < chapterCategorizations.size(); ++i) {
            std::cout << "Chapter " << i + 1 << ": " << chapterCategorizations[i] << '\n';
        }

        FileHandler fileHandler;
        fileHandler.writeLines(chapterCategorizations, "files/output/chapterCategorizations.txt");
        
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}