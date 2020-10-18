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
    LoadLiquidTypes(globalData, mpqLoader, dbcReader);
    LoadLiquidMaterials(globalData, mpqLoader, dbcReader);
    LoadCreatureModelData(globalData, mpqLoader, dbcReader);
    LoadCreatureDisplayInfo(globalData, mpqLoader, dbcReader);
    LoadEmotesText(globalData, mpqLoader, dbcReader);
    LoadSpell(globalData, mpqLoader, dbcReader);

    CreateDBCStringTableFile(globalData);
}

constexpr u32 InvalidNameIndex = std::numeric_limits<u32>().max();
u32 DBCExtractor::GetStringIndexFromField(DBCReader::DBCRow& row, u32 field)
{
    std::string value = row.GetString(field);

    if (value.length() == 0)
        return InvalidNameIndex;

    if (StringUtils::EndsWith(value, ".mdx"))
        value = value.substr(0, value.length() - 4) + ".nm2";

    if (StringUtils::EndsWith(value, ".m2"))
        value = value.substr(0, value.length() - 3) + ".nm2";

    return _dbcStringTable.AddString(value);
}

u32 DBCExtractor::GetLocaleStringIndexFromField(DBCReader::DBCRow& row, u32 field)
{
    std::string value = "";
    for (u8 i = 0; i < 16; i++)
    {
        value = row.GetString(field + i);

        if (value.length() != 0)
            break;
    }

    if (value.length() == 0)
        return InvalidNameIndex;

    if (StringUtils::EndsWith(value, ".mdx"))
        value = value.substr(0, value.length() - 4) + ".nm2";

    if (StringUtils::EndsWith(value, ".m2"))
        value = value.substr(0, value.length() - 3) + ".nm2";

    return _dbcStringTable.AddString(value);
}

bool DBCExtractor::LoadMap(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    std::shared_ptr<Bytebuffer> file = mpqLoader->GetFile("DBFilesClient\\Map.dbc");
    if (!file)
    {
        NC_LOG_ERROR("Failed to load Map.dbc");
        return false;
    }

    NC_LOG_MESSAGE("Loading Map.dbc...");
    if (dbcReader->Load(file) != 0)
        return false;

    u32 rows = dbcReader->GetNumRows();
    if (rows == 0)
        return false;

    _maps.reserve(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        DBC::Map& map = _maps.emplace_back();
        map.Id = row.GetUInt32(0);
        map.InternalName = GetStringIndexFromField(row, 1);
        map.InstanceType = row.GetUInt32(2);
        map.Flags = row.GetUInt32(3);
        map.Name = GetLocaleStringIndexFromField(row, 5);
        map.Expansion = row.GetUInt32(63);
        map.MaxPlayers = row.GetUInt32(65);
    }

    fs::path outputPath = globalData->ndbcPath / "Maps.ndbc";
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        NC_LOG_ERROR("Failed to create dbc file. Check admin permissions");
        return false;
    }

    DBC::NDBCHeader header;
    output.write(reinterpret_cast<char const*>(&header), sizeof(header)); // Write NDBC Header
    output.write(reinterpret_cast<char const*>(&rows), sizeof(u32)); // Write number of maps
    output.write(reinterpret_cast<char const*>(_maps.data()), rows * sizeof(DBC::Map)); // Write maps

    output.close();

    return true;
}
bool DBCExtractor::LoadLiquidTypes(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    std::shared_ptr<Bytebuffer> file = mpqLoader->GetFile("DBFilesClient\\LiquidType.dbc");
    if (!file)
    {
        NC_LOG_ERROR("Failed to load LiquidType.dbc");
        return false;
    }

    NC_LOG_MESSAGE("Loading LiquidType.dbc...");
    if (dbcReader->Load(file) != 0)
        return false;

    u32 rows = dbcReader->GetNumRows();
    if (rows == 0)
        return false;

    _liquidTypes.reserve(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        DBC::LiquidType& liquidType = _liquidTypes.emplace_back();
        liquidType.id = row.GetUInt32(0);
        liquidType.name = GetStringIndexFromField(row, 1);
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

    fs::path outputPath = globalData->ndbcPath / "LiquidTypes.ndbc";
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        NC_LOG_ERROR("Failed to create dbc file. Check admin permissions");
        return false;
    }

    DBC::NDBCHeader header;
    output.write(reinterpret_cast<char const*>(&header), sizeof(header)); // Write NDBC Header
    output.write(reinterpret_cast<char const*>(&rows), sizeof(u32)); // Write number of liquid types
    output.write(reinterpret_cast<char const*>(_liquidTypes.data()), rows * sizeof(DBC::LiquidType)); // Write liquid types

    output.close();

    return true;
}
bool DBCExtractor::LoadLiquidMaterials(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    std::shared_ptr<Bytebuffer> file = mpqLoader->GetFile("DBFilesClient\\LiquidMaterial.dbc");
    if (!file)
    {
        NC_LOG_ERROR("Failed to load LiquidMaterial.dbc");
        return false;
    }

    NC_LOG_MESSAGE("Loading LiquidMaterial.dbc...");
    if (dbcReader->Load(file) != 0)
        return false;

    u32 rows = dbcReader->GetNumRows();
    if (rows == 0)
        return false;

    _liquidMaterials.reserve(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        DBC::LiquidMaterial& liquidMaterial = _liquidMaterials.emplace_back();
        liquidMaterial.id = row.GetUInt32(0);
        liquidMaterial.liquidVertexFormat = row.GetUInt32(1);
        liquidMaterial.flags = row.GetUInt32(2);
    }

    fs::path outputPath = globalData->ndbcPath / "LiquidMaterials.ndbc";
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        NC_LOG_ERROR("Failed to create dbc file. Check admin permissions");
        return false;
    }

    DBC::NDBCHeader header;
    output.write(reinterpret_cast<char const*>(&header), sizeof(header)); // Write NDBC Header
    output.write(reinterpret_cast<char const*>(&rows), sizeof(u32)); // Write number of liquid materials
    output.write(reinterpret_cast<char const*>(_liquidMaterials.data()), rows * sizeof(DBC::LiquidMaterial)); // Write liquid materials

    output.close();

    return true;
}
bool DBCExtractor::LoadCreatureModelData(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    std::shared_ptr<Bytebuffer> file = mpqLoader->GetFile("DBFilesClient\\CreatureModelData.dbc");
    if (!file)
    {
        NC_LOG_ERROR("Failed to load CreatureModelData.dbc");
        return false;
    }

    NC_LOG_MESSAGE("Loading CreatureModelData.dbc...");
    if (dbcReader->Load(file) != 0)
        return false;

    u32 rows = dbcReader->GetNumRows();
    if (rows == 0)
        return false;

    _creatureModelDatas.reserve(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);
        {
            DBC::CreatureModelData& creatureModelData = _creatureModelDatas.emplace_back();
            creatureModelData.id = row.GetUInt32(0);
            creatureModelData.flags = row.GetUInt32(1);
            creatureModelData.modelPath = GetStringIndexFromField(row, 2);
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

    fs::path outputPath = globalData->ndbcPath / "CreatureModelData.ndbc";
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        NC_LOG_ERROR("Failed to create dbc file. Check admin permissions");
        return false;
    }

    DBC::NDBCHeader header;
    output.write(reinterpret_cast<char const*>(&header), sizeof(header)); // Write NDBC Header
    output.write(reinterpret_cast<char const*>(&rows), sizeof(u32)); // Write number of DBCCreatureModelDatas
    output.write(reinterpret_cast<char const*>(_creatureModelDatas.data()), rows * sizeof(DBC::CreatureModelData)); // Write DBCCreatureModelDatas

    output.close();

    return true;
}
bool DBCExtractor::LoadCreatureDisplayInfo(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    std::shared_ptr<Bytebuffer> file = mpqLoader->GetFile("DBFilesClient\\CreatureDisplayInfo.dbc");
    if (!file)
    {
        NC_LOG_ERROR("Failed to load CreatureDisplayInfo.dbc");
        return false;
    }

    NC_LOG_MESSAGE("Loading CreatureDisplayInfo.dbc...");
    if (dbcReader->Load(file) != 0)
        return false;

    u32 rows = dbcReader->GetNumRows();
    if (rows == 0)
        return false;

    _creatureDisplayInfos.reserve(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);
        {
            DBC::CreatureDisplayInfo& creatureDisplayInfo = _creatureDisplayInfos.emplace_back();
            creatureDisplayInfo.id = row.GetUInt32(0);
            creatureDisplayInfo.modelId = row.GetUInt32(1);
            creatureDisplayInfo.soundId = row.GetUInt32(2);
            creatureDisplayInfo.extraDisplayInfoId = row.GetUInt32(3);
            creatureDisplayInfo.scale = row.GetFloat(4);
            creatureDisplayInfo.opacity = row.GetUInt32(5);

            creatureDisplayInfo.texture1 = GetStringIndexFromField(row, 6);
            creatureDisplayInfo.texture2 = GetStringIndexFromField(row, 7);
            creatureDisplayInfo.texture3 = GetStringIndexFromField(row, 8);
            creatureDisplayInfo.portraitTextureName = GetStringIndexFromField(row, 9);

            creatureDisplayInfo.bloodLevelId = row.GetUInt32(10);
            creatureDisplayInfo.bloodId = row.GetUInt32(11);
            creatureDisplayInfo.npcSoundsId = row.GetUInt32(12);
            creatureDisplayInfo.particlesId = row.GetUInt32(13);
            creatureDisplayInfo.creatureGeosetData = row.GetUInt32(14);
            creatureDisplayInfo.objectEffectPackageId = row.GetUInt32(15);
        }
    }

    fs::path outputPath = globalData->ndbcPath / "CreatureDisplayInfo.ndbc";
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        NC_LOG_ERROR("Failed to create dbc file. Check admin permissions");
        return false;
    }

    DBC::NDBCHeader header;
    output.write(reinterpret_cast<char const*>(&header), sizeof(header)); // Write NDBC Header
    output.write(reinterpret_cast<char const*>(&rows), sizeof(u32)); // Write number of CreatureDisplayInfos
    output.write(reinterpret_cast<char const*>(_creatureDisplayInfos.data()), rows * sizeof(DBC::CreatureDisplayInfo)); // Write CreatureDisplayInfos

    output.close();

    return true;
}
bool DBCExtractor::LoadEmotesText(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    std::shared_ptr<Bytebuffer> file = mpqLoader->GetFile("DBFilesClient\\EmotesText.dbc");
    if (!file)
    {
        NC_LOG_ERROR("Failed to load EmotesText.dbc");
        return false;
    }

    NC_LOG_MESSAGE("Loading EmotesText.dbc...");

    if (dbcReader->Load(file) != 0)
        return false;

    u32 rows = dbcReader->GetNumRows();
    if (rows == 0)
        return false;

    _emotesTexts.reserve(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        DBC::EmotesText& emoteText = _emotesTexts.emplace_back();
        emoteText.Id = row.GetUInt32(0);
        emoteText.InternalName = GetStringIndexFromField(row, 1);
        emoteText.AnimationId = row.GetUInt32(2);
    }

    fs::path outputPath = globalData->ndbcPath / "EmotesText.ndbc";
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        NC_LOG_ERROR("Failed to create dbc file. Check admin permissions");
        return false;
    }

    DBC::NDBCHeader header;
    output.write(reinterpret_cast<char const*>(&header), sizeof(header)); // Write NDBC Header
    output.write(reinterpret_cast<char const*>(&rows), sizeof(u32)); // Write number of DBCEmotesTexts
    output.write(reinterpret_cast<char const*>(_emotesTexts.data()), rows * sizeof(DBC::EmotesText)); // Write DBCEmotesTexts

    output.close();

    return true;
}
bool DBCExtractor::LoadSpell(std::shared_ptr<GlobalData> globalData, std::shared_ptr<MPQLoader> mpqLoader, std::shared_ptr<DBCReader> dbcReader)
{
    std::shared_ptr<Bytebuffer> file = mpqLoader->GetFile("DBFilesClient\\Spell.dbc");
    if (!file)
    {
        NC_LOG_ERROR("Failed to load Spell.dbc");
        return false;
    }

    NC_LOG_MESSAGE("Loading Spell.dbc...");
    if (dbcReader->Load(file) != 0)
        return false;

    u32 rows = dbcReader->GetNumRows();
    if (rows == 0)
        return false;

    _spells.reserve(rows);

    for (u32 i = 0; i < rows; i++)
    {
        auto row = dbcReader->GetRow(i);

        DBC::Spell& spell = _spells.emplace_back();
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
        spell.SpellName = GetLocaleStringIndexFromField(row, 136);    // Skip 152 for SpellNameFlag
        spell.SpellSubText = GetLocaleStringIndexFromField(row, 153); // Skip 169 for RankFlags
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

    fs::path outputPath = globalData->ndbcPath / "Spell.ndbc";
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        NC_LOG_ERROR("Failed to create dbc file. Check admin permissions");
        return false;
    }

    DBC::NDBCHeader header;
    output.write(reinterpret_cast<char const*>(&header), sizeof(header)); // Write NDBC Header
    output.write(reinterpret_cast<char const*>(&rows), sizeof(u32)); // Write number of DBCSpells
    output.write(reinterpret_cast<char const*>(_spells.data()), rows * sizeof(DBC::Spell)); // Write DBCSpells

    output.close();

    return true;
}

void DBCExtractor::CreateDBCStringTableFile(std::shared_ptr<GlobalData> globalData)
{
    fs::path outputPath = globalData->ndbcPath / "NDBCStringTable.nst";

    // Create a file
    std::ofstream output(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!output)
    {
        printf("Failed to create DBC StringTable file. Check admin permissions\n");
        return;
    }

    // Serialize and write our StringTable to the file
    std::shared_ptr<Bytebuffer> stringTableByteBuffer = Bytebuffer::Borrow<8388608>();
    _dbcStringTable.Serialize(stringTableByteBuffer.get());
    output.write(reinterpret_cast<char const*>(stringTableByteBuffer->GetDataPointer()), stringTableByteBuffer->writtenData);

    output.close();
}
