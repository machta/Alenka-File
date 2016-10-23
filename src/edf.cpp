#include "edf.h"

#include "edflib.h"

#include <cassert>
#include <ctime>

using namespace std;

const int MIN_READ_CHUNK = 200;
const int OPT_READ_CHUNK = 2*1000;
const int MAX_READ_CHUNK = 2*1000*1000;

EDF::EDF(const string& filePath, const string& type) : DataFile(filePath)
{
	edfhdr = new edf_hdr_struct;

	string fp = filePath + "." + type;

	int err = edfopen_file_readonly(fp.c_str(), edfhdr, EDFLIB_DO_NOT_READ_ANNOTATIONS);
	assert(err == 0 && "File EDF was not successfully opened."); (void)err;

	samplesRecorded = edfhdr->signalparam[0].smp_in_file;

	samplingFrequency = samplesRecorded;
	samplingFrequency /= edfhdr->file_duration;
	samplingFrequency *= 10*1000*1000;

	numberOfChannels = edfhdr->edfsignals;

	readChunk = edfhdr->signalparam[0].smp_in_datarecord;
	if (readChunk < MIN_READ_CHUNK || readChunk > MAX_READ_CHUNK)
		readChunk = OPT_READ_CHUNK;
	else if (readChunk < OPT_READ_CHUNK)
		readChunk = OPT_READ_CHUNK - OPT_READ_CHUNK%readChunk;
	readChunkBuffer = new double[readChunk];
}

EDF::~EDF()
{
	delete[] readChunkBuffer;

	int err = edfclose_file(edfhdr->handle);
	assert(err == 0 && "File EDF couldn't be closed."); (void)err;

	delete edfhdr;
}

time_t EDF::getStartDate(int timeZone) const
{
	tm time;

	time.tm_mday = edfhdr->startdate_day;
	time.tm_mon = edfhdr->startdate_month - 1;
	time.tm_year = edfhdr->startdate_year - 1900;

	time.tm_sec = edfhdr->starttime_second;
	time.tm_min = edfhdr->starttime_minute;
	time.tm_hour = edfhdr->starttime_hour;

	return mktime(&time) - timeZone*60*60;
}

void EDF::save()
{

}

bool EDF::load()
{
	return true;
}

template<typename T>
void EDF::readSignalFromFileFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample)
{
	assert(firstSample <= lastSample && "Bad parameter order.");
	assert(lastSample < getSamplesRecorded() && "Reading out of bounds.");

	int handle = edfhdr->handle;
	long long err; (void)err;

	for (unsigned int i = 0; i < getChannelCount(); i++)
	{
		err = edfseek(handle, i, firstSample, EDFSEEK_SET);
		assert(err == static_cast<long long>(firstSample) && "edfseek failed.");
	}

	assert(readChunk > 0);

	uint64_t nextBoundary = (firstSample + readChunk - 1/readChunk);

	while (firstSample <= lastSample)
	{
		int last = min(nextBoundary, lastSample + 1);
		int n = last - firstSample;

		for (unsigned int i = 0; i < getChannelCount(); i++)
		{
			err = edfread_physical_samples(handle, i, n, readChunkBuffer);
			assert(err == n && "edfread_physical_samples failed.");

			for (int j = 0; j < n; j++)
				dataChannels[i][j] = readChunkBuffer[j];

			dataChannels[i] += n;
		}

		firstSample += n;
		nextBoundary += readChunk;
	}
}
