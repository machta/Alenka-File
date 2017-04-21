#include <AlenkaFile/mat.h>

#include <matio.h>

#include <cstdint>
#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace AlenkaFile;

namespace
{

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

MAT::MAT(const string& filePath) : DataFile(filePath)
{
	file = Mat_Open(filePath.c_str(), MAT_ACC_RDONLY);
	assert(file && "File  was not successfully opened.");

	matvar_t* Fs = Mat_VarRead(file, "Fs");
	assert(Fs);
	assert(Fs->dims[0] >= 1);
	decodeArray(Fs->data, &samplingFrequency, Fs->data_type);
	Mat_VarFree(Fs);

	int i = 0;
	samplesRecorded = 0;

	while (1)
	{
		string varName = "data" + to_string(i);

		matvar_t* var = Mat_VarReadInfo(file, varName.c_str());
		if (!var)
			break;
		data.push_back(var);

		assert(var->rank == 2);
		sizes.push_back(static_cast<int>(var->dims[0]));
		samplesRecorded += var->dims[0];

		if (i++ == 0)
			numberOfChannels = static_cast<int>(var->dims[1]);

		assert(numberOfChannels == static_cast<int>(var->dims[1]));
		assert(var->dims[1] < 10*1000 && "Too many channels.");
	}

	assert(i > 0);
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
	assert(lastSample < getSamplesRecorded() && "Reading out of bounds.");
	assert(dataChannels.size() == getChannelCount() && "Make sure dataChannels has the same number of channels as the file.");

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
			decodeArray(tmpBuffer.data(), dataChannels[k], data[i]->data_type, length, k*length);

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

//	for (int i = 0; i < defaultTracks->rowCount(); ++i)
//	{
//		Track t = defaultTracks->row(i);
//		t.label = "Track " + ;
//		defaultTracks->row(i, t);
//	}
}

} // namespace AlenkaFile
