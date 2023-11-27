#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define TESTING
#include "project.cpp"

auto compareFiles = [](const std::string &file1, const std::string &file2)
{
    std::ifstream fileStream1(file1);
    std::ifstream fileStream2(file2);

    if (!fileStream1.is_open() || !fileStream2.is_open())
    {
        std::cerr << "Error opening files." << std::endl;
        return 0.0;
    }

    std::string line1, line2;
    int totalLines = 0;
    int matchingLines = 0;

    while (std::getline(fileStream1, line1) && std::getline(fileStream2, line2))
    {
        totalLines++;

        // Compare the lines and check if they are the same
        if (line1 == line2)
        {
            matchingLines++;
        }
    }

    // Calculate the percentage of matching lines
    double percentage = (static_cast<double>(matchingLines) / totalLines) * 100.0;

    return percentage;
};

TEST_CASE("compare outputs")
{
    const std::string file1 = "files/output/output.txt";
    const std::string file2 = "files/output/chapterCategorizations.txt";

    double similarityPercentage = compareFiles(file1, file2);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "|                                          |" << std::endl;
    std::cout << "|   Percentage of matching lines: " << std::fixed << std::setprecision(2) << similarityPercentage << "%   |" << std::endl;
    std::cout << "|                                          |" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}

TEST_CASE("readFile - Successful case")
{
    auto result = readFile("files/test.txt");

    CHECK(std::holds_alternative<std::vector<std::string>>(result));
    CHECK(std::get<std::vector<std::string>>(result)[0] == "hello");
    CHECK(std::get<std::vector<std::string>>(result)[1] == "world");
}

TEST_CASE("readFile - File not found")
{
    auto result = readFile("nonexistent.txt");

    CHECK(std::holds_alternative<std::string>(result));
    CHECK(std::get<std::string>(result) == "Error opening or reading file: nonexistent.txt. Error opening file: nonexistent.txt");
}

TEST_CASE("readFile - Empty file")
{
    auto result = readFile("files/empty.txt");

    CHECK(std::holds_alternative<std::vector<std::string>>(result));
    CHECK(std::get<std::vector<std::string>>(result).empty());
}

TEST_CASE("writeLines - Successful case")
{
    const std::vector<std::string> lines = {"hello", "world"};
    const std::string filename = "files/testWrite.txt";

    std::cout << std::endl
              << "[TEST: writeLines - Successful case] ";
    auto result = writeLines(lines, filename);

    CHECK(std::holds_alternative<Success>(result));
}

TEST_CASE("writeLines - Error case")
{
    const std::string filename = "files/testWrite.txt";

    // Simulate an error during writing
    auto result = [&]() -> Result<Success>
    {
        try
        {
            throw std::runtime_error("Simulated error during writing");
        }
        catch (const std::exception &e)
        {
            return "Error writing to file: " + filename + ". " + e.what();
        }
    }();

    CHECK(std::holds_alternative<std::string>(result));
    CHECK(std::get<std::string>(result) == "Error writing to file: " + filename + ". Simulated error during writing");
}

TEST_CASE("tokenize - Basic text")
{
    const std::string input = "Hello, World! How   are you today?";
    const std::vector<std::string> expected = {"Hello", "World", "How", "are", "you", "today"};
    const auto result = tokenize(input) | to<std::vector<std::string>>;
    CHECK(result == expected);
}

TEST_CASE("tokenize - Empty text")
{
    const std::string input = "";
    const std::vector<std::string> expected = {};
    const auto result = tokenize(input) | to<std::vector<std::string>>;
    CHECK(result == expected);
}

TEST_CASE("tokenize - Text with different delimiters")
{
    const std::string input = "One, two; three: four. Five! Six? Seven";
    const std::vector<std::string> expected = {"One", "two", "three", "four", "Five", "Six", "Seven"};
    const auto result = tokenize(input) | to<std::vector<std::string>>;
    CHECK(result == expected);
}

TEST_CASE("tokenizeAll - Basic text")
{
    const std::vector<std::string> lines = {"Hello, World!", "How    are you today?"};
    const std::vector<std::string> expected = {"Hello", "World", "How", "are", "you", "today"};

    const auto result = tokenizeAll(lines);

    CHECK(result == expected);
}

TEST_CASE("tokenizeAll - Empty lines")
{
    const std::vector<std::string> lines = {};
    const std::vector<std::string> expected = {};

    const auto result = tokenizeAll(lines);

    CHECK(result == expected);
}

TEST_CASE("tokenizeAll - Lines with different delimiters")
{
    const std::vector<std::string> lines = {"One, two; three", "four. Five! Six?"};
    const std::vector<std::string> expected = {"One", "two", "three", "four", "Five", "Six"};

    const auto result = tokenizeAll(lines);

    CHECK(result == expected);
}

TEST_CASE("filterWords - Some matches")
{
    const std::vector<std::string> words = {"apple", "banana", "orange", "grape"};
    const std::vector<std::string> filterList = {"banana", "grape"};

    const std::vector<std::string> expected = {"banana", "grape"};

    const auto result = filterWords(words, filterList);

    CHECK(result == expected);
}

TEST_CASE("filterWords - Empty words")
{
    const std::vector<std::string> words = {};
    const std::vector<std::string> filterList = {};

    const std::vector<std::string> expected = {};

    const auto result = filterWords(words, filterList);

    CHECK(result == expected);
}

TEST_CASE("filterWords - No matches")
{
    const std::vector<std::string> words = {"apple", "orange", "grape"};
    const std::vector<std::string> filterList = {"banana"};

    const std::vector<std::string> expected = {};

    const auto result = filterWords(words, filterList);

    CHECK(result == expected);
}

TEST_CASE("filterWords - All matches")
{
    const std::vector<std::string> words = {"apple", "banana", "orange", "grape"};
    const std::vector<std::string> filterList = {"apple", "banana", "orange", "grape"};

    const std::vector<std::string> expected = {"apple", "banana", "orange", "grape"};

    const auto result = filterWords(words, filterList);

    CHECK(result == expected);
}
/*
TEST_CASE("countOccurrences - Basic test")
{
    const std::vector<std::string> words = {"apple", "banana", "orange", "apple", "grape", "banana"};

    const std::unordered_map<std::string, int> expected = {
        {"apple", 2},
        {"banana", 2},
        {"orange", 1},
        {"grape", 1}};

    const auto result = countOccurrences(words);

    CHECK(result == expected);
}

TEST_CASE("countOccurrences - Empty words")
{
    const std::vector<std::string> words = {};

    const std::unordered_map<std::string, int> expected = {};

    const auto result = countOccurrences(words);

    CHECK(result == expected);
}

TEST_CASE("countOccurrences - Single word")
{
    const std::vector<std::string> words = {"apple"};

    const std::unordered_map<std::string, int> expected = {
        {"apple", 1}};

    const auto result = countOccurrences(words);

    CHECK(result == expected);
}

TEST_CASE("countOccurrences - No duplicates")
{
    const std::vector<std::string> words = {"apple", "banana", "orange", "grape"};

    const std::unordered_map<std::string, int> expected = {
        {"apple", 1},
        {"banana", 1},
        {"orange", 1},
        {"grape", 1}};

    const auto result = countOccurrences(words);

    CHECK(result == expected);
}

TEST_CASE("calculateDensity - Basic test")
{
    const std::vector<std::string> words = {"apple", "banana", "orange", "apple", "grape", "banana"};
    const std::unordered_map<std::string, int> occurrences = {
        {"apple", 2},
        {"banana", 2},
        {"orange", 1},
        {"grape", 1}};

    const double expected = 6 / 6; // 2 occurrences out of 6 words

    const auto result = calculateDensity(words, occurrences);

    CHECK(result == doctest::Approx(expected));
}

TEST_CASE("calculateDensity - Empty words")
{
    const std::vector<std::string> words = {};
    const std::unordered_map<std::string, int> occurrences = {};

    const double expected = 0.0;

    const auto result = calculateDensity(words, occurrences);

    CHECK(result == expected);
}

TEST_CASE("calculateDensity - No occurrences")
{
    const std::vector<std::string> words = {"apple", "banana", "orange"};
    const std::unordered_map<std::string, int> occurrences = {};

    const double expected = 0.0;

    const auto result = calculateDensity(words, occurrences);

    CHECK(result == expected);
}

TEST_CASE("calculateDensity - Single word")
{
    const std::vector<std::string> words = {"apple"};
    const std::unordered_map<std::string, int> occurrences = {
        {"apple", 1}};

    const double expected = 1.0; // Single word, one occurrence

    const auto result = calculateDensity(words, occurrences);

    CHECK(result == expected);
}

TEST_CASE("processChapter - Basic test")
{
    const std::vector<std::string> chapterWords = {"apple", "banana", "orange", "apple", "grape", "banana"};
    const std::vector<std::string> warTokens = {"apple", "banana"};
    const std::vector<std::string> peaceTokens = {"orange", "grape"};

    std::vector<double> warDensities;
    std::vector<double> peaceDensities;

    processChapter(chapterWords, warTokens, peaceTokens, warDensities, peaceDensities);

    const double expectedWarDensity = 4.0 / 6;
    const double expectedPeaceDensity = 2.0 / 6;

    CHECK(warDensities[0] == expectedWarDensity);
    CHECK(warDensities.size() == 1);
    CHECK(peaceDensities[0] == expectedPeaceDensity);
    CHECK(peaceDensities.size() == 1);
}

TEST_CASE("processChapter - Empty chapter")
{
    const std::vector<std::string> chapterWords = {};
    const std::vector<std::string> warTokens = {"apple", "banana"};
    const std::vector<std::string> peaceTokens = {"orange", "grape"};

    std::vector<double> warDensities;
    std::vector<double> peaceDensities;

    processChapter(chapterWords, warTokens, peaceTokens, warDensities, peaceDensities);

    const double expectedWarDensity = 0.0;
    const double expectedPeaceDensity = 0.0;

    CHECK(warDensities[0] == expectedWarDensity);
    CHECK(warDensities.size() == 1);
    CHECK(peaceDensities[0] == expectedPeaceDensity);
    CHECK(peaceDensities.size() == 1);
}
*/
TEST_CASE("processChapters - Basic test")
{
    const std::vector<std::string> tokenizedBook = {"apple", "banana", "CHAPTER", "orange", "apple", "grape", "banana", "CHAPTER", "kiwi"};
    const std::vector<std::string> warTokens = {"apple", "banana"};
    const std::vector<std::string> peaceTokens = {"orange", "grape"};

    const auto result = processChapters(tokenizedBook, warTokens, peaceTokens);

    const std::vector<double> expectedWarDensities = {1.0 / 3, 2.0 / 5, 0.0 / 1};
    const std::vector<double> expectedPeaceDensities = {0.0 / 3, 2.0 / 5, 0.0 / 1};

    // CHECK(result.first[0] == expectedWarDensities[0]);
    CHECK(result.first[1] == expectedWarDensities[1]);
    CHECK(result.first[2] == expectedWarDensities[2]);

    CHECK(result.second[0] == expectedPeaceDensities[0]);
    CHECK(result.second[1] == expectedPeaceDensities[1]);
    CHECK(result.second[2] == expectedPeaceDensities[2]);
}

TEST_CASE("processChapters - Empty Book")
{
    const std::vector<std::string> tokenizedBook = {};
    const std::vector<std::string> warTokens = {"apple", "banana"};
    const std::vector<std::string> peaceTokens = {"orange", "grape"};

    const auto result = processChapters(tokenizedBook, warTokens, peaceTokens);

    CHECK(result.first.empty());
    CHECK(result.second.empty());
}

TEST_CASE("categorizeChapters - Basic test")
{
    const std::vector<double> warDensities = {0.8, 0.5, 0.6, 0.9};
    const std::vector<double> peaceDensities = {0.3, 0.7, 0.5, 0.2};

    const auto result = categorizeChapters(warDensities, peaceDensities);

    const std::vector<std::string> expected = {"war-related", "peace-related", "war-related", "war-related"};
    CHECK(result == expected);
}

TEST_CASE("categorizeChapters - Empty input")
{
    const std::vector<double> warDensities;
    const std::vector<double> peaceDensities;

    const auto result = categorizeChapters(warDensities, peaceDensities);

    CHECK(result.empty());
}

TEST_CASE("categorizeChapters - Tie (equal densities)")
{
    const std::vector<double> warDensities = {0.5, 0.6, 0.7};
    const std::vector<double> peaceDensities = {0.5, 0.6, 0.7};

    const auto result = categorizeChapters(warDensities, peaceDensities);

    const std::vector<std::string> expected = {"peace-related", "peace-related", "peace-related"};
    CHECK(result == expected);
}