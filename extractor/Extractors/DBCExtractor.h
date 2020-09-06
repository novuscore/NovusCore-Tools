#pragma once
#include <NovusTypes.h>

#include <Utils/DebugHandler.h>
#include <Containers/StringTable.h>

#include <filesystem>
namespace fs = std::filesystem;

#include "../Utils/JobBatch.h"
#include "../DBC/DBCReader.h"
#include "../DBC/DBCStructures.h"
#include "../MPQ/MPQLoader.h"

class GlobalData;
class DBCExtractor
{
public:
    void ExtractDBCs(std::shared_ptr<JobBatchRunner> jobBatchRunner);

    const std::vector<DBCMap>& GetMaps() { return _maps; }
    const std::vector<DBCCreatureModelData>& GetCreatureModelDatas() { return _creatureModelDatas; }
    const std::vector<DBCCreatureDisplayInfo>& GetCreatureDisplayInfos() { return _creatureDisplayInfos; }
    const std::vector<DBCEmotesText>& GetEmotesTexts() { return _emotesTexts; }
    const std::vector<DBCSpell>& GetSpells() { return _spells; }

    StringTable& GetStringTable() { return _dbcStringTable; }

private:
    u32 GetNameIndexFromField(DBCReader::DBCRow& row, u32 field);

    bool LoadMap(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadCreatureModelData(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadCreatureDisplayInfo(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadEmotesText(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadSpell(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);

    void CreateDBCStringTableFile(std::shared_ptr<GlobalData> globalData);
private:
    std::vector<DBCMap> _maps;
    std::vector<DBCCreatureModelData> _creatureModelDatas;
    std::vector<DBCCreatureDisplayInfo> _creatureDisplayInfos;
    std::vector<DBCEmotesText> _emotesTexts;
    std::vector<DBCSpell> _spells;

    StringTable _dbcStringTable;
};