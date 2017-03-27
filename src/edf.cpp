#include <AlenkaFile/edf.h>

#include "edflib_extended.h"
#include <boost/filesystem.hpp>

#include <algorithm>
#include <cassert>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <set>

using namespace std;
using namespace AlenkaFile;

namespace
{

const int MIN_READ_CHUNK = 200;
const int OPT_READ_CHUNK = 2*1000;
const int MAX_READ_CHUNK = 2*1000*1000;

void copyMetaInfo(int file, const edf_hdr_struct* edfhdr, int numberOfChannels, int samplingFrequency)
{
	const edf_param_struct* sp = edfhdr->signalparam;
	int res = 0;

	for (int i = 0; i < numberOfChannels; ++i)
	{
		res |=  edf_set_samplefrequency(file, i, samplingFrequency);
		res |=  edf_set_physical_maximum(file, i, sp[i].phys_max);
		res |=  edf_set_physical_minimum(file, i, sp[i].phys_min);
		res |=  edf_set_digital_maximum(file, i, sp[i].dig_max);
		res |=  edf_set_digital_minimum(file, i, sp[i].dig_min);
		res |=  edf_set_label(file, i, sp[i].label);
		res |=  edf_set_prefilter(file, i, sp[i].prefilter);
		res |=  edf_set_transducer(file, i, sp[i].transducer);
		res |=  edf_set_physical_dimension(file, i, sp[i].physdimension);
	}

	res |=  edf_set_startdatetime(file, edfhdr->startdate_year, edfhdr->startdate_month, edfhdr->startdate_day, edfhdr->starttime_hour, edfhdr->starttime_minute, edfhdr->starttime_second);
	res |=  edf_set_patientname(file, edfhdr->patient_name);
	res |=  edf_set_patientcode(file, edfhdr->patientcode);

	// TODO: Figure out how to set these. This version causes problems. Or don't copy it: isntead show a warning that the file will have some incorrect values, and make it a known issue.
	//edf_set_gender_char(file, edfhdr->gender);
	//edf_set_birthdate_char(file, edfhdr->birthdate);

	res |=  edf_set_patient_additional(file, edfhdr->patient_additional);
	res |=  edf_set_admincode(file, edfhdr->admincode);
	res |=  edf_set_technician(file, edfhdr->technician);
	res |=  edf_set_equipment(file, edfhdr->equipment);
	res |=  edf_set_recording_additional(file, edfhdr->recording_additional);

	assert(res == 0 && "Make sure all values were set correctly."); (void)res;
}

long long convertEventPosition(int sample, double samplingFrequency)
{
	return static_cast<long long>(round(sample/samplingFrequency*10000));
}

int convertEventPositionBack(long long onset, double samplingFrequency)
{
	return static_cast<int>(round(static_cast<double>(onset)*samplingFrequency/10000/1000));
}

} // namespace

namespace AlenkaFile
{

EDF::EDF(const string& filePath) : DataFile(filePath)
{
	edfhdr = new edf_hdr_struct;

	openFile();
}

EDF::~EDF()
{
	delete[] readChunkBuffer;

	int err = edfclose_file(edfhdr->handle);
	assert(err == 0 && "EDF file couldn't be closed."); (void)err;

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
	saveSecondaryFile();

	AbstractMontageTable* montageTable = getDataModel()->montageTable();
	int tablesToSave = 0;

	for (int i = 0; i < montageTable->rowCount(); ++i)
	{
		if (montageTable->row(i).save)
			++tablesToSave;
	}

	if (tablesToSave == 0)
		return;

	// Open a temporary file for writing.
	string tmpPathString = getFilePath() + ".%%%%.tmp";
	boost::filesystem::path tmpPath = boost::filesystem::unique_path(tmpPathString);

	int tmpFile = edfopen_file_writeonly(tmpPath.c_str(), edfhdr->filetype, numberOfChannels);
	if (tmpFile < 0)
		cerr << "edfopen_file_writeonly error: " << tmpFile << endl;
	assert(0 <= tmpFile  && "Temporary file was not successfully opened.");

	// Copy data into the new file.
	int sf = static_cast<int>(round(samplingFrequency));
	copyMetaInfo(tmpFile, edfhdr, numberOfChannels, sf);

	double* buffer = new double[numberOfChannels*sf*sizeof(double)];

	for (uint64_t sampleIndex = 0; sampleIndex < getSamplesRecorded(); sampleIndex += sf)
	{
		readSignal(buffer, sampleIndex, sampleIndex + sf - 1);

		for (int i = 0; i < numberOfChannels; ++i)
		{
			int res = edfwrite_physical_samples(tmpFile, buffer + i*sf);
			assert(res == 0 && "Test write success."); (void)res;
		}
	}

	// Write events.
	for (int i = 0; i < montageTable->rowCount(); ++i)
	{
		if (montageTable->row(i).save)
		{
			AbstractEventTable* eventTable = montageTable->eventTable(i);

			for (int j = 0; j < eventTable->rowCount(); ++j)
			{
				Event e = eventTable->row(j);

				// Skip events belonging to tracks greater thatn the number of channels in the file.
				// TODO: Perhaps make a warning about this?
				if (-1 <= e.channel && e.channel < static_cast<int>(getChannelCount()) && e.type >= 0)
				{
					long long onset = convertEventPosition(e.position, getSamplingFrequency());
					long long duration = convertEventPosition(e.duration, getSamplingFrequency());

					stringstream ss;
					ss << "t=" << e.type << " c=" << e.channel << "|" << e.label << "|" << e.description;

					int res = edfwrite_annotation_utf8(tmpFile, onset, duration, ss.str().c_str());
					assert(res == 0 && "Test write success."); (void)res;
				}
			}
		}
	}

	// Replace the original file with the new one.
	int res = edfclose_file(tmpFile);
	assert(res == 0 && "Closing tmp file failed."); (void)res;

	res = edfclose_file(edfhdr->handle);
	assert(res == 0 && "EDF file couldn't be closed."); (void)res;

	boost::filesystem::rename(tmpPath, getFilePath());

	openFile();
}

bool EDF::load()
{
	if (DataFile::loadSecondaryFile() == false)
	{
		fillDefaultMontage();
		loadEvents();
	}

	return true;
}

void EDF::openFile()
{
	int err = edfopen_file_readonly(getFilePath().c_str(), edfhdr, EDFLIB_READ_ALL_ANNOTATIONS);
	if (err < 0)
		cerr << "edfopen_file_readonly error: " << edfhdr->filetype << endl;
	assert(err == 0 && "File EDF was not successfully opened."); (void)err;

	samplesRecorded = edfhdr->signalparam[0].smp_in_file;

	samplingFrequency = static_cast<double>(samplesRecorded);
	samplingFrequency /= static_cast<double>(edfhdr->file_duration);
	samplingFrequency *= 10*1000*1000;

	numberOfChannels = edfhdr->edfsignals;

	readChunk = edfhdr->signalparam[0].smp_in_datarecord;
	if (readChunk < MIN_READ_CHUNK || readChunk > MAX_READ_CHUNK)
		readChunk = OPT_READ_CHUNK;
	else if (readChunk < OPT_READ_CHUNK)
		readChunk = OPT_READ_CHUNK - OPT_READ_CHUNK%readChunk;
	readChunkBuffer = new double[readChunk];
}

template<typename T>
void EDF::readChannelsFloatDouble(vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample)
{
	assert(firstSample <= lastSample && "Bad parameter order.");
	assert(lastSample < getSamplesRecorded() && "Reading out of bounds.");
	assert(dataChannels.size() == getChannelCount() && "Make sure dataChannels has the same number of channels as the file.");

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
		uint64_t last = min(nextBoundary, lastSample + 1);
		int n = static_cast<int>(last - firstSample);

		for (unsigned int i = 0; i < getChannelCount(); i++)
		{
			err = edfread_physical_samples(handle, i, n, readChunkBuffer);
			assert(err == n && "edfread_physical_samples failed.");

			for (int j = 0; j < n; j++)
				dataChannels[i][j] = static_cast<T>(readChunkBuffer[j]);

			dataChannels[i] += n;
		}

		firstSample += n;
		nextBoundary += readChunk;
	}
}

void EDF::fillDefaultMontage()
{
	getDataModel()->montageTable()->insertRows(0);

	assert(getChannelCount() > 0);

	AbstractTrackTable* defaultTracks = getDataModel()->montageTable()->trackTable(0);
	defaultTracks->insertRows(0, getChannelCount());

	for (int i = 0; i < defaultTracks->rowCount(); ++i)
	{
		Track t = defaultTracks->row(i);
		t.label = edfhdr->signalparam[i].label;
		defaultTracks->row(i, t);
	}
}

void EDF::loadEvents()
{
//	AbstractEventTypeTable* ett = getDataModel()->eventTypeTable();

//	assert(ett->rowCount() == 0);
//	ett->insertRows(0);

//	EventType type = ett->row(0);
//	type.id = 0;
//	type.name = "EDF annotations";
//	ett->row(0, type);

	assert(getDataModel()->montageTable()->rowCount() > 0);

	edf_annotation_struct event;
	int eventCount = static_cast<int>(edfhdr->annotations_in_file);

	AbstractEventTable* et = getDataModel()->montageTable()->eventTable(0);
	assert(et->rowCount() == 0);
	et->insertRows(0, eventCount);

	for (int i = 0; i < eventCount; ++i)
	{
		edf_get_annotation(edfhdr->handle, i, &event);

		Event e = et->row(i);

		e.position = convertEventPositionBack(event.onset, samplingFrequency);

		double duration = 0;
		sscanf(event.duration, "%lf", &duration);
		if (duration != 0)
			e.duration = static_cast<int>(round(duration*samplingFrequency));

		sscanf(event.annotation, "t=%d c=%d", &e.type, &e.channel);
		string annotation = event.annotation;

		auto first = annotation.find("|") + 1;
		auto second = annotation.find("|", first);

		string label = annotation.substr(first, second - first);
		if (label.size() > 0)
			e.label = label;

		string description = annotation.substr(second + 1);
		if (description.size() > 0)
			e.description = description;

		et->row(i, e);
	}

	addUsedEventTypes();
}

void EDF::addUsedEventTypes()
{
	AbstractEventTypeTable* ett = getDataModel()->eventTypeTable();
	AbstractEventTable* et = getDataModel()->montageTable()->eventTable(0);

	set<int> typesUsed;
	for (int i = 0; i < et->rowCount(); ++i)
		typesUsed.insert(et->row(i).type);

	int count = static_cast<int>(typesUsed.size());
	assert(ett->rowCount() == 0);
	ett->insertRows(0, count);

	auto it = typesUsed.begin();
	for (int i = 0; i < count; ++i)
	{
		EventType et = ett->row(i);

		int id = *it;
		et.id = id;
		et.name = "Type " + to_string(id);

		ett->row(i, et);

		++it;
	}

	for (int i = 0; i < et->rowCount(); ++i)
	{
		Event e = et->row(i);

		auto it = typesUsed.find(e.type);
		assert(it != typesUsed.end());

		e.type = static_cast<int>(distance(typesUsed.begin(), it));

		et->row(i, e);
	}
}

} // namespace AlenkaFile
