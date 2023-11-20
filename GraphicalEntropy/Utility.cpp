#include "Utility.h"


double CalculateEntropy(const std::vector<unsigned char>& data)
{
    std::vector<unsigned int> counters(256, 0);
    size_t fileSize = data.size();

    for (size_t i = 0; i < fileSize; ++i) {
        counters[data[i]]++;
    }

    double entropy = 0.0;
    for (size_t i = 0; i < counters.size(); ++i) {
        if (counters[i] != 0) {
            double probability = static_cast<double>(counters[i]) / fileSize;
            entropy -= probability * log2(probability);
        }
    }

    return entropy;
}

bool IsBinary(const std::vector<unsigned char>& data)
{
    for (auto byte : data)
    {
        // Check if the byte is outside the printable ASCII range
        if (byte < 32 || byte > 126)
        {
            return true; // Binary file
        }
    }
    return false; // Text file
}

std::wstring GetFileFormat(const std::wstring& filePath)
{
    // Extract the file extension
    size_t dotPosition = filePath.find_last_of(L".");
    if (dotPosition != std::wstring::npos) {
        return filePath.substr(dotPosition + 1);
    }
    return L"Unknown";
}