#include <AlenkaFile/mat.h>

#include <matio.h>
#include <boost/date_time.hpp>

#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace AlenkaFile;
using namespace boost::posix_time;

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
void decodeArray(void* a, B* b, matio_types type, int n = 1, int offset = 0)
{
#define CASE(a_, b_) case a_: convertArray(reinterpret_cast<b_*>(a) + offset, b, n); break;
	switch (type)
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

// TODO: Allow adding to these with a parameter from the constructor.
const vector<locale> formats {
	locale(locale::classic(), new time_input_facet("%d-%b-%Y %H:%M:%s")),
	locale(locale::classic(), new time_input_facet("%d-%B-%Y %H:%M:%s")),
	locale(locale::classic(), new time_input_facet("%Y-%m-%d %H:%M:%s")),
	locale(locale::classic(), new time_input_facet("%Y/%m/%d %H:%M:%s")),
	locale(locale::classic(), new time_input_facet("%d.%m.%Y %H:%M:%s"))
};

time_t pt_to_time_t(const ptime& pt)
{
	ptime timet_start(boost::gregorian::date(1970, 1, 1));
	time_duration diff = pt - timet_start;
	return diff.ticks()/time_duration::rep_type::ticks_per_second;
}

bool seconds_from_epoch(const string& s, double* seconds)
{
	ptime pt;

	for(size_t i = 0; i < formats.size(); ++i)
	{
		istringstream is(s);
		is.imbue(formats[i]);
		is >> pt;

		if (pt != ptime())
		{
			*seconds = static_cast<double>(pt_to_time_t(pt));
			return true;
		}
	}

	return false;
}

} // namespace

namespace AlenkaFile
{

// How to decode data in Matlab: data = double(d)*diag(mults);

MAT::MAT(const string& filePath, const MATvars& vars) : DataFile(filePath), vars(vars)
{
	file = Mat_Open(filePath.c_str(), MAT_ACC_RDONLY);

	if (!file)
		throw runtime_error("Error while opening " + filePath);

	readSamplingRate();
	readData();
	readMults();
	readDate();
}

MAT::~MAT()
{
	for (auto e : data)
		Mat_VarFree(e);
	Mat_Close(file);
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
		loadEvents();
		return false;
	}

	return true;
}

void MAT::readSamplingRate()
{
	matvar_t* Fs = Mat_VarRead(file, vars.frequency.c_str());

	if (Fs)
	{
		if (Fs->dims[0] <= 0)
			throw runtime_error("Bad MAT file format");

		decodeArray(Fs->data, &samplingFrequency, Fs->data_type);
	}
	else
	{
		cerr << "Warning: var " << vars.frequency << " missing in MAT file" << endl;

		samplingFrequency = 1000;
	}

	Mat_VarFree(Fs);
}

void MAT::readData()
{
	numberOfChannels = MAX_CHANNELS;
	samplesRecorded = 0;

	if (!readDataVar(vars.data))
	{
		int i = 0;
		string name;

		while (name = vars.data + to_string(i++), readDataVar(name));
	}

	if (sizes.empty())
		throw runtime_error("Empty MAT file");
}

void MAT::readMults()
{
	matvar_t* mults = Mat_VarRead(file, vars.multipliers.c_str());

	if (mults)
	{
		int cols = mults->rank < 2 ? 1 : static_cast<int>(mults->dims[1]);
		if (static_cast<int>(mults->dims[0]*cols) < numberOfChannels)
			throw runtime_error("Bad MAT file format");

		multipliers.resize(numberOfChannels);
		decodeArray(mults->data, multipliers.data(), mults->data_type, numberOfChannels);

	}
	else
	{
		multipliers.clear();
	}

	Mat_VarFree(mults);
}

void MAT::readDate()
{
	matvar_t* date = Mat_VarReadInfo(file, vars.date.c_str());

	if (date)
	{
		char tmp[8];

		int err = Mat_VarReadDataLinear(file, date, tmp, 0, 1, 1);
		assert(err == 0); (void)err;

		decodeArray(tmp, &days, date->data_type);
	}

	Mat_VarFree(date);
}

vector<string> MAT::readLabels()
{
	matvar_t* header = Mat_VarReadInfo(file, vars.header.c_str());
	vector<string> labels;
	labels.resize(numberOfChannels, "");

	if (header && header->class_type == MAT_C_STRUCT)
	{
		matvar_t* label = Mat_VarGetStructFieldByName(header, vars.label.c_str(), 0); // Apparently this doesn't need to be freed.

		if (label && label->class_type == MAT_C_CELL)
		{
			for (int i = 0; i < numberOfChannels; ++i)
			{
				matvar_t* cell = Mat_VarGetCell(label, i); // And this too.

				if (cell && cell->class_type == MAT_C_CHAR && cell->rank == 2 && cell->dims[0] == 1)
				{
					int err = Mat_VarReadDataAll(file, cell); // You must explicitly read the data.
					assert(err == 0); (void)err;

					int dim1 = static_cast<int>(cell->dims[1]);
					char* dataPtr = reinterpret_cast<char*>(cell->data);

					for (int j = 0; j < dim1; ++j)
						labels[i].push_back(dataPtr[j]);
				}
			}
		}
	}

	Mat_VarFree(header);
	return labels;
}

void MAT::readEvents(vector<int>* eventPositions, vector<int>* eventDurations)
{
	matvar_t* events = Mat_VarRead(file, vars.events.c_str());
	bool badDateString = false;

	if (events && events->class_type == MAT_C_STRUCT && 1 < events->rank)
	{
		int size = static_cast<int>(events->dims[1]);
		for (int i = 0; i < size; ++i)
		{
			matvar_t* position = Mat_VarGetStructFieldByIndex(events, vars.positionIndex, i);
			double positionValue = -1;
			double durationValue = 0;

			if (position)
			{
				if (position->class_type == MAT_C_CHAR && position->rank == 2 && position->dims[0] == 1)
				{
					int dim1 = static_cast<int>(position->dims[1]);
					char* dataPtr = reinterpret_cast<char*>(position->data);

					string positionString;
					for (int j = 0; j < dim1; ++j)
						positionString.push_back(dataPtr[j]);

					double seconds = 0;
					bool res = !seconds_from_epoch(positionString, &seconds);
					badDateString = badDateString || res;
					seconds /= 24*60*60;
					positionValue = DataFile::daysUpTo1970 + seconds;
				}
				else
				{
					decodeArray(position->data, &positionValue, position->data_type);
				}

				positionValue -= days;
				positionValue *= 24*60*60;
				positionValue *= samplingFrequency;
			}

			matvar_t* duration = Mat_VarGetStructFieldByIndex(events, vars.durationIndex, i);

			if (duration)
			{
				decodeArray(duration->data, &durationValue, duration->data_type);
				durationValue *= samplingFrequency;
			}

			//if (0 < positionValue)
			{
				eventPositions->push_back(static_cast<int>(round(positionValue)));
				eventDurations->push_back(static_cast<int>(round(durationValue)));
			}
		}
	}

	if (badDateString)
		cerr << "Warning: unrecognized date format" << endl;

	Mat_VarFree(events);
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
		assert(err == 0); (void)err;

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

	vector<string> labels = readLabels();

	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		if (!labels[i].empty())
		{
			auto r = defaultTracks->row(i);
			r.label = labels[i];
			defaultTracks->row(i, r);
		}
	}
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

void MAT::loadEvents()
{
	vector<int> eventPositions;
	vector<int> eventDurations;
	readEvents(&eventPositions, &eventDurations);

	AbstractEventTable* eventTable = getDataModel()->montageTable()->eventTable(0);

	assert(eventDurations.size() == eventDurations.size());
	int count = static_cast<int>(eventDurations.size());

	if (count > 0)
	{
		AbstractEventTypeTable* ett = getDataModel()->eventTypeTable();
		ett->insertRows(0);

		EventType et = ett->row(0);
		et.name = "MAT events";
		ett->row(0, et);

		eventTable->insertRows(0, count);

		for (int i = 0; i < count; ++i)
		{
			Event e = eventTable->row(i);

			e.type = 0;
			e.position = eventPositions[i];
			e.duration = eventDurations[i];
			e.channel = -1;

			eventTable->row(i, e);
		}
	}
}

} // namespace AlenkaFile
