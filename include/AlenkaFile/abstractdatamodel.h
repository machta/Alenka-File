#ifndef ALENKAFILE_ABSTRACTDATAMODEL_H
#define ALENKAFILE_ABSTRACTDATAMODEL_H

#include <string>
#include <cstdio>

namespace AlenkaFile
{

struct EventType
{
	int id;
	std::string name;
	double opacity;
	unsigned char color[3];
	bool hidden;
};

class AbstractEventTypeTable
{
public:
	virtual ~AbstractEventTypeTable() {}
	virtual int rowCount() = 0;
	virtual void insertRows(int row, int count = 1) = 0;
	virtual void removeRows(int row, int count = 1) = 0;
	virtual EventType row(int i) const = 0;
	virtual void row(int i, const EventType& value) = 0;
};

struct Event
{
	std::string label;
	int type;
	int position;
	int duration;
	int channel;
	std::string description;
};

class AbstractEventTable
{
public:
	virtual ~AbstractEventTable() {}
	virtual int rowCount() = 0;
	virtual void insertRows(int row, int count = 1) = 0;
	virtual void removeRows(int row, int count = 1) = 0;
	virtual Event row(int i) const = 0;
	virtual void row(int i, const Event& value) = 0;
};

struct Track
{
	std::string label;
	std::string code;
	unsigned char color[3];
	double amplitude;
	bool hidden;
};

class AbstractTrackTable
{
public:
	virtual ~AbstractTrackTable() {}
	virtual int rowCount() = 0;
	virtual void insertRows(int row, int count = 1) = 0;
	virtual void removeRows(int row, int count = 1) = 0;
	virtual Track row(int i) const = 0;
	virtual void row(int i, const Track& value) = 0;
};

struct Montage
{
	std::string name;
	bool save;
};

class AbstractMontageTable
{
public:
	virtual ~AbstractMontageTable() {}
	virtual int rowCount() = 0;
	virtual void insertRows(int row, int count = 1) = 0;
	virtual void removeRows(int row, int count = 1) = 0;
	virtual Montage row(int i) const = 0;
	virtual void row(int i, const Montage& value) = 0;
	virtual AbstractEventTable* eventTable(int i) = 0;
	virtual AbstractTrackTable* trackTable(int i) = 0;
};

struct DataModel
{
	AbstractEventTypeTable* eventTypeTable;
	AbstractMontageTable* montageTable;

	static std::string color2str(const unsigned char color[3])
	{
		std::string str = "#";

		for (int i = 0; i < 3; i++)
		{
			char tmp[3];
			sprintf(tmp, "%02x", color[i]);
			str += tmp;
		}
		return str;
	}
	static void str2color(const char* str, unsigned char* color)
	{
		unsigned int r, g, b;
		sscanf(str, "#%02x%02x%02x", &r, &g, &b);
		color[0] = static_cast<unsigned char>(r);
		color[1] = static_cast<unsigned char>(g);
		color[2] = static_cast<unsigned char>(b);
	}
};

} // namespace AlenkaFile

#endif // ALENKAFILE_ABSTRACTDATAMODEL_H
