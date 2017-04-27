#include <AlenkaFile/mat.h>

#include <matio.h>

#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace AlenkaFile;

namespace
{

const int MAX_CHANNELS = 1000*1000;

template<class A, class B>
void convertArray(A* a, B* b, int n)
{
	for (int i = 0; i < n; ++i)
		b[i] = static_cast<B>(a[i]);
}

template<class B>
void decodeArray(void* a, B* b, int code, int n = 1, int offset = 0)
{
#define CASE(a_, b_) case a_: convertArray(reinterpret_cast<b_*>(a) + offset, b, n); break;
	switch (code)
	{
		CASE(MAT_T_INT8, int8_t);
		CASE(MAT_T_UINT8, uint8_t);
		CASE(MAT_T_INT16, int16_t);
		CASE(MAT_T_UINT16, uint16_t);
		CASE(MAT_T_INT32, int32_t);
		CASE(MAT_T_UINT32, uint32_t);
		CASE(MAT_T_SINGLE, float);
		CASE(MAT_T_DOUBLE, double);
		CASE(MAT_T_INT64, int64_t);
		CASE(MAT_T_UINT64, uint64_t);
	default:
		runtime_error("Unsupported type");
		break;
	}
#undef CASE
}

} // namespace

namespace AlenkaFile
{

// How to decode data in Matlab: data = double(d)*diag(mults);

MAT::MAT(const string& filePath, const string& dataVarName, const string& frequencyVarName, const string& multipliersVarName)
	: DataFile(filePath), dataVarName(dataVarName), frequencyVarName(frequencyVarName), multipliersVarName(multipliersVarName)
{
	file = Mat_Open(filePath.c_str(), MAT_ACC_RDONLY);

	if (!file)
		throw runtime_error("Error while opening " + filePath);

	// Read sampling rate.
	matvar_t* Fs = Mat_VarRead(file, frequencyVarName.c_str());

	if (Fs)
	{
		if (Fs->dims[0] <= 0)
			throw runtime_error("Bad MAT file format");

		decodeArray(Fs->data, &samplingFrequency, Fs->data_type);

		Mat_VarFree(Fs);
	}
	else
	{
		cerr << "MAT: " << frequencyVarName << "missing" << endl;

		samplingFrequency = 1000;
	}

	// Read data.
	numberOfChannels = MAX_CHANNELS;
	samplesRecorded = 0;

	if (!readDataVar(dataVarName))
	{
		int i = 0;
		string name;

		while (name = dataVarName + to_string(i++), readDataVar(name));

		assert(0 < sizes.size());
	}

	// Read channel multipliers.
	matvar_t* mults = Mat_VarRead(file, multipliersVarName.c_str());

	if (mults)
	{
		int cols = mults->rank < 2 ? 1 : static_cast<int>(mults->dims[1]);
		if (static_cast<int>(mults->dims[0]*cols) < numberOfChannels)
			throw runtime_error("Bad MAT file format");

		multipliers.resize(numberOfChannels);
		decodeArray(mults->data, multipliers.data(), mults->data_type, numberOfChannels);

		Mat_VarFree(mults);
	}
	else
	{
		multipliers.clear();
	}
}

MAT::~MAT()
{
	for (auto e : data)
		Mat_VarFree(e);
	Mat_Close(file);
}

time_t MAT::getStartDate(int timeZone) const
{
	(void)timeZone;
	return 0;
}

void MAT::save()
{
	DataFile::save();
}

bool MAT::load()
{
	if (DataFile::loadSecondaryFile() == false)
	{
		fillDefaultMontage();
		//loadEvents();
		return false;
	}

	return true;
}

template<typename T>
void MAT::readChannelsFloatDouble(vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample)
{
	assert(firstSample <= lastSample && "Bad parameter order.");

	if (getSamplesRecorded() <= lastSample)
		invalid_argument("MAT: reading out of bounds");

	if (dataChannels.size() < getChannelCount())
		invalid_argument("MAT: too few dataChannels");

	int i = 0;
	uint64_t lastInChunk = 0;

	while (lastInChunk += sizes[i], lastInChunk < firstSample)
		++i;

	uint64_t firstInChunk = lastInChunk - sizes[i];
	--lastInChunk;

	for (uint64_t j = firstSample; j < lastSample;)
	{
		uint64_t last = min(lastSample, lastInChunk);
		int length = static_cast<int>(last - j + 1);
		tmpBuffer.resize(numberOfChannels*length*8);

		int start[2] = {static_cast<int>(j - firstInChunk), 0};
		int stride[2] = {1, 1};
		int edge[2] = {length, numberOfChannels};

		int err = Mat_VarReadData(file, data[i], tmpBuffer.data(), start, stride, edge);
		assert(err == 0);

		for (int k = 0; k < numberOfChannels; ++k)
		{
			decodeArray(tmpBuffer.data(), dataChannels[k], data[i]->data_type, length, k*length);

			if (!multipliers.empty())
			{
				T multi = static_cast<T>(multipliers[k]);

				for (int l = 0; l < length; ++l)
					dataChannels[k][l] *= multi;
			}
		}

		for (auto& e : dataChannels)
			e += length;
		firstInChunk += sizes[i];
		lastInChunk += sizes[i];
		j += length;
		++i;
	}
}

void MAT::fillDefaultMontage()
{
	getDataModel()->montageTable()->insertRows(0);

	assert(getChannelCount() > 0);

	AbstractTrackTable* defaultTracks = getDataModel()->montageTable()->trackTable(0);
	defaultTracks->insertRows(0, getChannelCount());
}

bool MAT::readDataVar(const string& varName)
{
	matvar_t* var = Mat_VarReadInfo(file, varName.c_str());

	if (!var)
		return false;

	data.push_back(var);

	if (var->rank != 2)
		throw runtime_error("Bad MAT file format");

	sizes.push_back(static_cast<int>(var->dims[0]));
	samplesRecorded += var->dims[0];

	if (numberOfChannels == MAX_CHANNELS)
	{
		numberOfChannels = static_cast<int>(var->dims[1]);

		if (MAX_CHANNELS < numberOfChannels)
			throw runtime_error("Too many channes in " + varName + ". You probably saved the data with channels in rows by mistake.");
	}

	if (numberOfChannels != static_cast<int>(var->dims[1]))
		throw runtime_error("All data variables must have the same number of channels/columns");

	return true;
}

} // namespace AlenkaFile
