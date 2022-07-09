#include "global.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "battle_setup.h"
#include "event_data.h"
#include "random.h"

#include "rogue_controller.h"

#define ROGUE_TRAINER_COUNT (FLAG_ROGUE_TRAINER_END - FLAG_ROGUE_TRAINER_START + 1)
#define ROGUE_ITEM_COUNT (FLAG_ROGUE_ITEM_END - FLAG_ROGUE_ITEM_START + 1)

static void RandomiseEnabledTrainers(void);
static void RandomiseEnabledItems(void);


bool8 Rogue_IsRunActive(void)
{
    return FlagGet(FLAG_ROGUE_RUN_ACTIVE);
}

bool8 Rogue_ForceExpAll(void)
{
    return TRUE;
}

void Rogue_OnNewGame(void)
{
    FlagClear(FLAG_ROGUE_RUN_ACTIVE);

    FlagSet(FLAG_SYS_B_DASH);
    //FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_POKEMON_GET);
    EnableNationalPokedex();
}

void Rogue_OnLoadMap(void)
{
    // TODO - Remove this

    // Seems to be working? Need to track against flag here though, as this gets called for started maps
    //FlagSet(FLAG_SYS_POKEMON_GET);
    
        //*((s32*)((void*)0)) = 123;

    //s32 i;
    //struct WarpEvent *warpEvent = gMapHeader.events->warps;
    //u8 warpCount = gMapHeader.events->warpCount;
    //
    //for (i = 0; i < warpCount; i++, warpEvent++)
    //{
    //    if(i == 0)
    //    {
    //        // Skip first warp as that should always be left as the entry warp
    //        continue;
    //    }
//
    //    // Should be Prof lab
    //    warpEvent->warpId = 0;
    //    warpEvent->mapNum = 4;
    //    warpEvent->mapGroup = 0;
    //}

    // TODO - Do something
}

void Rogue_OnWarpIntoMap(void)
{
    if(gMapHeader.mapLayoutId == LAYOUT_ROGUE_HUB_TRANSITION)
    {
        FlagSet(FLAG_ROGUE_RUN_ACTIVE);
        FlagClear(FLAG_SYS_POKEDEX_GET);
    }
    else if(gMapHeader.mapLayoutId == LAYOUT_ROGUE_HUB)
    {
        FlagClear(FLAG_ROGUE_RUN_ACTIVE);
        FlagSet(FLAG_SYS_POKEDEX_GET);
    }

    if(Rogue_IsRunActive())
    {
        // Reset trainer encounters
        s16 i;
        for(i = 0; i < TRAINERS_COUNT; ++i)
        {
            ClearTrainerFlag(i);
        }

        RandomiseEnabledTrainers();
        RandomiseEnabledItems();
    }
}

void Rogue_OnSetWarpData(struct WarpData *warp)
{
    if(Rogue_IsRunActive())
    {
        warp->mapGroup = MAP_GROUP(ROGUE_ROUTE_FIELD0);
        warp->mapNum = MAP_NUM(ROGUE_ROUTE_FIELD0);
        warp->warpId = 0;
        warp->x = -1;
        warp->y = -1;
    }
}

void Rogue_Battle_StartTrainerBattle(void)
{
    //extern u16 gTrainerBattleOpponent_A;
    //extern u16 gTrainerBattleOpponent_B;

    // TODO - this doesn't really work as it messes up the trainer encounter flag..
    //if(gTrainerBattleOpponent_A != 0)
    //
    //   gTrainerBattleOpponent_A = TRAINER_DRAKE;
    //
}

void Rogue_CreateTrainerMon(u16 trainerNum, struct Pokemon *mon, u16 species, u8 level, u8 fixedIV, u8 hasFixedPersonality, u32 fixedPersonality, u8 otIdType, u32 fixedOtId)
{
    //gTrainers[trainerNum]

    species = SPECIES_YANMA;
    level = 5;
    CreateMon(mon, species, level, fixedIV, hasFixedPersonality, fixedPersonality, otIdType, fixedOtId);
}

void Rogue_CreateWildMon(u8 area, u16* species, u8* level)
{
    if(Rogue_IsRunActive())
    {
        *species = 120;
        *level = 69;
    }
}

//struct WarpEvent
//{
//    s16 x, y;
//    u8 elevation;
//    u8 warpId;
//    u8 mapNum;
//    u8 mapGroup;
//};

//static s8 GetWarpEventAtPosition(struct MapHeader *mapHeader, u16 x, u16 y, u8 elevation)
//{
//    s32 i;
//    struct WarpEvent *warpEvent = mapHeader->events->warps;
//    u8 warpCount = mapHeader->events->warpCount;
//
//    for (i = 0; i < warpCount; i++, warpEvent++)
//    {
//        if ((u16)warpEvent->x == x && (u16)warpEvent->y == y)
//        {
//            if (warpEvent->elevation == elevation || warpEvent->elevation == 0)
//                return i;
//        }
//    }
//    return WARP_ID_NONE;
//}

static void RandomiseEnabledTrainers(void)
{
    s32 i;
    for(i = 0; i < ROGUE_TRAINER_COUNT; ++i)
    {
        if((Random() % 2) == 0)
        {
            // Clear flag to show
            FlagClear(FLAG_ROGUE_TRAINER_START + i);
        }
        else
        {
            // Set flag to hide
            FlagSet(FLAG_ROGUE_TRAINER_START + i);
        }
    }
}

static void RandomiseEnabledItems(void)
{
    s32 i;
    for(i = 0; i < ROGUE_ITEM_COUNT; ++i)
    {
            FlagClear(FLAG_ROGUE_ITEM_START + i);
        //if((Random() % 2) == 0)
        //{
        //    // Clear flag to show
        //    FlagClear(FLAG_ROGUE_ITEM_START + i);
        //}
        //else
        //{
        //    // Set flag to hide
        //    FlagSet(FLAG_ROGUE_ITEM_START + i);
        //}
    }

    VarSet(VAR_ROGUE_ITEM0, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM1, ITEM_BERRY_JUICE);
    VarSet(VAR_ROGUE_ITEM2, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM3, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM4, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM5, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM6, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM7, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM8, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM9, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM10, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM11, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM12, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM13, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM14, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM15, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM16, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM17, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM18, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM19, ITEM_POKE_BALL);
    VarSet(VAR_ROGUE_ITEM20, ITEM_POKE_BALL);
}