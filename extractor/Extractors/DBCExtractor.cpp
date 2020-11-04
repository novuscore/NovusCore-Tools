#include "DBCExtractor.h"
#include <Utils/StringUtils.h>

#include "../GlobalData.h"
#include "../Utils/ServiceLocator.h"
#include "../Utils/JobBatchRunner.h"
#include "../Formats/BLP/BlpConvert.h"

#include <fstream>
#include <tracy/Tracy.hpp>

void DBCExtractor::ExtractDBCs(std::shared_ptr<JobBatchRunner> jobBatchRunner)
{
    NC_LOG_SUCCESS("Fetching DBCs");

    auto& globalData = ServiceLocator::GetGlobalData();
    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();
    std::shared_ptr<DBCReader> dbcReader = ServiceLocator::GetDBCReader();

    if (!dbcReader)
    {
        NC_LOG_ERROR("Failed to load dbc files. DBCReader is nullptr");
        return;
    }

    LoadMap(globalData, mpqLoader, dbcReader);
    LoadAreaTable(globalData, mpqLoader, dbcReader);
    LoadLight(globalData, mpqLoader, dbcReader);
    LoadLightParams(globalData, mpqLoader, dbcReader);
    LoadLightIntBand(globalData, mpqLoader, dbcReader);
    LoadLightFloatBand(globalData, mpqLoader, dbcReader);
    LoadLightSkybox(globalData, mpqLoader, dbcReader);
    LoadLiquidTypes(globalData, mpqLoader, dbcReader);
    LoadLiquidMaterials(globalData, mpqLoader, dbcReader);
    LoadCreatureModelData(globalData, mpqLoader, dbcReader);
    LoadCreatureDisplayInfo(globalData, mpqLoader, dbcReader);
    LoadEmotesText(globalData, mpqLoader, dbcReader);
    //LoadSpell(globalData, mpqLoader, dbcReader); // Enable this when we need it, currently slowing down debugging
}

bool DBCExtractor::LoadDBCFile(std::string_view path, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader, u32& numRows)
{
    std::shared_ptr<Bytebuffer> file = mpqLoader->GetFile(path);
    if (!file)
    {
        NC_LOG_ERROR("Failed to load %s", path.data());
        return false;
    }

    NC_LOG_MESSAGE("Loading %s...", path.data());

    if (dbcReader->Load(file) != 0)
        return false;

    numRows = dbcReader->GetNumRows();
    if (numRows == 0)
        return false;

    return true;
}

u32 DBCExtractor::GetStringIndexFromField(StringTable& stringTable, DBCReader::DBCRow& row, u32 field)
{
    std::string value = row.GetString(field);

    if (value.length() == 0)
        return NDBC_INVALID_STRING_INDEX;

    if (StringUtils::EndsWith(value, ".mdx"))
        value = value.substr(0, value.length() - 4) + ".cmodel";

    if (StringUtils::EndsWith(value, ".m2"))
        value = value.substr(0, value.length() - 3) + ".cmodel";

    return stringTable.AddString(value);
}
u32 DBCExtractor::GetLocaleStringIndexFromField(StringTable& stringTable, DBCReader::DBCRow& row, u32 field)
{
    std::string value = "";
    for (u8 i = 0; i < 16; i++)
    {
        value = row.GetString(field + i);

        if (value.length() != 0)
            break;
    }

    if (value.length() == 0)
        return NDBC_INVALID_STRING_INDEX;

    if (StringUtils::EndsWith(value, ".mdx"))
        value = value.substr(0, value.length() - 4) + ".cmodel";

    if (StringUtils::EndsWith(value, ".m2"))
        value = value.substr(0, value.length() - 3) + ".cmodel";

    return stringTable.AddString(value);
}

bool DBCExtractor::LoadMap(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\Map.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable(static_cast<u64>(rows) * 2);
    _maps.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::Map& map = _maps[i];
        map.id = row.GetUInt32(0);
        map.internalName = GetStringIndexFromField(stringTable, row, 1);
        map.instanceType = row.GetUInt32(2);
        map.flags = row.GetUInt32(3);
        map.name = GetLocaleStringIndexFromField(stringTable, row, 5);
        map.expansion = row.GetUInt32(63);
        map.maxPlayers = row.GetUInt32(65);
    }

    return SaveDBCFile(globalData, "Maps.ndbc", _maps, stringTable);
}
bool DBCExtractor::LoadAreaTable(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\AreaTable.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable(rows);
    _areas.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::AreaTable& area = _areas[i];
        area.id = row.GetUInt32(0);
        area.mapId = row.GetUInt32(1);
        area.parentId = row.GetUInt32(2);
        area.areaBit = row.GetUInt32(3);

        u32 flags = row.GetUInt32(4);
        area.flags = *reinterpret_cast<NDBC::AreaTableFlag*>(&flags);
        area.areaLevel = row.GetUInt32(10);
        area.name = GetLocaleStringIndexFromField(stringTable, row, 11); 
    }

    return SaveDBCFile(globalData, "AreaTable.ndbc", _areas, stringTable);
}
bool DBCExtractor::LoadLight(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\Light.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable;
    _lights.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::Light& light = _lights[i];
        light.id = row.GetUInt32(0);
        light.mapId = row.GetUInt32(1);

        /* These values are stored in inches, convert directly to yards
           We also swizzle the position directly into our coordinate system
        
            X = Z;
            Y = Y;
            Z = X;
        */
        light.position.x = row.GetFloat(4) / 36.0f;
        light.position.y = row.GetFloat(3) / 36.0f;
        light.position.z = row.GetFloat(2) / 36.0f;
        light.fallOff.x = row.GetFloat(5) / 36.0f;
        light.fallOff.y = row.GetFloat(6) / 36.0f;

        light.paramClearId = row.GetUInt32(7);
        light.paramClearInWaterId = row.GetUInt32(8);
        light.paramStormId = row.GetUInt32(9);
        light.paramStormInWaterId = row.GetUInt32(10);
        light.paramDeathId = row.GetUInt32(11);

        light.paramUnk1Id = row.GetUInt32(12);
        light.paramUnk2Id = row.GetUInt32(13);
        light.paramUnk3Id = row.GetUInt32(14);
    }

    return SaveDBCFile(globalData, "Light.ndbc", _lights, stringTable);
}
bool DBCExtractor::LoadLightParams(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\LightParams.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable;
    _lightParams.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::LightParams& lightParams = _lightParams[i];
        lightParams.id = row.GetUInt32(0);
        lightParams.highlightSky = row.GetUInt32(1);
        lightParams.lightSkyboxId = row.GetUInt32(2);
        lightParams.cloudTypeId = row.GetUInt32(3);
        lightParams.glow = row.GetFloat(4);
        lightParams.waterShallowAlpha = row.GetFloat(5);
        lightParams.waterDeepAlpha = row.GetFloat(6);
        lightParams.oceanShallowAlpha = row.GetFloat(7);
        lightParams.oceanDeepAlpha = row.GetFloat(8);
        lightParams.flags = row.GetUInt32(9);
    }

    return SaveDBCFile(globalData, "LightParams.ndbc", _lightParams, stringTable);
}
bool DBCExtractor::LoadLightIntBand(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\LightIntBand.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable;
    _lightIntBands.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::LightIntBand& lightIntBand = _lightIntBands[i];
        lightIntBand.id = row.GetUInt32(0);
        lightIntBand.entries = row.GetUInt32(1);

        u32 timeValueBaseOffset = 2; // Time Values start from column 2
        u32 colorValueBaseOffset = 18; // Color Values start from column 18
        for (u32 i = 0; i < 16; i++)
        {
            // For the time values we multiply by 30 to get per second values (Blizzard stores these as 1 value per 30 seconds)
            lightIntBand.timeValues[i] = row.GetUInt32(timeValueBaseOffset + i) * 30;
            lightIntBand.colorValues[i] = row.GetUInt32(colorValueBaseOffset + i);
        }
    }

    return SaveDBCFile(globalData, "LightIntBand.ndbc", _lightIntBands, stringTable);
}
bool DBCExtractor::LoadLightFloatBand(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\LightFloatBand.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable;
    _lightFloatBands.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::LightFloatBand& lightFloatBand = _lightFloatBands[i];
        lightFloatBand.id = row.GetUInt32(0);
        lightFloatBand.entries = row.GetUInt32(1);

        u32 timeValueBaseOffset = 2; // Time Values start from column 2
        u32 valuesBaseOffset = 18; // Color Values start from column 18
        for (u32 i = 0; i < 16; i++)
        {
            // For the time values we multiply by 30 to get per second values (Blizzard stores these as 1 value per 30 seconds)
            lightFloatBand.timeValues[i] = row.GetUInt32(timeValueBaseOffset + i) * 30;
            lightFloatBand.values[i] = row.GetFloat(valuesBaseOffset + i);
        }
    }

    return SaveDBCFile(globalData, "LightFloatBand.ndbc", _lightFloatBands, stringTable);
}
bool DBCExtractor::LoadLightSkybox(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\LightSkybox.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable(rows);
    _lightSkyboxes.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::LightSkybox& lightSkybox = _lightSkyboxes[i];
        lightSkybox.id = row.GetUInt32(0);
        lightSkybox.modelPath = GetStringIndexFromField(stringTable, row, 1);
        lightSkybox.flags = row.GetUInt32(2);
    }

    return SaveDBCFile(globalData, "LightSkybox.ndbc", _lightSkyboxes, stringTable);
}
bool DBCExtractor::LoadLiquidTypes(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\LiquidType.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable(rows);
    _liquidTypes.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::LiquidType& liquidType = _liquidTypes[i];
        liquidType.id = row.GetUInt32(0);
        liquidType.name = GetStringIndexFromField(stringTable, row, 1);
        liquidType.flags = row.GetUInt32(2);
        liquidType.type = row.GetUInt32(3);
        liquidType.soundEntriesId = row.GetUInt32(4);
        liquidType.spellId = row.GetUInt32(5);
        liquidType.maxDarkenDepth = row.GetFloat(6);
        liquidType.fogDarkenIntensity = row.GetFloat(7);
        liquidType.ambDarkenIntensity = row.GetFloat(8);
        liquidType.dirDarkenIntensity = row.GetFloat(9);
        liquidType.lightId = row.GetUInt32(10);
        liquidType.particleScale = row.GetFloat(11);
        liquidType.particleMovement = row.GetUInt32(12);
        liquidType.particleTextureSlots = row.GetUInt32(13);
        liquidType.liquidMaterialId = row.GetUInt32(14);
    }

    return SaveDBCFile(globalData, "LiquidTypes.ndbc", _liquidTypes, stringTable);
}
bool DBCExtractor::LoadLiquidMaterials(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\LiquidMaterial.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable;
    _liquidMaterials.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::LiquidMaterial& liquidMaterial = _liquidMaterials[i];
        liquidMaterial.id = row.GetUInt32(0);
        liquidMaterial.liquidVertexFormat = row.GetUInt32(1);
        liquidMaterial.flags = row.GetUInt32(2);
    }

    return SaveDBCFile(globalData, "LiquidMaterials.ndbc", _liquidMaterials, stringTable);
}
bool DBCExtractor::LoadCreatureModelData(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\CreatureModelData.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable(rows);
    _creatureModelDatas.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);
        {
            NDBC::CreatureModelData& creatureModelData = _creatureModelDatas[i];
            creatureModelData.id = row.GetUInt32(0);
            creatureModelData.flags = row.GetUInt32(1);
            creatureModelData.modelPath = GetStringIndexFromField(stringTable, row, 2);
            creatureModelData.sizeClass = row.GetUInt32(3);
            creatureModelData.modelScale = row.GetFloat(4);
            creatureModelData.bloodLevelId = row.GetUInt32(5);
            creatureModelData.footPrintId = row.GetUInt32(6);

            creatureModelData.footPrintTextureLength = row.GetFloat(7);
            creatureModelData.footprintTextureWidth = row.GetFloat(8);
            creatureModelData.footprintParticleScale = row.GetFloat(9);
            creatureModelData.foleyMaterialId = row.GetUInt32(10);

            creatureModelData.footstepShakeSize = row.GetUInt32(11);
            creatureModelData.deathThudShakeSize = row.GetUInt32(12);
            creatureModelData.soundDataId = row.GetUInt32(13);
            creatureModelData.collisionWidth = row.GetFloat(14);
            creatureModelData.collisionHeight = row.GetFloat(15);
            creatureModelData.mountHeight = row.GetFloat(16);
            creatureModelData.geoBoxMin = vec3(row.GetFloat(17), row.GetFloat(18), row.GetFloat(19));
            creatureModelData.geoBoxMax = vec3(row.GetFloat(20), row.GetFloat(21), row.GetFloat(22));
            creatureModelData.worldEffectScale = row.GetFloat(23);
            creatureModelData.attachedEffectScale = row.GetFloat(24);
            creatureModelData.missileCollisionRadius = row.GetFloat(25);
            creatureModelData.missileCollisionPush = row.GetFloat(26);
            creatureModelData.missileCollisionRaise = row.GetFloat(27);
        }
    }

    return SaveDBCFile(globalData, "CreatureModelData.ndbc", _creatureModelDatas, stringTable);
}
bool DBCExtractor::LoadCreatureDisplayInfo(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\CreatureDisplayInfo.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable(static_cast<u64>(rows) * 3);
    _creatureDisplayInfos.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);
        {
            NDBC::CreatureDisplayInfo& creatureDisplayInfo = _creatureDisplayInfos[i];
            creatureDisplayInfo.id = row.GetUInt32(0);
            creatureDisplayInfo.modelId = row.GetUInt32(1);
            creatureDisplayInfo.soundId = row.GetUInt32(2);
            creatureDisplayInfo.extraDisplayInfoId = row.GetUInt32(3);
            creatureDisplayInfo.scale = row.GetFloat(4);
            creatureDisplayInfo.opacity = row.GetUInt32(5);

            creatureDisplayInfo.texture1 = GetStringIndexFromField(stringTable, row, 6);
            creatureDisplayInfo.texture2 = GetStringIndexFromField(stringTable, row, 7);
            creatureDisplayInfo.texture3 = GetStringIndexFromField(stringTable, row, 8);
            creatureDisplayInfo.portraitTextureName = GetStringIndexFromField(stringTable, row, 9);

            creatureDisplayInfo.bloodLevelId = row.GetUInt32(10);
            creatureDisplayInfo.bloodId = row.GetUInt32(11);
            creatureDisplayInfo.npcSoundsId = row.GetUInt32(12);
            creatureDisplayInfo.particlesId = row.GetUInt32(13);
            creatureDisplayInfo.creatureGeosetData = row.GetUInt32(14);
            creatureDisplayInfo.objectEffectPackageId = row.GetUInt32(15);
        }
    }

    return SaveDBCFile(globalData, "CreatureDisplayInfo.ndbc", _creatureDisplayInfos, stringTable);
}
bool DBCExtractor::LoadEmotesText(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\EmotesText.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable(rows);
    _emotesTexts.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::EmotesText& emoteText = _emotesTexts[i];
        emoteText.id = row.GetUInt32(0);
        emoteText.internalName = GetStringIndexFromField(stringTable, row, 1);
        emoteText.animationId = row.GetUInt32(2);
    }

    return SaveDBCFile(globalData, "EmotesText.ndbc", _emotesTexts, stringTable);
}
bool DBCExtractor::LoadSpell(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    u32 rows = 0;
    if (!LoadDBCFile("DBFilesClient\\Spell.dbc", mpqLoader, dbcReader, rows))
        return false;

    StringTable stringTable(static_cast<u64>(rows) * 2);
    _spells.resize(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        NDBC::Spell& spell = _spells[i];
        spell.Id = row.GetUInt32(0);
        spell.SpellCategory = row.GetUInt32(1);
        spell.DispelType = row.GetUInt32(2);
        spell.Mechanic = row.GetUInt32(3);
        spell.Attributes = row.GetUInt32(4);
        spell.AttributesExA = row.GetUInt32(5);
        spell.AttributesExB = row.GetUInt32(6);
        spell.AttributesExC = row.GetUInt32(7);
        spell.AttributesExD = row.GetUInt32(8);
        spell.AttributesExE = row.GetUInt32(9);
        spell.AttributesExF = row.GetUInt32(10);
        spell.AttributesExG = row.GetUInt32(11);
        spell.StanceMask = row.GetUInt64(12);         // This is 8 bytes wide, so skip 13
        spell.StanceExcludeMask = row.GetUInt64(14); // This is 8 bytes wide, so skip 15
        spell.Targets = row.GetUInt32(16);
        spell.TargetCreatureType = row.GetUInt32(17);
        spell.SpellFocusObject = row.GetUInt32(18);
        spell.FacingCasterFlags = row.GetUInt32(19);
        spell.CasterAuraState = row.GetUInt32(20);
        spell.TargetAuraState = row.GetUInt32(21);
        spell.CasterAuraStateNot = row.GetUInt32(22);
        spell.TargetAuraStateNot = row.GetUInt32(23);
        spell.CasterAuraSpell = row.GetUInt32(24);
        spell.TargetAuraSpell = row.GetUInt32(25);
        spell.ExcludeCasterAuraSpell = row.GetUInt32(26);
        spell.ExcludeTargetAuraSpell = row.GetUInt32(27);
        spell.CastingTimeIndex = row.GetUInt32(28);
        spell.RecoveryTime = row.GetUInt32(29);
        spell.CategoryRecoveryTime = row.GetUInt32(30);
        spell.InterruptFlags = row.GetUInt32(31);
        spell.AuraInterruptFlags = row.GetUInt32(32);
        spell.ChannelInterruptFlags = row.GetUInt32(33);
        spell.ProcFlags = row.GetUInt32(34);
        spell.ProcChance = row.GetUInt32(35);
        spell.ProcCharges = row.GetUInt32(36);
        spell.MaxLevel = row.GetUInt32(37);
        spell.BaseLevel = row.GetUInt32(38);
        spell.SpellLevel = row.GetUInt32(39);
        spell.DurationIndex = row.GetUInt32(40);
        spell.PowerType = row.GetInt32(41);
        spell.ManaCost = row.GetUInt32(42);
        spell.ManaCostPerlevel = row.GetUInt32(43);
        spell.ManaPerSecond = row.GetUInt32(44);
        spell.ManaPerSecondPerLevel = row.GetUInt32(45);
        spell.RangeIndex = row.GetUInt32(46);
        spell.Speed = row.GetFloat(47);
        spell.ModalNextSpell = row.GetUInt32(48);
        spell.StackAmount = row.GetUInt32(49);
        spell.Totem[0] = row.GetUInt32(50);
        spell.Totem[1] = row.GetUInt32(51);
        spell.Reagent[0] = row.GetInt32(52);
        spell.Reagent[1] = row.GetInt32(53);
        spell.Reagent[2] = row.GetInt32(54);
        spell.Reagent[3] = row.GetInt32(55);
        spell.Reagent[4] = row.GetInt32(56);
        spell.Reagent[5] = row.GetInt32(57);
        spell.Reagent[6] = row.GetInt32(58);
        spell.Reagent[7] = row.GetInt32(59);
        spell.ReagentCount[0] = row.GetInt32(60);
        spell.ReagentCount[1] = row.GetInt32(61);
        spell.ReagentCount[2] = row.GetInt32(62);
        spell.ReagentCount[3] = row.GetInt32(63);
        spell.ReagentCount[4] = row.GetInt32(64);
        spell.ReagentCount[5] = row.GetInt32(65);
        spell.ReagentCount[6] = row.GetInt32(66);
        spell.ReagentCount[7] = row.GetInt32(67);
        spell.EquippedItemClass = row.GetInt32(68);
        spell.EquippedItemSubClassMask = row.GetInt32(69);
        spell.EquippedItemInventoryTypeMask = row.GetInt32(70);
        spell.Effect[0] = row.GetUInt32(71);
        spell.Effect[1] = row.GetUInt32(72);
        spell.Effect[2] = row.GetUInt32(73);
        spell.EffectDieSides[0] = row.GetInt32(74);
        spell.EffectDieSides[1] = row.GetInt32(75);
        spell.EffectDieSides[2] = row.GetInt32(76);
        spell.EffectRealPointsPerLevel[0] = row.GetFloat(77);
        spell.EffectRealPointsPerLevel[1] = row.GetFloat(78);
        spell.EffectRealPointsPerLevel[2] = row.GetFloat(79);
        spell.EffectBasePoints[0] = row.GetInt32(80);
        spell.EffectBasePoints[1] = row.GetInt32(81);
        spell.EffectBasePoints[2] = row.GetInt32(82);
        spell.EffectMechanic[0] = row.GetUInt32(83);
        spell.EffectMechanic[1] = row.GetUInt32(84);
        spell.EffectMechanic[2] = row.GetUInt32(85);
        spell.EffectImplicitTargetA[0] = row.GetUInt32(86);
        spell.EffectImplicitTargetA[1] = row.GetUInt32(87);
        spell.EffectImplicitTargetA[2] = row.GetUInt32(88);
        spell.EffectImplicitTargetB[0] = row.GetUInt32(89);
        spell.EffectImplicitTargetB[1] = row.GetUInt32(90);
        spell.EffectImplicitTargetB[2] = row.GetUInt32(91);
        spell.EffectRadiusIndex[0] = row.GetUInt32(92);
        spell.EffectRadiusIndex[1] = row.GetUInt32(93);
        spell.EffectRadiusIndex[2] = row.GetUInt32(94);
        spell.EffectApplyAuraName[0] = row.GetUInt32(95);
        spell.EffectApplyAuraName[1] = row.GetUInt32(96);
        spell.EffectApplyAuraName[2] = row.GetUInt32(97);
        spell.EffectAuraPeriod[0] = row.GetUInt32(98);
        spell.EffectAuraPeriod[1] = row.GetUInt32(99);
        spell.EffectAuraPeriod[2] = row.GetUInt32(100);
        spell.EffectAmplitude[0] = row.GetFloat(101);
        spell.EffectAmplitude[1] = row.GetFloat(102);
        spell.EffectAmplitude[2] = row.GetFloat(103);
        spell.EffectChainTarget[0] = row.GetUInt32(104);
        spell.EffectChainTarget[1] = row.GetUInt32(105);
        spell.EffectChainTarget[2] = row.GetUInt32(106);
        spell.EffectItemType[0] = row.GetUInt32(107);
        spell.EffectItemType[1] = row.GetUInt32(108);
        spell.EffectItemType[2] = row.GetUInt32(109);
        spell.EffectMiscValueA[0] = row.GetInt32(110);
        spell.EffectMiscValueA[1] = row.GetInt32(111);
        spell.EffectMiscValueA[2] = row.GetInt32(112);
        spell.EffectMiscValueB[0] = row.GetInt32(113);
        spell.EffectMiscValueB[1] = row.GetInt32(114);
        spell.EffectMiscValueB[2] = row.GetInt32(115);
        spell.EffectTriggerSpell[0] = row.GetUInt32(116);
        spell.EffectTriggerSpell[1] = row.GetUInt32(117);
        spell.EffectTriggerSpell[2] = row.GetUInt32(118);
        spell.EffectPointsPerComboPoint[0] = row.GetFloat(119);
        spell.EffectPointsPerComboPoint[1] = row.GetFloat(120);
        spell.EffectPointsPerComboPoint[2] = row.GetFloat(121);
        spell.EffectSpellClassMaskA[0] = row.GetUInt32(122);
        spell.EffectSpellClassMaskA[1] = row.GetUInt32(123);
        spell.EffectSpellClassMaskA[2] = row.GetUInt32(124);
        spell.EffectSpellClassMaskB[0] = row.GetUInt32(125);
        spell.EffectSpellClassMaskB[1] = row.GetUInt32(126);
        spell.EffectSpellClassMaskB[2] = row.GetUInt32(127);
        spell.EffectSpellClassMaskC[0] = row.GetUInt32(128);
        spell.EffectSpellClassMaskC[1] = row.GetUInt32(129);
        spell.EffectSpellClassMaskC[2] = row.GetUInt32(130);
        spell.SpellVisual[0] = row.GetUInt32(131);
        spell.SpellVisual[1] = row.GetUInt32(132);
        spell.SpellIconID = row.GetUInt32(133);
        spell.ActiveIconID = row.GetUInt32(134);
        spell.SpellPriority = row.GetUInt32(135);
        spell.SpellName = GetLocaleStringIndexFromField(stringTable, row, 136);    // Skip 152 for SpellNameFlag
        spell.SpellSubText = GetLocaleStringIndexFromField(stringTable, row, 153); // Skip 169 for RankFlags
        // Skip 170 - 203 for unnecessary text stuff that we don't need
        spell.ManaCostPercentage = row.GetUInt32(204);
        spell.StartRecoveryCategory = row.GetUInt32(205);
        spell.StartRecoveryTime = row.GetUInt32(206);
        spell.MaxTargetLevel = row.GetUInt32(207);
        spell.SpellClassSet = row.GetUInt32(208);
        spell.SpellClassMask[0] = row.GetUInt32(209);
        spell.SpellClassMask[1] = row.GetUInt32(210);
        spell.SpellClassMask[2] = row.GetUInt32(211);
        spell.MaxTargets = row.GetUInt32(212);
        spell.DefenseType = row.GetUInt32(213);
        spell.PreventionType = row.GetUInt32(214);
        spell.StanceBarOrder = row.GetUInt32(215);
        spell.EffectChainAmplitude[0] = row.GetFloat(216);
        spell.EffectChainAmplitude[1] = row.GetFloat(217);
        spell.EffectChainAmplitude[2] = row.GetFloat(218);
        spell.MinFactionId = row.GetUInt32(219);
        spell.MinReputation = row.GetUInt32(220);
        spell.RequiredAuraVision = row.GetUInt32(221);
        spell.TotemCategory[0] = row.GetUInt32(222);
        spell.TotemCategory[1] = row.GetUInt32(223);
        spell.RequiredAreaId = row.GetInt32(224);
        spell.SchoolMask = row.GetUInt32(225);
        spell.RuneCostID = row.GetUInt32(226);
        spell.SpellMissileID = row.GetUInt32(227);
        spell.PowerDisplayID = row.GetInt32(228);
        spell.EffectBonusMultiplier[0] = row.GetFloat(229);
        spell.EffectBonusMultiplier[1] = row.GetFloat(230);
        spell.EffectBonusMultiplier[2] = row.GetFloat(231);
        spell.SpellDescriptionVariableID = row.GetUInt32(232);
        spell.SpellDifficultyID = row.GetUInt32(233);
    }

    return SaveDBCFile(globalData, "Spell.ndbc", _spells, stringTable);
}