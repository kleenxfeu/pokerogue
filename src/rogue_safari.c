#include "global.h"
#include "constants/items.h"

#include "event_data.h"
#include "random.h"
#include "string_util.h"

#include "rogue_controller.h"
#include "rogue_save.h"
#include "rogue_safari.h"

#define INVALID_SAFARI_MON_IDX 255

struct SafariData
{
    u8 spawnIndex;
    u8 pendingBattleIdx;
    u8 slotToIndexMap[FOLLOWMON_MAX_SPAWN_SLOTS];
};

static EWRAM_DATA struct SafariData sSafariData = {0};

static void ZeroSafariMon(struct RogueSafariMon* mon);
static u8 AllocSafariMonSlot();
static u8 FreeSafariMonSlotCount();

void RogueSafari_PushMon(struct Pokemon* mon)
{
    if(!mon->rogueExtraData.isSafariIllegal)
    {
        RogueSafari_PushBoxMon(&mon->box);

        // Just in case we somehow try to add this mon twice, don't
        mon->rogueExtraData.isSafariIllegal = TRUE;
    }
}

void RogueSafari_PushBoxMon(struct BoxPokemon* monToCopy)
{
    u8 index = AllocSafariMonSlot();
    struct RogueSafariMon* writeMon = &gRogueSaveBlock->safariMons[index];

    ZeroSafariMon(writeMon);
    RogueSafari_CopyToSafariMon(monToCopy, writeMon);

    writeMon->priorityCounter = 1;

    if(writeMon->shinyFlag)
    {
        // Shinies will last much longer than regular mons
        writeMon->priorityCounter += 10;
    }

    // TODO - Handle legends?
    // TODO - Track if mon used in major fights (or lots of fights)
}

static void ZeroSafariMon(struct RogueSafariMon* mon)
{
    memset(mon, 0, sizeof(struct RogueSafariMon));
    mon->nickname[0] = 0xFF;
}

void RogueSafari_ResetSpawns()
{
    u8 i;

    sSafariData.spawnIndex = Random() % ARRAY_COUNT(gRogueSaveBlock->safariMons);
    sSafariData.pendingBattleIdx = INVALID_SAFARI_MON_IDX;

    for(i = 0; i < ARRAY_COUNT(sSafariData.slotToIndexMap); ++i)
    {
        sSafariData.slotToIndexMap[i] = INVALID_SAFARI_MON_IDX;
    }
}

#define COPY_MON_DATA(param, data) \
    temp = fromMon->param; \
    SetBoxMonData(toMon, data, &temp);

void RogueSafari_CopyFromSafariMon(struct RogueSafariMon* fromMon, struct BoxPokemon* toMon)
{
    u32 temp;

    COPY_MON_DATA(species, MON_DATA_SPECIES);

    COPY_MON_DATA(hpIV, MON_DATA_HP_IV);
    COPY_MON_DATA(attackIV, MON_DATA_ATK_IV);
    COPY_MON_DATA(defenseIV, MON_DATA_DEF_IV);
    COPY_MON_DATA(speedIV, MON_DATA_SPEED_IV);
    COPY_MON_DATA(spAttackIV, MON_DATA_SPATK_IV);
    COPY_MON_DATA(spDefenseIV, MON_DATA_SPDEF_IV);

    COPY_MON_DATA(pokeball, MON_DATA_POKEBALL);
    COPY_MON_DATA(abilityNum, MON_DATA_ABILITY_NUM);
    COPY_MON_DATA(genderFlag, MON_DATA_GENDER_FLAG);
    COPY_MON_DATA(shinyFlag, MON_DATA_IS_SHINY);

    SetBoxMonData(toMon, MON_DATA_NICKNAME, fromMon->nickname);
    COPY_MON_DATA(nature, MON_DATA_NATURE);
}

#undef COPY_MON_DATA

#define COPY_MON_DATA(param, data) \
    temp = GetBoxMonData(fromMon, data, NULL); \
    toMon->param = temp

void RogueSafari_CopyToSafariMon(struct BoxPokemon* fromMon, struct RogueSafariMon* toMon)
{
    u32 temp;

    COPY_MON_DATA(species, MON_DATA_SPECIES);

    COPY_MON_DATA(hpIV, MON_DATA_HP_IV);
    COPY_MON_DATA(attackIV, MON_DATA_ATK_IV);
    COPY_MON_DATA(defenseIV, MON_DATA_DEF_IV);
    COPY_MON_DATA(speedIV, MON_DATA_SPEED_IV);
    COPY_MON_DATA(spAttackIV, MON_DATA_SPATK_IV);
    COPY_MON_DATA(spDefenseIV, MON_DATA_SPDEF_IV);

    COPY_MON_DATA(pokeball, MON_DATA_POKEBALL);
    COPY_MON_DATA(abilityNum, MON_DATA_ABILITY_NUM);
    COPY_MON_DATA(genderFlag, MON_DATA_GENDER_FLAG);
    COPY_MON_DATA(shinyFlag, MON_DATA_IS_SHINY);

    GetBoxMonData(fromMon, MON_DATA_NICKNAME, toMon->nickname);

    // Use original nature
    temp = GetBoxMonData(fromMon, MON_DATA_PERSONALITY);
    toMon->nature = GetNatureFromPersonality(temp);
}

#undef COPY_MON_DATA

static bool8 IsMonAlreadySpawned(u8 safariIndex)
{
    u8 i;

    for(i = 0; i < ARRAY_COUNT(sSafariData.slotToIndexMap); ++i)
    {
        if(sSafariData.slotToIndexMap[i] == safariIndex)
            return TRUE;
    }

    return FALSE;
}

struct RogueSafariMon* RogueSafari_ChooseSafariMonForSlot(u8 slot)
{
    u8 i;

    for(i = 0; i < ARRAY_COUNT(gRogueSaveBlock->safariMons); ++i)
    {
        sSafariData.spawnIndex = (sSafariData.spawnIndex + 1) % ARRAY_COUNT(gRogueSaveBlock->safariMons);

        if(gRogueSaveBlock->safariMons[sSafariData.spawnIndex].species != SPECIES_NONE && !IsMonAlreadySpawned(sSafariData.spawnIndex))
        {
            sSafariData.slotToIndexMap[slot] = sSafariData.spawnIndex;
            return &gRogueSaveBlock->safariMons[sSafariData.spawnIndex];
        }
    }

    // Couldn't find a mon to spawn
    return NULL;
}

void RogueSafari_RemoveMonFromSlot(u8 slot)
{
    if(slot < ARRAY_COUNT(sSafariData.slotToIndexMap))
    {
        sSafariData.slotToIndexMap[slot] = INVALID_SAFARI_MON_IDX;
    }
}

struct RogueSafariMon* RogueSafari_GetSafariMonAt(u8 index)
{
    if(index < ARRAY_COUNT(sSafariData.slotToIndexMap))
    {
        u8 safariIndex = sSafariData.slotToIndexMap[index];

        if(safariIndex < ARRAY_COUNT(gRogueSaveBlock->safariMons))
            return &gRogueSaveBlock->safariMons[safariIndex];
    }

    return NULL;
}

void RogueSafari_ClearSafariMonAtIdx(u8 index)
{
    if(index < ARRAY_COUNT(gRogueSaveBlock->safariMons))
    {
        u8 i;

        for(i = 0; i < ARRAY_COUNT(sSafariData.slotToIndexMap); ++i)
        {
            if(sSafariData.slotToIndexMap[i] == index)
                sSafariData.slotToIndexMap[i] = INVALID_SAFARI_MON_IDX;
        }

        ZeroSafariMon(&gRogueSaveBlock->safariMons[index]);
        
        if(sSafariData.pendingBattleIdx == index)
            sSafariData.pendingBattleIdx = INVALID_SAFARI_MON_IDX;
    }
}

void RogueSafari_EnqueueBattleMon(u8 slot)
{
    AGB_ASSERT(slot < ARRAY_COUNT(sSafariData.slotToIndexMap));

    if(slot < ARRAY_COUNT(sSafariData.slotToIndexMap))
    {
        sSafariData.pendingBattleIdx = sSafariData.slotToIndexMap[slot];
    }
}

u8 RogueSafari_GetPendingBattleMonIdx()
{
    return sSafariData.pendingBattleIdx;
}

struct RogueSafariMon* RogueSafari_GetPendingBattleMon()
{
    u8 safariIndex = sSafariData.pendingBattleIdx;

    if(safariIndex < ARRAY_COUNT(gRogueSaveBlock->safariMons))
        return &gRogueSaveBlock->safariMons[safariIndex];

    return NULL;
}

static u8 AllocSafariMonSlot()
{
    u8 i;

    for(i = 0; i < ARRAY_COUNT(gRogueSaveBlock->safariMons); ++i)
    {
        if(gRogueSaveBlock->safariMons[i].species == SPECIES_NONE)
        {
            // There is a free slot here
            return i;
        }
    }

    // If we got here, that means we need to get rid of a mon and take it's slot
    {
        u8 lowestPriority = 255;
        u16 idx, offset;

        // Count down priorities
        for(i = 0; i < ARRAY_COUNT(gRogueSaveBlock->safariMons); ++i)
        {
            if(gRogueSaveBlock->safariMons[i].priorityCounter != 0)
                --gRogueSaveBlock->safariMons[i].priorityCounter;

            // Keep track of lowest priority in case there isn't a free slot
            lowestPriority = min(lowestPriority, gRogueSaveBlock->safariMons[i].priorityCounter); 
        }

        offset = Random();

        // Find first mon of priority and give back it's slot
        for(i = 0; i < ARRAY_COUNT(gRogueSaveBlock->safariMons); ++i)
        {
            idx = (offset + i) % ARRAY_COUNT(gRogueSaveBlock->safariMons);

            if(gRogueSaveBlock->safariMons[idx].priorityCounter == lowestPriority)
            {
                return idx;
            }
        }
    }

    // Should never reach here
    AGB_ASSERT(FALSE);
    return 0;
}

static u8 FreeSafariMonSlotCount()
{
    u8 i;
    u8 count = 0;

    for(i = 0; i < ARRAY_COUNT(gRogueSaveBlock->safariMons); ++i)
    {
        if(gRogueSaveBlock->safariMons[i].species != SPECIES_NONE)
            ++count;
    }

    return count;
}

u16 RogueSafari_GetActivePokeballType()
{
    u16 itemId = VarGet(VAR_ROGUE_SAFARI_BALL_TYPE);

    if(itemId >= FIRST_BALL && itemId <= LAST_BALL)
        return itemId;

    return ITEM_POKE_BALL;
}

void RogueSafari_SetActivePokeballType(u16 itemId)
{
    VarSet(VAR_ROGUE_SAFARI_BALL_TYPE, itemId);
}