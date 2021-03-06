/* Copyright (C) 2006 - 2013 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Instance_Obsidian_Sanctum
SD%Complete: 95%
SDComment:
SDCategory: Obsidian Sanctum
EndScriptData */

#include "precompiled.h"
#include "obsidian_sanctum.h"

/* Obsidian Sanctum encounters:
0 - Sartharion
*/

instance_obsidian_sanctum::instance_obsidian_sanctum(Map* pMap) : ScriptedInstance(pMap)
{
    Initialize();
};

void instance_obsidian_sanctum::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    m_uiAcolyteShadronGUID.Clear();
    m_uiAcolyteVesperonGUID.Clear();

    m_lTrashMobsGUIDlist.clear();
    m_lTrashMobsGUIDlist.clear();
    m_lEggsGuidList.clear();
    m_lWhelpsGuidList.clear();
    m_lBlazesGuidList.clear();
    m_lHitByVolcanoGuidList.clear();
}

void instance_obsidian_sanctum::OnCreatureCreate(Creature* pCreature)
{
    switch(pCreature->GetEntry())
    {
        case NPC_SARTHARION:
        //three dragons below set to active state once created.
        //we must expect bigger raid to encounter main boss, and then three dragons must be active due to grid differences
        case NPC_TENEBRON:
        case NPC_SHADRON:
        case NPC_VESPERON:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            pCreature->SetActiveObjectState(true);
            break;
        // trash mobs aggro when Sartharion is engaged
        case NPC_ONYX_BROOD_GENERAL:
        case NPC_ONYX_BLAZE_MISTRESS:
        case NPC_ONYX_FLIGHT_CAPTAIN:
        case NPC_ONYX_SANCTUM_GUARDIAN:
            m_lTrashMobsGUIDlist.push_back(pCreature->GetObjectGuid());
            break;
    }
}

void instance_obsidian_sanctum::SetAcolyteGuid(uint32 uiEntry, ObjectGuid guid)
{
    m_mNpcEntryGuidStore[uiEntry] = guid;
}

void instance_obsidian_sanctum::SetData(uint32 uiType, uint32 uiData)
{
    m_auiEncounter[uiType - 1] = uiData;
}

uint32 instance_obsidian_sanctum::GetData(uint32 uiType) const
{
    return m_auiEncounter[uiType - 1];
}

bool instance_obsidian_sanctum::IsEncounterInProgress() const
{
    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;

    return false;
}

bool instance_obsidian_sanctum::CheckAchievementCriteriaMeet(uint32 uiCriteriaId, Player const* pSource, Unit const* pTarget, uint32 uiMiscValue1) const
{
    switch (uiCriteriaId)
    {
        case ACHIEV_CRIT_ASSIST_10:
            if (instance->IsRegularDifficulty())
                return GetData(TYPE_SARTH_HARD_ONE) == DONE;
            break;
        case ACHIEV_CRIT_ASSIST_25:
            if (!instance->IsRegularDifficulty())
                return GetData(TYPE_SARTH_HARD_ONE) == DONE;
            break;
        case ACHIEV_CRIT_DUO_10:
            if (instance->IsRegularDifficulty())
                return GetData(TYPE_SARTH_HARD_TWO) == DONE;
            break;
        case ACHIEV_CRIT_DUO_25:
            if (!instance->IsRegularDifficulty())
                return GetData(TYPE_SARTH_HARD_TWO) == DONE;
            break;
        case ACHIEV_CRIT_ZONE_10:
            if (instance->IsRegularDifficulty())
                return GetData(TYPE_SARTH_HARD_THREE) == DONE;
            break;
        case ACHIEV_CRIT_ZONE_25:
            if (!instance->IsRegularDifficulty())
                return GetData(TYPE_SARTH_HARD_THREE) == DONE;
            break;
        case ACHIEV_CRIT_VOLCANO_10:
            if (instance->IsRegularDifficulty())
            {
                for (GuidList::const_iterator i = m_lHitByVolcanoGuidList.begin(); i != m_lHitByVolcanoGuidList.end(); i++)
                    if (pSource->GetObjectGuid() == *i)
                        return false;

                return true;
            }
            break;
        case ACHIEV_CRIT_VOLCANO_25:
            if (!instance->IsRegularDifficulty())
            {
                for (GuidList::const_iterator i = m_lHitByVolcanoGuidList.begin(); i != m_lHitByVolcanoGuidList.end(); i++)
                    if (pSource->GetObjectGuid() == *i)
                        return false;

                return true;
            }
            break;
    }

    return false;
}

bool instance_obsidian_sanctum::CheckConditionCriteriaMeet(Player const* pPlayer, uint32 uiInstanceConditionId, WorldObject const* pConditionSource, ConditionSource conditionSourceType) const
{
    switch (uiInstanceConditionId)
    {
        case INSTANCE_CONDITION_ID_HARD_MODE:               // Exactly one dragon alive on event start
            return GetData(TYPE_SARTH_HARD_ONE) == DONE;
        case INSTANCE_CONDITION_ID_HARD_MODE_2:             // Exactly two dragons alive on event start
            return GetData(TYPE_SARTH_HARD_TWO) == DONE;
        case INSTANCE_CONDITION_ID_HARD_MODE_3:             // All three dragons alive on event start
            return GetData(TYPE_SARTH_HARD_THREE) == DONE;
        default:
            break;
    }

    script_error_log("instance_obsidian_sanctum::CheckConditionCriteriaMeet called with unsupported Id %u. Called with param plr %s, src %s, condition source type %u",
                        uiInstanceConditionId, pPlayer ? pPlayer->GetGuidStr().c_str() : "NULL", pConditionSource ? pConditionSource->GetGuidStr().c_str() : "NULL", conditionSourceType);
    return false;
}

InstanceData* GetInstanceData_instance_obsidian_sanctum(Map* pMap)
{
    return new instance_obsidian_sanctum(pMap);
}

void AddSC_instance_obsidian_sanctum()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_obsidian_sanctum";
    newscript->GetInstanceData = GetInstanceData_instance_obsidian_sanctum;
    newscript->RegisterSelf();
}
