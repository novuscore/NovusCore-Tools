#pragma once
#include <NovusTypes.h>
#include <Utils/DebugHandler.h>
#include <Containers/StringTable.h>

#include "../Utils/JobBatch.h"
#include "../Utils/DBCReader.h"
#include "../Utils/MPQLoader.h"
#include "../Formats/DBC/Structures.h"

#include <filesystem>
namespace fs = std::filesystem;

class GlobalData;
class DBCExtractor
{
public:
    void ExtractDBCs(std::shared_ptr<JobBatchRunner> jobBatchRunner);

    const std::vector<DBC::Map>& GetMaps() { return _maps; }
    const std::vector<DBC::CreatureModelData>& GetCreatureModelDatas() { return _creatureModelDatas; }
    const std::vector<DBC::CreatureDisplayInfo>& GetCreatureDisplayInfos() { return _creatureDisplayInfos; }
    const std::vector<DBC::EmotesText>& GetEmotesTexts() { return _emotesTexts; }
    const std::vector<DBC::Spell>& GetSpells() { return _spells; }

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
    std::vector<DBC::Map> _maps;
    std::vector<DBC::CreatureModelData> _creatureModelDatas;
    std::vector<DBC::CreatureDisplayInfo> _creatureDisplayInfos;
    std::vector<DBC::EmotesText> _emotesTexts;
    std::vector<DBC::Spell> _spells;

    StringTable _dbcStringTable;
};