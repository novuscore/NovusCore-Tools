/*
# MIT License

# Copyright(c) 2018-2019 NovusCore

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
*/
#pragma once
#include <NovusTypes.h>
#include <string>


constexpr i32 NDBC_TOKEN = 1313096259;

struct NDBCHeader
{
    u32 token = NDBC_TOKEN;
    u32 version = 1;
};

struct DBCMap
{
    u32 Id;
    u32 Name;
    u32 InternalName;
    u32 InstanceType;
    u32 Flags;
    u32 Expansion;
    u32 MaxPlayers;
};

struct DBCLight
{
    u32 id;
    u32 mapId; // This is a reference to Map.dbc
    vec3 position; // X, Y and Z position for the center of the Light(Skybox) Sphere.
    vec2 fallOff; // X == Start (Inner Radius), Y == End (Outer Radius)
    i32 paramsClear; // Used in clear weather
    i32 paramsClearInWater; // Used in clear weather while being underwater.
    i32 paramsStorm; // Used in storm like weather.
    i32 paramsStormInWater; // Used in storm like weather while being underwater.
    i32 paramsDeath; // Appears to be hardcoded in the client, but maybe not.
    

    // These are only used in WoTLK
    i32 paramUnk1;
    i32 paramUnk2;
    i32 paramUnk3;
};

struct DBCLightSkybox
{
    u32 id;
    u32 modelPath;
    u32 flags;
};

struct DBCCreatureDisplayInfo
{
    u32 id;
    u32 modelId; // Reference into CreatureModelData
    u32 soundId; // Reference into CreatureSoundData
    u32 extraDisplayInfoId; // Reference into CreatureDisplayInfoExtra
    f32 scale;
    u32 opacity;
    u32 texture1;
    u32 texture2;
    u32 texture3;
    u32 portraitTextureName;
    u32 bloodLevelId; // Reference into UnitBloodLevels
    u32 bloodId; // Reference into UnitBlood
    u32 npcSoundsId; // Reference into NPCSounds
    u32 particlesId; // Reference into ParticleColor
    u32 creatureGeosetData;
    u32 objectEffectPackageId; // Reference into ObjectEffectPackage
};

struct DBCCreatureModelData
{
    u32 id;
    u32 flags;
    u32 modelPath;

    u32 sizeClass;
    f32 modelScale;

    u32 bloodLevelId; // Reference into UnitBloodLevels

    u32 footPrintId; // Reference into FootprintTextures
    f32 footPrintTextureLength;
    f32 footprintTextureWidth;
    f32 footprintParticleScale;
    u32 foleyMaterialId;
    u32 footstepShakeSize; // Reference into CameraShakes
    u32 deathThudShakeSize; // Reference into CameraShakes
    u32 soundDataId; // Reference into CreatureSoundData

    f32 collisionWidth;
    f32 collisionHeight;
    f32 mountHeight;

    vec3 geoBoxMin;
    vec3 geoBoxMax;

    f32 worldEffectScale;
    f32 attachedEffectScale;

    f32 missileCollisionRadius;
    f32 missileCollisionPush;
    f32 missileCollisionRaise;
};

struct DBCEmotesText
{
    u32 Id;
    u32 InternalName;
    u32 AnimationId;
};

struct DBCSpell
{
    u32 Id;
    u32 SpellCategory;
    u32 DispelType;
    u32 Mechanic;
    u32 Attributes;
    u32 AttributesExA;
    u32 AttributesExB;
    u32 AttributesExC;
    u32 AttributesExD;
    u32 AttributesExE;
    u32 AttributesExF;
    u32 AttributesExG;
    u64 StanceMask;
    u64 StanceExcludeMask;
    u32 Targets;
    u32 TargetCreatureType;
    u32 SpellFocusObject;
    u32 FacingCasterFlags;
    u32 CasterAuraState;
    u32 TargetAuraState;
    u32 CasterAuraStateNot;
    u32 TargetAuraStateNot;
    u32 CasterAuraSpell;
    u32 TargetAuraSpell;
    u32 ExcludeCasterAuraSpell;
    u32 ExcludeTargetAuraSpell;
    u32 CastingTimeIndex;
    u32 RecoveryTime;
    u32 CategoryRecoveryTime;
    u32 InterruptFlags;
    u32 AuraInterruptFlags;
    u32 ChannelInterruptFlags;
    u32 ProcFlags;
    u32 ProcChance;
    u32 ProcCharges;
    u32 MaxLevel;
    u32 BaseLevel;
    u32 SpellLevel;
    u32 DurationIndex;
    i32 PowerType;
    u32 ManaCost;
    u32 ManaCostPerlevel;
    u32 ManaPerSecond;
    u32 ManaPerSecondPerLevel;
    u32 RangeIndex;
    f32 Speed;
    u32 ModalNextSpell;
    u32 StackAmount;
    u32 Totem[2];
    i32 Reagent[8];
    i32 ReagentCount[8];
    i32 EquippedItemClass;
    i32 EquippedItemSubClassMask;
    i32 EquippedItemInventoryTypeMask;
    u32 Effect[3];
    i32 EffectDieSides[3];
    f32 EffectRealPointsPerLevel[3];
    i32 EffectBasePoints[3];
    u32 EffectMechanic[3];
    u32 EffectImplicitTargetA[3];
    u32 EffectImplicitTargetB[3];
    u32 EffectRadiusIndex[3];
    u32 EffectApplyAuraName[3];
    u32 EffectAuraPeriod[3];
    f32 EffectAmplitude[3];
    u32 EffectChainTarget[3];
    u32 EffectItemType[3];
    i32 EffectMiscValueA[3];
    i32 EffectMiscValueB[3];
    u32 EffectTriggerSpell[3];
    f32 EffectPointsPerComboPoint[3];
    u32 EffectSpellClassMaskA[3];
    u32 EffectSpellClassMaskB[3];
    u32 EffectSpellClassMaskC[3];
    u32 SpellVisual[2];
    u32 SpellIconID;
    u32 ActiveIconID;
    u32 SpellPriority;  
    u32 SpellName;
    u32 SpellSubText;
    u32 ManaCostPercentage;
    u32 StartRecoveryCategory;
    u32 StartRecoveryTime;
    u32 MaxTargetLevel;
    u32 SpellClassSet;
    u32 SpellClassMask[3];
    u32 MaxTargets;
    u32 DefenseType;
    u32 PreventionType;
    u32 StanceBarOrder;
    f32 EffectChainAmplitude[3];
    u32 MinFactionId;
    u32 MinReputation;
    u32 RequiredAuraVision;
    u32 TotemCategory[2];
    i32 RequiredAreaId;
    u32 SchoolMask;
    u32 RuneCostID;
    u32 SpellMissileID;
    i32 PowerDisplayID;
    f32 EffectBonusMultiplier[3];
    u32 SpellDescriptionVariableID;
    u32 SpellDifficultyID;
};