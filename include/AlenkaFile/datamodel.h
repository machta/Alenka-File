#ifndef ALENKAFILE_DATAMODEL_H
#define ALENKAFILE_DATAMODEL_H

#include "AlenkaFile/abstractdatamodel.h"

#include <vector>

namespace AlenkaFile
{

class EventTypeTable : public AbstractEventTypeTable
{
public:
	virtual ~EventTypeTable() override {}
	virtual int rowCount() const override { return static_cast<int>(table.size()); }
	virtual void insertRows(int row, int count) override;
	virtual void removeRows(int row, int count) override;
	virtual EventType row(int i) const override { return table[i]; }
	virtual void row(int i, const EventType& value) override { table[i] = value; }

private:
	std::vector<EventType> table;
};

class EventTable : public AbstractEventTable
{
public:
	virtual ~EventTable() override {}
	virtual int rowCount() const override { return static_cast<int>(table.size()); }
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual Event row(int i) const override { return table[i]; }
	virtual void row(int i, const Event& value) override { table[i] = value; }

private:
	std::vector<Event> table;
};

class TrackTable : public AbstractTrackTable
{
public:
	virtual ~TrackTable() override {}
	virtual int rowCount() const override { return static_cast<int>(table.size()); }
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual Track row(int i) const override { return table[i]; }
	virtual void row(int i, const Track& value) override { table[i] = value; }

private:
	std::vector<Track> table;
};

class MontageTable : public AbstractMontageTable
{
public:
	virtual ~MontageTable() override;
	virtual int rowCount() const override { return static_cast<int>(table.size()); }
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual Montage row(int i) const override { return table[i]; }
	virtual void row(int i, const Montage& value) override { table[i] = value; }
	virtual AbstractEventTable* eventTable(int i) override;
	virtual AbstractTrackTable* trackTable(int i) override;

protected:
	virtual AbstractEventTable* makeEventTable() override { return new EventTable(); }
	virtual AbstractTrackTable* makeTrackTable() override { return new TrackTable(); }

private:
	std::vector<Montage> table;
	std::vector<AbstractEventTable*> eTable;
	std::vector<AbstractTrackTable*> tTable;
};

} // namespace AlenkaFile

#endif // ALENKAFILE_DATAMODEL_H
