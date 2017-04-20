#include <AlenkaFile/mat.h>

#include <matio.h>
#include <cstdint>
#include <stdexcept>

using namespace std;
using namespace AlenkaFile;

namespace
{

const int READ_CHUNK = 1000;

template<class A, class B>
void convertArray(A* a, B* b, int n)
{
	for (int i = 0; i < n; ++i)
		b[i] = static_cast<B>(a[i]);
}

template<class B>
void decodeArray(void* a, B* b, int code, int n = 1)
{
#define CASE(a_, b_) case a_: convertArray(reinterpret_cast<b_*>(a), b, n); break;
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

	data = Mat_VarReadInfo(file, "data");
	assert(data->rank == 2);
	samplesRecorded = data->dims[0];
	assert(data->dims[1] < 10*1000 && "Too many channels.");
	numberOfChannels = static_cast<int>(data->dims[1]);

	readChunkBuffer = malloc(READ_CHUNK*sizeof(double));
}

MAT::~MAT()
{
	free(readChunkBuffer);
	Mat_VarFree(data);
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
	return DataFile::load();
}

template<typename T>
void MAT::readChannelsFloatDouble(vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample)
{
	for (unsigned int i = 0; i < dataChannels.size(); ++i)
	{
		for (uint64_t j = firstSample; j < lastSample; j += READ_CHUNK)
		{
			int toRead = min(READ_CHUNK, static_cast<int>(lastSample - j + 1));

			int start[2] = {static_cast<int>(j), static_cast<int>(i)};
			int stride[2] = {1, 1};
			int edge[2] = {toRead, 1};

			int err = Mat_VarReadData(file, data, readChunkBuffer, start, stride, edge);
			assert(err == 0);

			decodeArray(readChunkBuffer, dataChannels[i], data->data_type, toRead);

			dataChannels[i] += toRead;
		}
	}
}

} // namespace AlenkaFile
