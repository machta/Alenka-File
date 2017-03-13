#include <AlenkaFile/datafile.h>

#include "AlenkaFile/datamodel.h"

#include <cassert>

using namespace AlenkaFile;
using namespace std;

namespace
{

template<class T>
void eraseVector(vector<T>& v, int i, int count)
{
	v.erase(v.begin() + i, v.begin() + i + count);
}

} // namespace

namespace AlenkaFile
{

void EventTypeTable::insertRows(int row, int count)
{
	for (int i = 0; i < count; i++)
	{
		EventType et;
		et.id = row + i;
		et.name = "Type " + to_string(row + i);
		et.opacity = 0.25;
		et.color[0] = 255;
		et.color[1] = et.color[2] = 0;
		et.hidden = false;

		table.insert(table.begin() + row + i, et);
	}
}

void EventTypeTable::removeRows(int row, int count)
{
	eraseVector(table, row, count);
}

void EventTable::insertRows(int row, int count)
{
	for (int i = 0; i < count; i++)
	{
		Event e;
		e.label = "Event " + to_string(row + i);
		e.position = 0;
		e.duration = 1;
		e.channel = -2;

		table.insert(table.begin() + row + i, e);
	}
}

void EventTable::removeRows(int row, int count)
{
	eraseVector(table, row, count);
}

void TrackTable::insertRows(int row, int count)
{
	for (int i = 0; i < count; i++)
	{
		Track t;
		t.label = "Track " + to_string(row + i);
		t.code = "out = in(" + to_string(i) + ");";
		t.color[0] = t.color[1] = t.color[2] = 0;
		t.amplitude = -0.000008;
		t.hidden = false;

		table.insert(table.begin() + row + i, t);
	}
}

void TrackTable::removeRows(int row, int count)
{
	eraseVector(table, row, count);
}

MontageTable::~MontageTable()
{
	assert(eTable.size() == tTable.size());

	for (unsigned int i = 0; i < eTable.size(); i++)
	{
		delete eTable[i];
		delete tTable[i];
	}
}

void MontageTable::insertRows(int row, int count)
{
	for (int i = 0; i < count; i++)
	{
		Montage m;
		m.name = "Montage " + to_string(row + i);
		m.save = false;

		table.insert(table.begin() + row + i, m);
	}

	eTable.insert(eTable.begin() + row, count, nullptr);
	tTable.insert(tTable.begin() + row, count, nullptr);
	for (int i = 0; i < count; i++)
	{
		eTable[row + i] = makeEventTable();
		tTable[row + i] = makeTrackTable();
	}
}

void MontageTable::removeRows(int row, int count)
{
	eraseVector(table, row, count);

	for (int i = 0; i < count; i++)
	{
		delete eTable[row + i];
		delete tTable[row + i];
	}
	eraseVector(eTable, row, count);
	eraseVector(tTable, row, count);
}

AbstractEventTable*MontageTable::eventTable(int i)
{
	return eTable[i];
}

AbstractTrackTable*MontageTable::trackTable(int i)
{
	return tTable[i];
}

} // namespace AlenkaFile
