#include <AlenkaFile/gdf2.h>

#include <algorithm>
#include <cstring>

using namespace std;
using namespace AlenkaFile;

namespace
{

void readRecord(fstream& file, char* rawBuffer, double* samples, int n, int sampleSize, const function<double (void*)>& convertor, bool isLittleEndian)
{
	file.read(rawBuffer, sampleSize*n);

	if (isLittleEndian == false)
	{
		for (int i = 0; i < n; ++i)
		{
			DataFile::changeEndianness(rawBuffer + i*sampleSize, sampleSize);
		}
	}

	for (int i = 0; i < n; i++)
	{
		samples[i] = convertor(rawBuffer);
		rawBuffer += sampleSize;
	}
}

/**
 * @brief calibrateSamples
 *
 * This must be used in order to scale the samples properly.
 * The samples are usually stored as ineger values and must be "adjusted"
 * to get the proper floating point value.
 *
 * When this is not used, the raw data is returned.
 */
void calibrateSamples(double* samples, int n, double digitalMinimum, double scale, double physicalMinimum)
{
	for (int i = 0; i < n; i++)
	{
		samples[i] -= digitalMinimum;
		samples[i] /= scale;
		samples[i] += physicalMinimum;
	}
}

} // namespace

// TODO: handle fstream exceptions in a clear and more informative way

namespace AlenkaFile
{

GDF2::GDF2(const string& filePath, bool uncalibrated) : DataFile(filePath)
{
	isLittleEndian = testLittleEndian();

	file.exceptions(ifstream::failbit | ifstream::badbit);
	file.open(filePath, file.in | file.out | file.binary);
	assert(file.is_open() && "File GDF2 was not successfully opened.");

	// Load fixed header.
	seekFile(0, true);

	readFile(fh.versionID, 8);
	fh.versionID[8] = 0;
	int major, minor;
	sscanf(fh.versionID + 4, "%d.%d", &major, &minor);
	version = minor + 100*major;

	if (string(fh.versionID, 3) != "GDF" || major != 2)
	{
		throw runtime_error("Unrecognized file format.");
	}

	readFile(fh.patientID, 66);
	fh.patientID[66] = 0;

	seekFile(10);

	readFile(&fh.drugs);

	readFile(&fh.weight);

	readFile(&fh.height);

	readFile(&fh.patientDetails);

	readFile(fh.recordingID, 64);
	fh.recordingID[64] = 0;

	readFile(fh.recordingLocation, 4);

	readFile(fh.startDate, 2);

	readFile(fh.birthday, 2);

	readFile(&fh.headerLength);

	readFile(fh.ICD, 6);

	readFile(&fh.equipmentProviderID);

	seekFile(6);

	readFile(fh.headsize, 3);

	readFile(fh.positionRE, 3);

	readFile(fh.positionGE, 3);

	readFile(&fh.numberOfDataRecords);

	if (fh.numberOfDataRecords < 0)
	{
		runtime_error("GDF file with unknown number of data records is not supported.");
	}

	double duration;
	if (version > 220)
	{
		double* ptr = reinterpret_cast<double*>(fh.durationOfDataRecord);
		readFile(ptr, 1);
		duration = *ptr;
	}
	else
	{
		uint32_t* ptr = reinterpret_cast<uint32_t*>(fh.durationOfDataRecord);
		readFile(ptr, 2);
		duration = static_cast<double>(ptr[0])/static_cast<double>(ptr[1]);
	}

	readFile(&fh.numberOfChannels);

	// Load variable header.
	seekFile(2);
	assert(tellFile() == streampos(256) && "Make sure we read all of the fixed header.");

	vh.label = new char[getChannelCount()][16 + 1];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		readFile(vh.label[i], 16);
		vh.label[i][16] = 0;
	}

	vh.typeOfSensor = new char[getChannelCount()][80 + 1];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		readFile(vh.typeOfSensor[i], 80);
		vh.typeOfSensor[i][80] = 0;
	}

	seekFile(6*getChannelCount());

	vh.physicalDimensionCode = new uint16_t[getChannelCount()];
	readFile(vh.physicalDimensionCode, getChannelCount());

	vh.physicalMinimum = new double[getChannelCount()];
	readFile(vh.physicalMinimum, getChannelCount());

	vh.physicalMaximum = new double[getChannelCount()];
	readFile(vh.physicalMaximum, getChannelCount());

	vh.digitalMinimum = new double[getChannelCount()];
	readFile(vh.digitalMinimum, getChannelCount());

	vh.digitalMaximum = new double[getChannelCount()];
	readFile(vh.digitalMaximum, getChannelCount());

	seekFile(64*getChannelCount());

	vh.timeOffset = new float[getChannelCount()];
	readFile(vh.timeOffset, getChannelCount());

	vh.lowpass = new float[getChannelCount()];
	readFile(vh.lowpass, getChannelCount());

	vh.highpass = new float[getChannelCount()];
	readFile(vh.highpass, getChannelCount());

	vh.notch = new float[getChannelCount()];
	readFile(vh.notch, getChannelCount());

	vh.samplesPerRecord = new uint32_t[getChannelCount()];
	readFile(vh.samplesPerRecord, getChannelCount());

	vh.typeOfData = new uint32_t[getChannelCount()];
	readFile(vh.typeOfData, getChannelCount());

	vh.sensorPosition = new float[getChannelCount()][3];
	readFile(*vh.sensorPosition, 3*getChannelCount());

	vh.sensorInfo = new char[getChannelCount()][20];
	readFile(*vh.sensorInfo, 20*getChannelCount());

	assert((tellFile() == streampos(-1) || tellFile() == streampos(256 + 256*getChannelCount())) && "Make sure we read all of the variable header.");

	// Initialize other members.
	samplesRecorded = vh.samplesPerRecord[0]*fh.numberOfDataRecords;

	startOfData = 256*fh.headerLength;

#define CASE(a_, b_)\
case a_:\
	dataTypeSize = sizeof(b_);\
	convertSampleToDouble = [] (void* sample) -> double\
	{\
		b_ tmp = *reinterpret_cast<b_*>(sample);\
		return static_cast<double>(tmp);\
	};\
	break;

	switch (vh.typeOfData[0])
	{
		CASE(1, int8_t);
		CASE(2, uint8_t);
		CASE(3, int16_t);
		CASE(4, uint16_t);
		CASE(5, int32_t);
		CASE(6, uint32_t);
		CASE(7, int64_t);
		CASE(8, uint64_t);
		CASE(16, float);
		CASE(17, double);
	default:
		throw runtime_error("Unsupported data type.");
		break;
	}

#undef CASE

	if (uncalibrated == false)
	{
		scale = new double[getChannelCount()];
		for (unsigned int i = 0; i < getChannelCount(); ++i)
		{
			scale[i] = (vh.digitalMaximum[i] - vh.digitalMinimum[i])/
					   (vh.physicalMaximum[i] - vh.physicalMinimum[i]);
		}
	}
	else
	{
		scale = nullptr;
	}

	samplingFrequency = vh.samplesPerRecord[0]/duration;

	int64_t dataRecordBytes = vh.samplesPerRecord[0]*getChannelCount()*dataTypeSize;
	startOfEventTable = startOfData + dataRecordBytes*fh.numberOfDataRecords;

	recordRawBuffer = new char[vh.samplesPerRecord[0]*dataTypeSize];
	recordDoubleBuffer = new double[vh.samplesPerRecord[0]];
}

GDF2::~GDF2()
{
	delete[] recordRawBuffer;
	delete[] recordDoubleBuffer;
	delete[] scale;

	// Delete variable header.
	delete[] vh.label;
	delete[] vh.typeOfSensor;
	delete[] vh.physicalDimensionCode;
	delete[] vh.physicalMinimum;
	delete[] vh.physicalMaximum;
	delete[] vh.digitalMinimum;
	delete[] vh.digitalMaximum;
	delete[] vh.timeOffset;
	delete[] vh.lowpass;
	delete[] vh.highpass;
	delete[] vh.notch;
	delete[] vh.samplesPerRecord;
	delete[] vh.typeOfData;
	delete[] vh.sensorPosition;
	delete[] vh.sensorInfo;
}

void GDF2::save(pugi::xml_document* const infoFile)
{
	DataFile::save(infoFile);
}

bool GDF2::load(pugi::xml_document* infoFile)
{
	if (DataFile::load(infoFile) == false)
	{

	}

	return true;
}

void GDF2::readGdfEventTable(int /*numberOfEvents*/, int /*eventTableMode*/)
{
}

template<typename T>
void GDF2::readSignalFromFileFloatDouble(vector<T*> dataChannels, const uint64_t firstSample, const uint64_t lastSample)
{
	assert(firstSample <= lastSample && "Bad parameter order.");
	assert(lastSample < getSamplesRecorded() && "Reading out of bounds.");
	assert(dataChannels.size() == getChannelCount() && "Make sure dataChannels has the same number of channels as the file.");

	int samplesPerRecord = vh.samplesPerRecord[0];
	int recordChannelBytes = samplesPerRecord*dataTypeSize;

	uint64_t recordI = firstSample/samplesPerRecord;
	seekFile(startOfData + recordI*recordChannelBytes*getChannelCount(), true);

	int firstSampleToCopy = static_cast<int>(firstSample%samplesPerRecord);

	for (; recordI <= lastSample/samplesPerRecord; ++recordI)
	{
		int copyCount = min<int>(samplesPerRecord - firstSampleToCopy, static_cast<int>(lastSample - recordI*samplesPerRecord) - firstSampleToCopy + 1);

		assert(copyCount > 0 && "Ensure there is something to copy");
		assert(firstSample + copyCount - 1 <= lastSample && "Make sure we don't write beyond the output buffer.");
		assert(firstSampleToCopy + copyCount <= samplesPerRecord && "Make sure we don't acceed tmp buffer size.");

		for (unsigned int channelI = 0; channelI < getChannelCount(); ++channelI)
		{
			readRecord(file, recordRawBuffer, recordDoubleBuffer, samplesPerRecord, dataTypeSize, convertSampleToDouble, isLittleEndian);

			if (scale != nullptr)
				calibrateSamples(recordDoubleBuffer, samplesPerRecord, vh.digitalMinimum[channelI], scale[channelI], vh.physicalMinimum[channelI]);

			for (int i = 0; i < copyCount; i++)
				dataChannels[channelI][i] = static_cast<T>(recordDoubleBuffer[firstSampleToCopy + i]);

			dataChannels[channelI] += copyCount;
		}

		firstSampleToCopy = 0;
	}
}

} // namespace AlenkaFile
