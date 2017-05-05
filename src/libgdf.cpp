#include <AlenkaFile/libgdf.h>

#include <GDF/Reader.h>

using namespace std;
using namespace gdf;
using namespace AlenkaFile;

namespace
{

const int MIN_READ_CHUNK = 200;
const int OPT_READ_CHUNK = 2*1000;
const int MAX_READ_CHUNK = 2*1000*1000;

} // namespace

namespace AlenkaFile
{

LibGDF::LibGDF(const string& filePath) : DataFile(filePath)
{
	gdfReader = new Reader();
	gdfReader->open(filePath);

	samplesRecorded = gdfReader->getSignalHeader_readonly(0).get_samples_per_record()*gdfReader->getMainHeader_readonly().get_num_datarecords();
	samplingFrequency = gdfReader->getSignalHeader_readonly(0).get_samplerate();
	numberOfChannels = gdfReader->getMainHeader_readonly().get_num_signals();

	readChunk = gdfReader->getSignalHeader_readonly(0).get_samples_per_record();
	if (readChunk < MIN_READ_CHUNK || readChunk > MAX_READ_CHUNK)
		readChunk = OPT_READ_CHUNK;
	else if (readChunk < OPT_READ_CHUNK)
		readChunk = OPT_READ_CHUNK - OPT_READ_CHUNK%readChunk;
	readChunkBuffer = new double[readChunk];
}

LibGDF::~LibGDF()
{
	delete[] readChunkBuffer;

	gdfReader->close();
	delete gdfReader;
}

double LibGDF::getStartDate() const
{
	uint32_t startDate[2];
	memcpy(startDate, &gdfReader->getMainHeader_readonly().get_recording_start(), sizeof(startDate));

	double fractionOfDay = ldexp(static_cast<double>(startDate[0]), -32);
	double days = startDate[1];
	return days + fractionOfDay;
}

// TODO: Save events to the primary file.
void LibGDF::save()
{
	DataFile::save();
}

bool LibGDF::load()
{
	return DataFile::load();
}

template<typename T>
void LibGDF::readSignalFromFileFloatDouble(vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample)
{
	assert(firstSample <= lastSample && "Bad parameter order.");
	assert(readChunk > 0);

	if (getSamplesRecorded() <= lastSample)
		invalid_argument("LibGDF: reading out of bounds");

	if (dataChannels.size() < getChannelCount())
		invalid_argument("LibGDF: too few dataChannels");

	uint64_t nextBoundary = (firstSample + readChunk - 1/readChunk);

	while (firstSample <= lastSample)
	{
		uint64_t last = min(nextBoundary, lastSample + 1);
		int n = static_cast<int>(last - firstSample);

		for (uint16 i = 0; i < getChannelCount(); i++)
		{
			gdfReader->getSignal(i, readChunkBuffer, firstSample, last);

			for (int j = 0; j < n; j++)
				dataChannels[i][j] = static_cast<T>(readChunkBuffer[j]);

			dataChannels[i] += n;
		}

		firstSample += n;
		nextBoundary += readChunk;
	}
}

} // namespace AlenkaFile
