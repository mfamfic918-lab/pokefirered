#include "global.h"
#include "difficulty.h"
#include "event_data.h"
#include "random.h"
#include "pokemon.h"
#include "item.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/pokemon.h"

#if DIFF_EXTRAS

// Options chosen during Oak's speech. They are kept in RAM until NewGameInitData
// stores them in VAR_DIFF_OPTIONS (bit mask, see include/difficulty.h).
EWRAM_DATA u16 gDiffOptionsChoice = 0;

#define VAR_DIFF_OPTIONS   VAR_0x40F4
#define VAR_DIFF_STARTER_0 VAR_0x40F5
#define VAR_DIFF_STARTER_1 VAR_0x40F6
#define VAR_DIFF_STARTER_2 VAR_0x40F7

bool8 DiffOpt(u16 mask)
{
    return (VarGet(VAR_DIFF_OPTIONS) & mask) != 0;
}

// 0xFF when mono-type mode is off
u8 DiffOpt_MonoType(void)
{
    u8 v = (VarGet(VAR_DIFF_OPTIONS) >> DIFFOPT_MONO_SHIFT) & 0x1F;
    return v == 0 ? 0xFF : v - 1;
}

static u16 GetBaseStatTotal(u16 species)
{
    return gSpeciesInfo[species].baseHP + gSpeciesInfo[species].baseAttack
         + gSpeciesInfo[species].baseDefense + gSpeciesInfo[species].baseSpeed
         + gSpeciesInfo[species].baseSpAttack + gSpeciesInfo[species].baseSpDefense;
}

// Picks a weak, first-stage-like species (optionally of a given type; 0xFF = any)
static u16 PickStarterSpecies(u8 type, u16 avoid1, u16 avoid2)
{
    static const u16 limits[] = {330, 400, 500};
    u8 pass;
    u16 tries;

    for (pass = 0; pass < 3; pass++)
    {
        for (tries = 0; tries < 800; tries++)
        {
            u16 s = (Random() % (NUM_SPECIES - 1)) + 1;
            u16 total = GetBaseStatTotal(s);

            if (total == 0 || total > limits[pass] || s == SPECIES_UNOWN || s == avoid1 || s == avoid2)
                continue;
            if (type != 0xFF && gSpeciesInfo[s].types[0] != type && gSpeciesInfo[s].types[1] != type)
                continue;
            return s;
        }
    }
    return SPECIES_RATTATA;
}

// Called at the end of NewGameInitData.
void DiffOptions_ApplyNewGame(void)
{
    u16 opts = gDiffOptionsChoice;
    u8 mono;

    gDiffOptionsChoice = 0;
    if (opts & DIFFOPT_DEATH_MODE)
        opts = DIFFOPT_DEATH_MODE | DIFFOPT_NUZLOCKE;
    VarSet(VAR_DIFF_OPTIONS, opts);
    if (opts & DIFFOPT_NUZLOCKE)
        FlagSet(FLAG_DIFF_NUZLOCKE);
    if (!(opts & DIFFOPT_TM_VENDOR))
        FlagSet(FLAG_DIFF_HIDE_TM_VENDOR);

    mono = DiffOpt_MonoType();
    if (opts & DIFFOPT_RANDOM_STARTERS)
        mono = 0xFF;
    if ((opts & DIFFOPT_RANDOM_STARTERS) || mono != 0xFF)
    {
        u16 a = PickStarterSpecies(mono, SPECIES_NONE, SPECIES_NONE);
        u16 b = PickStarterSpecies(mono, a, SPECIES_NONE);
        u16 c = PickStarterSpecies(mono, a, b);
        VarSet(VAR_DIFF_STARTER_0, a);
        VarSet(VAR_DIFF_STARTER_1, b);
        VarSet(VAR_DIFF_STARTER_2, c);
    }
}

// The three starters of Oak's lab are replaced when random / mono-type starters are on.
u16 DiffOptions_RemapStarter(u16 species, u8 level)
{
    u16 repl;

    if (level != 5 || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB)
     || gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB))
        return species;
    if (!DiffOpt(DIFFOPT_RANDOM_STARTERS) && DiffOpt_MonoType() == 0xFF)
        return species;
    switch (species)
    {
    case SPECIES_BULBASAUR:
        repl = VarGet(VAR_DIFF_STARTER_0);
        break;
    case SPECIES_CHARMANDER:
        repl = VarGet(VAR_DIFF_STARTER_1);
        break;
    case SPECIES_SQUIRTLE:
        repl = VarGet(VAR_DIFF_STARTER_2);
        break;
    default:
        return species;
    }
    return repl != SPECIES_NONE ? repl : species;
}

// Random ground items: a mix of useful things (never key items or HMs)
static const u16 sRandomItems[] =
{
    ITEM_POTION, ITEM_SUPER_POTION, ITEM_HYPER_POTION, ITEM_MAX_POTION, ITEM_FULL_RESTORE,
    ITEM_REVIVE, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_BURN_HEAL, ITEM_ICE_HEAL,
    ITEM_FULL_HEAL, ITEM_ETHER, ITEM_ELIXIR, ITEM_MAX_ETHER, ITEM_MAX_ELIXIR,
    ITEM_POKE_BALL, ITEM_GREAT_BALL, ITEM_ULTRA_BALL, ITEM_REPEL, ITEM_SUPER_REPEL, ITEM_MAX_REPEL,
    ITEM_ESCAPE_ROPE, ITEM_RARE_CANDY, ITEM_PP_UP, ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON,
    ITEM_CARBOS, ITEM_CALCIUM, ITEM_ZINC, ITEM_X_ATTACK, ITEM_X_DEFEND, ITEM_X_SPEED,
    ITEM_FIRE_STONE, ITEM_WATER_STONE, ITEM_THUNDER_STONE, ITEM_LEAF_STONE, ITEM_MOON_STONE,
    ITEM_NUGGET, ITEM_LEFTOVERS, ITEM_FOCUS_BAND, ITEM_QUICK_CLAW, ITEM_SHELL_BELL, ITEM_BRIGHT_POWDER,
};

u16 DiffOptions_RandomFoundItem(u16 item)
{
    if (!DiffOpt(DIFFOPT_RANDOM_ITEMS))
        return item;
    if (item >= ITEM_HM01 && item < ITEM_HM01 + 8) // HMs
        return item;
    if (ItemId_GetPocket(item) == POCKET_KEY_ITEMS)
        return item;
    return sRandomItems[Random() % NELEMS(sRandomItems)];
}

// Random abilities: every species gets a fixed, shuffled ability per slot
u8 DiffOptions_RandomAbility(u16 species, bool8 abilityNum)
{
    u32 h = (species * 2 + abilityNum + 1) * 2654435761u;
    u8 ability;

    h ^= h >> 15;
    h *= 2246822519u;
    h ^= h >> 13;
    ability = 1 + (h % (ABILITIES_COUNT - 1));
    if (ability == ABILITY_WONDER_GUARD)
        ability = ABILITY_STURDY;
    return ability;
}

// Death mode: a move of each type that hits the player's party hard.
static const u16 sCounterMoves[NUMBER_OF_MON_TYPES] =
{
    [TYPE_NORMAL]   = MOVE_NONE,
    [TYPE_FIGHTING] = MOVE_BRICK_BREAK,
    [TYPE_FLYING]   = MOVE_WING_ATTACK,
    [TYPE_POISON]   = MOVE_SLUDGE_BOMB,
    [TYPE_GROUND]   = MOVE_EARTHQUAKE,
    [TYPE_ROCK]     = MOVE_ROCK_SLIDE,
    [TYPE_BUG]      = MOVE_SILVER_WIND,
    [TYPE_GHOST]    = MOVE_SHADOW_BALL,
    [TYPE_STEEL]    = MOVE_METEOR_MASH,
    [TYPE_MYSTERY]  = MOVE_NONE,
    [TYPE_FIRE]     = MOVE_FLAMETHROWER,
    [TYPE_WATER]    = MOVE_SURF,
    [TYPE_GRASS]    = MOVE_GIGA_DRAIN,
    [TYPE_ELECTRIC] = MOVE_THUNDERBOLT,
    [TYPE_PSYCHIC]  = MOVE_PSYCHIC,
    [TYPE_ICE]      = MOVE_ICE_BEAM,
    [TYPE_DRAGON]   = MOVE_DRAGON_CLAW,
    [TYPE_DARK]     = MOVE_CRUNCH,
};

extern const u8 gTypeEffectiveness[336];

static u8 TypeMultiplier(u8 atkType, u8 defType)
{
    u16 i;

    for (i = 0; gTypeEffectiveness[i] != 0xFF; i += 3)
    {
        if (gTypeEffectiveness[i] == 0xFE) // type chart marker for Foresight
            continue;
        if (gTypeEffectiveness[i] == atkType && gTypeEffectiveness[i + 1] == defType)
            return gTypeEffectiveness[i + 2];
    }
    return 10;
}

// Gives the opponent's Pokemon moves of the two types that are most effective
// against the player's current party.
void DiffOptions_AdjustTrainerParty(struct Pokemon *party, u8 count)
{
    u8 score[NUMBER_OF_MON_TYPES];
    u8 t, i, best, second;

    if (!DiffOpt(DIFFOPT_DEATH_MODE))
        return;
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        score[t] = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        u8 d1, d2;

        if (species == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_SANITY_IS_EGG))
            continue;
        d1 = gSpeciesInfo[species].types[0];
        d2 = gSpeciesInfo[species].types[1];
        for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        {
            u16 m;

            if (sCounterMoves[t] == MOVE_NONE)
                continue;
            m = TypeMultiplier(t, d1);
            if (d2 != d1)
                m = m * TypeMultiplier(t, d2) / 10;
            if (m >= 20)
                score[t]++;
        }
    }
    best = 0xFF;
    second = 0xFF;
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
    {
        if (sCounterMoves[t] == MOVE_NONE || score[t] == 0)
            continue;
        if (best == 0xFF || score[t] > score[best])
        {
            second = best;
            best = t;
        }
        else if (second == 0xFF || score[t] > score[second])
        {
            second = t;
        }
    }
    for (i = 0; i < count; i++)
    {
        u16 move;
        u8 pp;

        if (best != 0xFF)
        {
            move = sCounterMoves[best];
            SetMonData(&party[i], MON_DATA_MOVE3, &move);
            pp = 10;
            SetMonData(&party[i], MON_DATA_PP3, &pp);
        }
        if (second != 0xFF)
        {
            move = sCounterMoves[second];
            SetMonData(&party[i], MON_DATA_MOVE4, &move);
            pp = 10;
            SetMonData(&party[i], MON_DATA_PP4, &pp);
        }
    }
}

#endif // DIFF_EXTRAS
