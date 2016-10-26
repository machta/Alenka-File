#include "libgdf.h"

#include <GDF/Reader.h>

using namespace std;
using namespace gdf;

const int MIN_READ_CHUNK = 200;
const int OPT_READ_CHUNK = 2*1000;
const int MAX_READ_CHUNK = 2*1000*1000;

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

time_t LibGDF::getStartDate(int timeZone) const
{
	uint32_t startDate[2];
	memcpy(startDate, &gdfReader->getMainHeader_readonly().get_recording_start(), sizeof(startDate));

	double fractionOfDay = ldexp(static_cast<double>(startDate[0]), -32);
	double seconds = (startDate[1] - 719529 + fractionOfDay)*24*60*60;
	return static_cast<time_t>(round(seconds));
}

void LibGDF::save()
{

}

bool LibGDF::load()
{
	return true;
}

template<typename T>
void LibGDF::readSignalFromFileFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample)
{
	assert(firstSample <= lastSample && "Bad parameter order.");
	assert(lastSample < getSamplesRecorded() && "Reading out of bounds.");
	assert(dataChannels.size() == getChannelCount() && "Make sure dataChannels has the same number of channels as the file.");
	assert(readChunk > 0);

	uint64_t nextBoundary = (firstSample + readChunk - 1/readChunk);

	while (firstSample <= lastSample)
	{
		uint64_t last = min(nextBoundary, lastSample + 1);
		int n = last - firstSample;

		for (unsigned int i = 0; i < getChannelCount(); i++)
		{
			gdfReader->getSignal(i, readChunkBuffer, firstSample, last);

			for (int j = 0; j < n; j++)
				dataChannels[i][j] = readChunkBuffer[j];

			dataChannels[i] += n;
		}

		firstSample += n;
		nextBoundary += readChunk;
	}
}
