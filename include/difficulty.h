#ifndef GUARD_DIFFICULTY_H
#define GUARD_DIFFICULTY_H

// Difficulty selection.
//
// The difficulty is chosen when building the ROM:
//
//     make DIFFICULTY=easy      ("Ruta Tranquila")
//     make DIFFICULTY=medium    ("Liga Equilibrada")
//     make DIFFICULTY=hard      ("Modo Extremo")
//
// Without DIFFICULTY (or with DIFFICULTY=vanilla) the game plays like the
// original. The Makefile passes -DDIFFICULTY=<id> to the C compiler and
// --defsym DIFFICULTY=<id> to the assembler, using the ids below.

#define DIFFICULTY_VANILLA 0
#define DIFFICULTY_EASY    1
#define DIFFICULTY_MEDIUM  2
#define DIFFICULTY_HARD    3

#ifndef DIFFICULTY
#define DIFFICULTY DIFFICULTY_VANILLA
#endif

// ---------------------------------------------------------------------------
// Tunable values. Everything is a percentage unless noted otherwise.
// ---------------------------------------------------------------------------

#if DIFFICULTY == DIFFICULTY_EASY

#define DIFF_WILD_LEVEL_PERCENT      90   // Wild Pokemon levels
#define DIFF_TRAINER_LEVEL_PERCENT   90   // Trainer Pokemon levels
#define DIFF_EXP_PERCENT            120   // Experience gained from battles
#define DIFF_SHOP_PRICE_PERCENT     100   // Price of items bought in Marts
#define DIFF_AI_SWITCH_CHANCE        25   // Chance (%) per turn that a trainer is allowed to consider switching
#define DIFF_AI_ITEM_CHANCE          25   // Chance (%) per turn that a trainer is allowed to consider using an item
#define DIFF_EXTRA_MART_HEALING       1   // Marts also sell basic healing items
#define DIFF_FOUND_HEAL_BONUS         1   // Healing items found on the ground / hidden give +1
#define DIFF_FOUND_HEAL_REPLACE       0
#define DIFF_BOSS_TEAMS               0   // Use the custom gym leader / Elite Four teams

#elif DIFFICULTY == DIFFICULTY_MEDIUM

#define DIFF_WILD_LEVEL_PERCENT     100
#define DIFF_TRAINER_LEVEL_PERCENT  105
#define DIFF_EXP_PERCENT            100
#define DIFF_SHOP_PRICE_PERCENT     100
#define DIFF_AI_SWITCH_CHANCE       100
#define DIFF_AI_ITEM_CHANCE         100
#define DIFF_EXTRA_MART_HEALING       0
#define DIFF_FOUND_HEAL_BONUS         0
#define DIFF_FOUND_HEAL_REPLACE       0
#define DIFF_BOSS_TEAMS               1
#define DIFF_BOSS_IV                128 // 0-255, scaled to 0-31 in game

#elif DIFFICULTY == DIFFICULTY_HARD

#define DIFF_WILD_LEVEL_PERCENT     115
#define DIFF_TRAINER_LEVEL_PERCENT  120
#define DIFF_EXP_PERCENT            100   // Set to e.g. 80 for a stricter, nuzlocke-style experience curve
#define DIFF_SHOP_PRICE_PERCENT     125
#define DIFF_AI_SWITCH_CHANCE       100
#define DIFF_AI_ITEM_CHANCE         100
#define DIFF_EXTRA_MART_HEALING       0
#define DIFF_FOUND_HEAL_BONUS         0
#define DIFF_FOUND_HEAL_REPLACE       1   // Cheap healing items found on the ground become Poke Balls
#define DIFF_BOSS_TEAMS               1
#define DIFF_BOSS_IV                255

#endif // DIFFICULTY

// ---------------------------------------------------------------------------
// Extra features, enabled on the three difficulties (not on the original game)
// ---------------------------------------------------------------------------

#if DIFFICULTY != DIFFICULTY_VANILLA
#define DIFF_EXTRAS             1  // master switch for the features below
#define DIFF_RUN_ANYWHERE       1  // Running Shoes from the start, also inside buildings
#define DIFF_INFINITE_TMS       1  // TMs are not used up when taught
#define DIFF_EXP_ALL            1  // every healthy party Pokemon gets Exp. Share experience
#define DIFF_EXP_PC             1  // Pokemon stored in the PC boxes also gain experience
#define DIFF_NUZLOCKE           1  // optional Nuzlocke mode, chosen during Oak's speech
#else
#define DIFF_EXTRAS             0
#define DIFF_RUN_ANYWHERE       0
#define DIFF_INFINITE_TMS       0
#define DIFF_EXP_ALL            0
#define DIFF_EXP_PC             0
#define DIFF_NUZLOCKE           0
#endif

#if DIFFICULTY == DIFFICULTY_HARD
#define DIFF_RANDOM_NPC_TEAMS   1  // trainers (except gym leaders, Elite Four and Champion) get different Pokemon
#else
#define DIFF_RANDOM_NPC_TEAMS   0
#endif

// Options chosen in Oak's speech, stored in VAR_0x40F4
#define DIFFOPT_SHINY_CLAUSE     (1 << 0)  // Nuzlocke: shiny Pokemon can always be caught
#define DIFFOPT_RANDOM_ITEMS     (1 << 1)  // items on the ground are random
#define DIFFOPT_RANDOM_STARTERS  (1 << 2)  // random starters
#define DIFFOPT_RANDOM_MOVES     (1 << 3)  // random starting moves
#define DIFFOPT_RANDOM_ABILITIES (1 << 4)  // random abilities
#define DIFFOPT_TM_VENDOR        (1 << 5)  // random-TM vendor in the Marts
#define DIFFOPT_DEATH_MODE       (1 << 6)  // Hard only: see DiffOptions_AdjustTrainerParty
#define DIFFOPT_NUZLOCKE         (1 << 7)
#define DIFFOPT_MONO_SHIFT       8         // bits 8-12: mono-type starters (type + 1, 0 = off)

#if DIFF_NUZLOCKE
extern u16 gDiffOptionsChoice;
bool8 DiffOpt(u16 mask);
u8 DiffOpt_MonoType(void);
void DiffOptions_ApplyNewGame(void);
u16 DiffOptions_RemapStarter(u16 species, u8 level);
u16 DiffOptions_RandomFoundItem(u16 item);
u8 DiffOptions_RandomAbility(u16 species, bool8 abilityNum);
struct Pokemon;
void DiffOptions_AdjustTrainerParty(struct Pokemon *party, u8 count);
bool8 DiffNuzlocke_IsActive(void);
void DiffNuzlocke_OnWildBattleStart(void);
bool8 DiffNuzlocke_CanCatch(void);
void DiffNuzlocke_AfterBattle(void);
#endif

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

#if DIFFICULTY != DIFFICULTY_VANILLA

// Scales a level by a percentage, keeping it between 2 and 100.
// 'lvl' may be evaluated more than once, so it must have no side effects.
#define DIFF_SCALE_LEVEL(lvl, pct) \
    ((((lvl) * (pct) / 100) < 2) ? 2 : ((((lvl) * (pct) / 100) > 100) ? 100 : ((lvl) * (pct) / 100)))

#endif // DIFFICULTY != DIFFICULTY_VANILLA

#if DIFFICULTY == DIFFICULTY_EASY

#define DIFF_AI_MAY_SWITCH() ((Random() % 100) < DIFF_AI_SWITCH_CHANCE)
#define DIFF_AI_MAY_USE_ITEM() ((Random() % 100) < DIFF_AI_ITEM_CHANCE)

#else

#define DIFF_AI_MAY_SWITCH() (TRUE)
#define DIFF_AI_MAY_USE_ITEM() (TRUE)

#endif

// Gym leaders were NoItemCustomMoves in the original game. With the custom
// boss teams they carry held items, so the trainer data uses this macro.
#if DIFF_BOSS_TEAMS
#define LEADER_PARTY(party) ITEM_CUSTOM_MOVES(party)
#else
#define LEADER_PARTY(party) NO_ITEM_CUSTOM_MOVES(party)
#endif

#endif // GUARD_DIFFICULTY_H
