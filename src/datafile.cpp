#include "datafile.h"

#include "pugixml.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <type_traits>

using namespace std;
using namespace pugi;

namespace
{

template<typename T>
void fillWithZeroes(vector<T*>& dataChannels, uint64_t n)
{
	for (auto& e : dataChannels)
		for (uint64_t i = 0; i < n; i++)
		{
			*e = 0;
			e++;
		}
}

template<typename T>
void readSignalFloatDouble(DataFile* file, T* data, int64_t firstSample, int64_t lastSample)
{
	if (lastSample < firstSample)
		throw invalid_argument("'lastSample' must be greater than or equal to 'firstSample'.");

	vector<T*> dataChannels(file->getChannelCount());

	int64_t len = lastSample - firstSample + 1;
	for (unsigned int i = 0; i < file->getChannelCount(); i++)
	{
		dataChannels[i] = data + i*len;
	}

	if (firstSample < 0)
	{
		fillWithZeroes(dataChannels, -firstSample);
		firstSample = 0;
	}

	int64_t lastInFile = min<int64_t>(file->getSamplesRecorded() - 1, lastSample);

	file->readSignalFromFile(dataChannels, firstSample, lastInFile);

	if (lastInFile < lastSample)
	{
		for (auto& e : dataChannels)
			e += lastInFile - firstSample + 1;

		fillWithZeroes(dataChannels, lastSample - lastInFile);

		assert(dataChannels.front() == data + len && "Is the first channel pointer pointing just past the first channel");
	}
}

} // namespace

DataFile::DataFile(const string& filePath) : filePath(filePath)
{
}

DataFile::~DataFile()
{
}

void DataFile::save(xml_document* const infoFile)
{
	string fp = filePath + ".info";
	bool res = infoFile->save_file(fp.c_str());
	assert(res && "Assure the .info file was written successfully."); (void)res;
}

bool DataFile::load(xml_document* infoFile)
{
	string fp = filePath + ".info";
	xml_parse_result res = infoFile->load_file(fp.c_str());

	return res;
}

void DataFile::readSignal(float* data, int64_t firstSample, int64_t lastSample)
{
	readSignalFloatDouble(this, data, firstSample, lastSample);
}

void DataFile::readSignal(double* data, int64_t firstSample, int64_t lastSample)
{
	readSignalFloatDouble(this, data, firstSample, lastSample);
}
