/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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
 SDName: Elwynn_Forest
 SD%Complete: 0
 SDComment: Placeholder
 SDCategory: Elwynn Forest
 EndScriptData */

 /* ContentData
 EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/BaseAI/CreatureAI.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{

	SPELL_NIBLE_REFLEXES = 6433,                 // removed after phase 1
	SPELL_SMITE_STOMP = 6432,
	SPELL_FLAME_BREATH = 9573,
	SPELL_TAIL_SWIPE = 15847,

	PHASE_1 = 1,
	PHASE_2 = 2,
	PHASE_3 = 3,
};


struct hoggerAI : public ScriptedAI
{
	hoggerAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

	uint32 m_uiResetTimer;
	// Let's do phases
    uint32 m_uiPhase;
	uint32 m_uiTransformTimer;
    uint32 m_uiBreathTimer = 8000;
    bool m_hasTransformed;

	void Reset() override
    {
        m_uiPhase = PHASE_1;
        m_uiBreathTimer = 8000;
        m_hasTransformed = false;
    }

    void WatchAndChangeToPhase2()
    {
        if (m_creature->GetHealthPercent() < 66.0f && m_uiPhase == PHASE_1)
        {
            DoCastSpellIfCan(m_creature, SPELL_NIBLE_REFLEXES, CAST_AURA_NOT_PRESENT);
            m_uiPhase = PHASE_2;
        }
    }

    bool TransformAndChangeToPhase3()
    {
        if (m_creature->GetHealthPercent() < 20.0f && !m_hasTransformed)
        {
            m_creature->SetDisplayId(6374);
            m_creature->SetLevel(15);
            m_creature->SetMaxHealth(2000);
            m_creature->SetHealth(300);

            m_creature->MonsterSay(
                "Heh... heh... you thought to corner a WRETCHED GNOLL? HOW AMUSING. THIS MASK OF FUR AND FILTH WAS ONLY A GAME. NOW TREMBLE, MORTALS, BEFORE THE TRUE POWER OF THE "
                "DRAGON WHO WILL INVADE YOUR PRECIOUS STORMWIND!",
                LANG_UNIVERSAL);

            m_uiPhase = PHASE_3;
            m_hasTransformed = true;
            return true;
        }
        return false;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        switch (m_uiPhase)
        {
            case PHASE_1: WatchAndChangeToPhase2(); break;

            case PHASE_2:
                if (m_creature->GetHealthPercent() < 20.0f)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_SMITE_STOMP, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT) == CAST_OK)
                    {
                        m_creature->RemoveAurasDueToSpell(SPELL_NIBLE_REFLEXES);
                        TransformAndChangeToPhase3();
                    }
                }
                break;

            case PHASE_3:
                if (m_hasTransformed)
                {
                    if (m_uiBreathTimer <= uiDiff)
                    {
                        if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_FLAME_BREATH) == CAST_OK)
                            m_uiBreathTimer = 8000;
                    }
                    else
                        m_uiBreathTimer -= uiDiff;
                }
                break;
        }
        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_Hogger(Creature* pCreature)
{
	return new hoggerAI(pCreature);
}


enum
{
    NPC_FORLORN_SPIRIT = 2044,
};

// Marshal Haggard's Chest ID: 1562
struct go_marshal_haggards_chestAI : public GameObjectAI
{
    go_marshal_haggards_chestAI(GameObject* go) : GameObjectAI(go) {}

    void OnLootStateChange(Unit* user)
    {
        if (m_go->GetLootState() == GO_JUST_DEACTIVATED)
        {
            Creature* forlornSpirit = GetClosestCreatureWithEntry(m_go, NPC_FORLORN_SPIRIT, 20.0f);
            if (!forlornSpirit || !forlornSpirit->IsAlive())
            {
                if (Creature* spirit = m_go->SummonCreature(NPC_FORLORN_SPIRIT, -9553.5127f, -1430.542f, 62.377f, 4.6077f, TEMPSPAWN_TIMED_OOC_DESPAWN, 300000, true, false))
                {
                    spirit->AI()->AttackStart(user);
                }
            }
            
        }
    }
};

void AddSC_elwynn_forest()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_hogger";
    pNewScript->GetAI = &GetAI_Hogger;
    pNewScript->RegisterSelf();
}
