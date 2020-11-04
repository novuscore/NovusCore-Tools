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

constexpr u32 NDBC_INVALID_STRING_INDEX = std::numeric_limits<u32>().max();

class GlobalData;
class DBCExtractor
{
public:
    void ExtractDBCs(std::shared_ptr<JobBatchRunner> jobBatchRunner);

    const std::vector<NDBC::Map>& GetMaps() { return _maps; }
    const std::vector<NDBC::AreaTable>& GetAreas() { return _areas; }
    const std::vector<NDBC::Light>& GetLights() { return _lights; }
    const std::vector<NDBC::LightParams>& GetLightParams() { return _lightParams; }
    const std::vector<NDBC::LightIntBand>& GetLightIntBands() { return _lightIntBands; }
    const std::vector<NDBC::LightFloatBand>& GetLightFloatBands() { return _lightFloatBands; }
    const std::vector<NDBC::LightSkybox>& GetLightSkyboxes() { return _lightSkyboxes; }
    const std::vector<NDBC::LiquidType>& GetLiquidTypes() { return _liquidTypes; }
    const std::vector<NDBC::LiquidMaterial>& GetLiquidMaterials() { return _liquidMaterials; }
    const std::vector<NDBC::CreatureModelData>& GetCreatureModelDatas() { return _creatureModelDatas; }
    const std::vector<NDBC::CreatureDisplayInfo>& GetCreatureDisplayInfos() { return _creatureDisplayInfos; }
    const std::vector<NDBC::EmotesText>& GetEmotesTexts() { return _emotesTexts; }
    const std::vector<NDBC::Spell>& GetSpells() { return _spells; }

    StringTable& GetStringTable() { return _dbcStringTable; }

private:
    bool LoadDBCFile(std::string_view path, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader, u32& numRows);

    template <typename T>
    bool SaveDBCFile(std::shared_ptr<GlobalData> globalData, fs::path name, std::vector<T>& data, StringTable& stringTable)
    {
        fs::path outputPath = globalData->ndbcPath / name;
        std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
        if (!output)
        {
            NC_LOG_ERROR("Failed to create %s. Check admin permissions", name.string().c_str());
            return false;
        }

        u32 numRows = static_cast<u32>(data.size());

        static NDBC::NDBCHeader header;
        output.write(reinterpret_cast<char const*>(&header), sizeof(header)); // Write NDBC Header
        output.write(reinterpret_cast<char const*>(&numRows), sizeof(u32)); // Write number of rows
        output.write(reinterpret_cast<char const*>(data.data()), numRows * sizeof(T)); // Write Data

        // Serialize and write our StringTable to the file
        std::shared_ptr<Bytebuffer> stringTableByteBuffer = Bytebuffer::Borrow<8388608>();
        if (!stringTable.Serialize(stringTableByteBuffer.get()))
        {
            NC_LOG_ERROR("Failed to write StringTable for %s", name.string().c_str());
            return false;
        }

        if (stringTableByteBuffer->writtenData > 0)
        {
            output.write(reinterpret_cast<char const*>(stringTableByteBuffer->GetDataPointer()), stringTableByteBuffer->writtenData);
        }

        output.close();

        return true;
    }

    u32 GetStringIndexFromField(StringTable& stringTable, DBCReader::DBCRow& row, u32 field);
    u32 GetLocaleStringIndexFromField(StringTable& stringTable, DBCReader::DBCRow& row, u32 field);

    bool LoadMap(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadAreaTable(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadLight(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadLightParams(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadLightIntBand(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadLightFloatBand(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadLightSkybox(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadLiquidTypes(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadLiquidMaterials(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadCreatureModelData(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadCreatureDisplayInfo(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadEmotesText(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);
    bool LoadSpell(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader);

private:
    std::vector<NDBC::Map> _maps;
    std::vector<NDBC::AreaTable> _areas;
    std::vector<NDBC::Light> _lights;
    std::vector<NDBC::LightParams> _lightParams;
    std::vector<NDBC::LightIntBand> _lightIntBands;
    std::vector<NDBC::LightFloatBand> _lightFloatBands;
    std::vector<NDBC::LightSkybox> _lightSkyboxes;
    std::vector<NDBC::LiquidType> _liquidTypes;
    std::vector<NDBC::LiquidMaterial> _liquidMaterials;
    std::vector<NDBC::CreatureModelData> _creatureModelDatas;
    std::vector<NDBC::CreatureDisplayInfo> _creatureDisplayInfos;
    std::vector<NDBC::EmotesText> _emotesTexts;
    std::vector<NDBC::Spell> _spells;

    StringTable _dbcStringTable;
};