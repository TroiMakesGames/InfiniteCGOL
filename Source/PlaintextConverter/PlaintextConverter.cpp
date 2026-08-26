#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::string inputFilename;

    std::cout << "Enter input .txt filename: ";
    std::getline(std::cin, inputFilename);

    std::ifstream inputFile(inputFilename);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file: " << inputFilename << "\n";
        return 1;
    }

    // Create output filename
    std::string outputFilename = inputFilename;

    size_t dot = outputFilename.find_last_of('.');
    if (dot != std::string::npos) {
        outputFilename = outputFilename.substr(0, dot);
    }

    outputFilename += "_converted.txt";

    std::ofstream outputFile(outputFilename);

    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not create output file.\n";
        return 1;
    }

    std::string line;
    int y = 0;

    while (std::getline(inputFile, line)) {

        // Ignore LifeWiki comments / metadata
        if (!line.empty() && line[0] == '!') {
            continue;
        }

        // Ignore completely empty lines
        if (line.empty()) {
            continue;
        }

        for (int x = 0; x < static_cast<int>(line.length()); x++) {

            if (line[x] == 'O') {
                outputFile << "lvcl " << x << " " << y << "\n";
            }
        }

        y++;
    }

    inputFile.close();
    outputFile.close();

    std::cout << "Conversion complete!\n";
    std::cout << "Output file: " << outputFilename << "\n";

    return 0;
}