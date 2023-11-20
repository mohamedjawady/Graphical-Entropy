#ifndef ENTRYPY_H
#define ENTROPY_H

#include <vector>
#include <string>

double CalculateEntropy(const std::vector<unsigned char>& data);
bool IsBinary(const std::vector<unsigned char>& data);
std::wstring GetFileFormat(const std::wstring& filePath);

#endif