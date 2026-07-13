#include "modding.h"
#include "global.h"
#include "recompconfig.h"
#include "recompui.h"
#include "recomputils.h"

#define SAVE_EDITOR_MAGIC_SINGLE_METER 0x30
#define SAVE_EDITOR_MAGIC_DOUBLE_METER (SAVE_EDITOR_MAGIC_SINGLE_METER * 2)
#define MIN_HEARTS 3
#define MAX_HEARTS 20
#define MAX_BANK_RUPEES 5000
#define SKULL_TOKEN_SWAMP_SHIFT 16
#define SKULL_TOKEN_MASK 0xFFFF
#define SAVE_EDITOR_ITEM_TOGGLE_COUNT 12
#define SAVE_EDITOR_MASK_COUNT 24
#define SAVE_EDITOR_TINGLE_MAP_COUNT 6
#define SAVE_EDITOR_BOTTLE_COUNT 6
#define SAVE_EDITOR_BOTTLE_CONTENT_COUNT 22

static RecompuiContext editor_context;
static RecompuiResource root;
static RecompuiResource panel;
static RecompuiResource button_row;
static RecompuiResource content_parent;
static RecompuiResource page_general;
static RecompuiResource page_stats;
static RecompuiResource page_equipment;
static RecompuiResource page_quest;
static RecompuiResource page_dungeons;
static RecompuiResource general_page_button;
static RecompuiResource stats_page_button;
static RecompuiResource equipment_page_button;
static RecompuiResource quest_page_button;
static RecompuiResource dungeons_page_button;
static RecompuiResource name_input;
static RecompuiResource day_slider;
static RecompuiResource time_slider;
static RecompuiResource time_speed_slider;
static RecompuiResource bomber_code_input;
static RecompuiResource rupees_slider;
static RecompuiResource bank_slider;
static RecompuiResource wallet_radio;
static RecompuiResource hearts_slider;
static RecompuiResource health_slider;
static RecompuiResource double_defense_radio;
static RecompuiResource magic_level_radio;
static RecompuiResource magic_slider;
static RecompuiResource spin_attack_radio;
static RecompuiResource chateau_radio;
static RecompuiResource tatl_radio;
static RecompuiResource intro_radio;
static RecompuiResource owl_save_radio;
static RecompuiResource sword_radio;
static RecompuiResource shield_radio;
static RecompuiResource quiver_radio;
static RecompuiResource bomb_bag_radio;
static RecompuiResource stick_upgrade_radio;
static RecompuiResource nut_upgrade_radio;
static RecompuiResource arrows_slider;
static RecompuiResource bombs_slider;
static RecompuiResource bombchus_slider;
static RecompuiResource sticks_slider;
static RecompuiResource nuts_slider;
static RecompuiResource powder_keg_radio;
static RecompuiResource magic_beans_radio;
static RecompuiResource magic_beans_count_slider;
static RecompuiResource tingle_map_radios[SAVE_EDITOR_TINGLE_MAP_COUNT];
static RecompuiResource bottle_radios[SAVE_EDITOR_BOTTLE_COUNT];
static RecompuiResource item_radios[SAVE_EDITOR_ITEM_TOGGLE_COUNT];
static RecompuiResource mask_radios[SAVE_EDITOR_MASK_COUNT];
static RecompuiResource heart_piece_slider;
static RecompuiResource swamp_tokens_slider;
static RecompuiResource ocean_tokens_slider;
static RecompuiResource notebook_radio;
static RecompuiResource remains_radios[4];
static RecompuiResource song_radios[12];
static RecompuiResource dungeon_map_radios[4];
static RecompuiResource dungeon_compass_radios[4];
static RecompuiResource dungeon_boss_key_radios[4];
static RecompuiResource dungeon_key_sliders[4];
static RecompuiResource dungeon_fairy_sliders[4];
static RecompuiResource apply_button;
static RecompuiResource give_quest_button;
static RecompuiResource reset_quest_button;
static RecompuiResource give_dungeons_button;
static RecompuiResource reset_dungeons_button;
static RecompuiResource close_button;

static bool editor_shown = false;
static s32 active_page = 0;
static PlayState* current_play_state = NULL;

static bool string_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return false;
    }

    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return false;
        }
        a++;
        b++;
    }

    return *a == *b;
}

static u16 editor_hotkey_combo(void) {
    char* hotkey = recomp_get_config_string("editor_hotkey");
    u16 combo = BTN_L | BTN_CUP;

    if (string_equals(hotkey, "Disabled")) {
        combo = 0;
    } else if (string_equals(hotkey, "L + D-Up")) {
        combo = BTN_L | BTN_DUP;
    } else if (string_equals(hotkey, "L + C-Up")) {
        combo = BTN_L | BTN_CUP;
    } else if (string_equals(hotkey, "L + R + D-Up")) {
        combo = BTN_L | BTN_R | BTN_DUP;
    } else if (string_equals(hotkey, "L + R")) {
        combo = BTN_L | BTN_R;
    }

    if (hotkey != NULL) {
        recomp_free_config_string(hotkey);
    }

    return combo;
}

static bool editor_hotkey_pressed(Input* input, u16 hotkey) {
    static bool hotkey_down_last_frame = false;
    u16 buttons;
    bool hotkey_down;
    bool pressed;

    if (hotkey == 0) {
        hotkey_down_last_frame = false;
        return false;
    }

    buttons = input->cur.button | input->press.button;
    hotkey_down = CHECK_BTN_ALL(buttons, hotkey);
    pressed = hotkey_down && !hotkey_down_last_frame;

    hotkey_down_last_frame = hotkey_down;
    return pressed;
}

static const s32 s_remains[] = {
    QUEST_REMAINS_ODOLWA,
    QUEST_REMAINS_GOHT,
    QUEST_REMAINS_GYORG,
    QUEST_REMAINS_TWINMOLD,
};

static const s32 s_songs[] = {
    QUEST_SONG_SONATA,
    QUEST_SONG_LULLABY,
    QUEST_SONG_BOSSA_NOVA,
    QUEST_SONG_ELEGY,
    QUEST_SONG_OATH,
    QUEST_SONG_SARIA,
    QUEST_SONG_TIME,
    QUEST_SONG_HEALING,
    QUEST_SONG_EPONA,
    QUEST_SONG_SOARING,
    QUEST_SONG_STORMS,
    QUEST_SONG_SUN,
};

static const s32 s_dungeons[] = {
    DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE,
    DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE,
    DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE,
    DUNGEON_SCENE_INDEX_STONE_TOWER_TEMPLE,
};

static const s32 s_dungeon_key_max[] = { 1, 3, 1, 4 };

typedef struct {
    const char* label;
    s32 item;
} SaveEditorItemToggle;

typedef struct {
    const char* label;
    s32 map;
    s32 week_event_flag;
    u16 cloud_mask;
} SaveEditorTingleMap;

typedef struct {
    const char* label;
    s32 slot;
} SaveEditorBottleSlot;

static const SaveEditorItemToggle s_item_toggles[SAVE_EDITOR_ITEM_TOGGLE_COUNT] = {
    { "Ocarina", ITEM_OCARINA_OF_TIME },
    { "Fire Arrows", ITEM_ARROW_FIRE },
    { "Ice Arrows", ITEM_ARROW_ICE },
    { "Light Arrows", ITEM_ARROW_LIGHT },
    { "Pictograph Box", ITEM_PICTOGRAPH_BOX },
    { "Lens of Truth", ITEM_LENS_OF_TRUTH },
    { "Hookshot", ITEM_HOOKSHOT },
    { "Great Fairy Sword", ITEM_SWORD_GREAT_FAIRY },
    { "Room Key", ITEM_ROOM_KEY },
    { "Letter to Mama", ITEM_LETTER_MAMA },
    { "Letter to Kafei", ITEM_LETTER_TO_KAFEI },
    { "Pendant of Memories", ITEM_PENDANT_OF_MEMORIES },
};

static const SaveEditorItemToggle s_mask_toggles[SAVE_EDITOR_MASK_COUNT] = {
    { "Deku Mask", ITEM_MASK_DEKU },
    { "Goron Mask", ITEM_MASK_GORON },
    { "Zora Mask", ITEM_MASK_ZORA },
    { "Fierce Deity Mask", ITEM_MASK_FIERCE_DEITY },
    { "Mask of Truth", ITEM_MASK_TRUTH },
    { "Kafei's Mask", ITEM_MASK_KAFEIS_MASK },
    { "All-Night Mask", ITEM_MASK_ALL_NIGHT },
    { "Bunny Hood", ITEM_MASK_BUNNY },
    { "Keaton Mask", ITEM_MASK_KEATON },
    { "Garo's Mask", ITEM_MASK_GARO },
    { "Romani's Mask", ITEM_MASK_ROMANI },
    { "Circus Leader's Mask", ITEM_MASK_CIRCUS_LEADER },
    { "Postman's Hat", ITEM_MASK_POSTMAN },
    { "Couple's Mask", ITEM_MASK_COUPLE },
    { "Great Fairy's Mask", ITEM_MASK_GREAT_FAIRY },
    { "Gibdo Mask", ITEM_MASK_GIBDO },
    { "Don Gero's Mask", ITEM_MASK_DON_GERO },
    { "Kamaro's Mask", ITEM_MASK_KAMARO },
    { "Captain's Hat", ITEM_MASK_CAPTAIN },
    { "Stone Mask", ITEM_MASK_STONE },
    { "Bremen Mask", ITEM_MASK_BREMEN },
    { "Blast Mask", ITEM_MASK_BLAST },
    { "Mask of Scents", ITEM_MASK_SCENTS },
    { "Giant's Mask", ITEM_MASK_GIANT },
};

static const SaveEditorTingleMap s_tingle_maps[SAVE_EDITOR_TINGLE_MAP_COUNT] = {
    { "Clock Town", TINGLE_MAP_CLOCK_TOWN, WEEKEVENTREG_TINGLE_MAP_BOUGHT_CLOCK_TOWN, 0x0003 },
    { "Woodfall", TINGLE_MAP_WOODFALL, WEEKEVENTREG_TINGLE_MAP_BOUGHT_WOODFALL, 0x001C },
    { "Snowhead", TINGLE_MAP_SNOWHEAD, WEEKEVENTREG_TINGLE_MAP_BOUGHT_SNOWHEAD, 0x00E0 },
    { "Romani Ranch", TINGLE_MAP_ROMANI_RANCH, WEEKEVENTREG_TINGLE_MAP_BOUGHT_ROMANI_RANCH, 0x0100 },
    { "Great Bay", TINGLE_MAP_GREAT_BAY, WEEKEVENTREG_TINGLE_MAP_BOUGHT_GREAT_BAY, 0x1E00 },
    { "Stone Tower", TINGLE_MAP_STONE_TOWER, WEEKEVENTREG_TINGLE_MAP_BOUGHT_STONE_TOWER, 0x6000 },
};

static const SaveEditorBottleSlot s_bottle_slots[SAVE_EDITOR_BOTTLE_COUNT] = {
    { "Bottle 1", SLOT_BOTTLE_1 },
    { "Bottle 2", SLOT_BOTTLE_2 },
    { "Bottle 3", SLOT_BOTTLE_3 },
    { "Bottle 4", SLOT_BOTTLE_4 },
    { "Bottle 5", SLOT_BOTTLE_5 },
    { "Bottle 6", SLOT_BOTTLE_6 },
};

static const s32 s_bottle_contents[SAVE_EDITOR_BOTTLE_CONTENT_COUNT] = {
    ITEM_NONE,
    ITEM_BOTTLE,
    ITEM_POTION_RED,
    ITEM_POTION_GREEN,
    ITEM_POTION_BLUE,
    ITEM_CHATEAU,
    ITEM_SPRING_WATER,
    ITEM_HOT_SPRING_WATER,
    ITEM_MILK_BOTTLE,
    ITEM_MILK_HALF,
    ITEM_FAIRY,
    ITEM_FISH,
    ITEM_BUG,
    ITEM_POE,
    ITEM_BIG_POE,
    ITEM_ZORA_EGG,
    ITEM_DEKU_PRINCESS,
    ITEM_GOLD_DUST,
    ITEM_MUSHROOM,
    ITEM_SEAHORSE,
    ITEM_BLUE_FIRE,
    ITEM_HYLIAN_LOACH,
};

static void set_active_page(s32 page) {
    active_page = page;
    recompui_set_display(page_general, page == 0 ? DISPLAY_FLEX : DISPLAY_NONE);
    recompui_set_display(page_stats, page == 1 ? DISPLAY_FLEX : DISPLAY_NONE);
    recompui_set_display(page_equipment, page == 2 ? DISPLAY_FLEX : DISPLAY_NONE);
    recompui_set_display(page_quest, page == 3 ? DISPLAY_FLEX : DISPLAY_NONE);
    recompui_set_display(page_dungeons, page == 4 ? DISPLAY_FLEX : DISPLAY_NONE);
}

static char z2_ascii(int code) {
    if (code < 10) {
        return (char)(code + 0x30);
    }
    if (code < 36) {
        return (char)(code + 0x37);
    }
    if (code < 62) {
        return (char)(code + 0x3D);
    }
    if (code == 62) {
        return ' ';
    }
    if (code == 63 || code == 64) {
        return (char)(code - 0x12);
    }
    return (char)code;
}

static char ascii2_z(int code) {
    if (code >= '0' && code <= '9') {
        return (char)(code - 0x30);
    }
    if (code >= 'A' && code <= 'Z') {
        return (char)(code - 0x37);
    }
    if (code >= 'a' && code <= 'z') {
        return (char)(code - 0x3D);
    }
    if (code == ' ' || code == 0) {
        return 62;
    }
    if (code == '_' || code == '.') {
        return (char)(code + 0x12);
    }
    return 62;
}

static s32 clamp_s32(s32 value, s32 min, s32 max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static s32 display_day_to_save_day(s32 day) {
    return clamp_s32(day, 1, 4) - 1;
}

static s32 save_day_to_display_day(s32 day) {
    return clamp_s32(day + 1, 1, 4);
}

static s32 current_magic_level(void) {
    if (gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired ||
        (gSaveContext.save.saveInfo.playerData.magicLevel >= 2)) {
        return 2;
    }
    if (gSaveContext.save.saveInfo.playerData.isMagicAcquired ||
        (gSaveContext.save.saveInfo.playerData.magicLevel >= 1)) {
        return 1;
    }
    return 0;
}

static void set_quest_flag(s32 quest, bool enabled);
static void set_week_flag(s32 flag, bool enabled);
static void give_item_with_ammo(s32 item, s32 ammo);
static void set_all_quest_items(bool enabled);
static void set_all_dungeon_items(bool enabled);

static void apply_ui_item(RecompuiResource radio, s32 item) {
    INV_CONTENT(item) = recompui_get_input_value_u32(radio) != 0 ? item : ITEM_NONE;
}

static void apply_live_day_time(s32 day, s32 time, s32 time_speed_offset) {
    u8 event_inf_bits;

    gSaveContext.save.day = display_day_to_save_day(day);
    gSaveContext.save.eventDayCount = gSaveContext.save.day;
    gSaveContext.save.time = clamp_s32(time, 0, 0xFFFF);
    gSaveContext.save.timeSpeedOffset = clamp_s32(time_speed_offset, -2, 18);
    gSaveContext.skyboxTime = gSaveContext.save.time;
    gSaveContext.save.isNight =
        ((CURRENT_TIME >= CLOCK_TIME(18, 0)) || (CURRENT_TIME < CLOCK_TIME(6, 0))) ? true : false;
    EVENTINF_SET_7_E0(gSaveContext.save.day, event_inf_bits);

    if (current_play_state != NULL) {
        Interface_NewDay(current_play_state, CURRENT_DAY);
    }
}

static void get_player_name(char* name) {
    for (s32 i = 0; i < 8; i++) {
        name[i] = z2_ascii(gSaveContext.save.saveInfo.playerData.playerName[i]);
    }
    name[8] = '\0';
}

static void set_player_name(const char* name) {
    s32 i;

    for (i = 0; i < 8 && name[i] != '\0'; i++) {
        gSaveContext.save.saveInfo.playerData.playerName[i] = ascii2_z(name[i]);
    }
    for (; i < 8; i++) {
        gSaveContext.save.saveInfo.playerData.playerName[i] = ascii2_z(' ');
    }
}

static s32 wallet_cap(void) {
    switch (GET_CUR_UPG_VALUE(UPG_WALLET)) {
        case 0:
            return 99;
        case 1:
            return 200;
        case 2:
            return 500;
        default:
            return 999;
    }
}

static u32 swamp_skull_tokens(void) {
    return (gSaveContext.save.saveInfo.skullTokenCount >> SKULL_TOKEN_SWAMP_SHIFT) & SKULL_TOKEN_MASK;
}

static u32 ocean_skull_tokens(void) {
    return gSaveContext.save.saveInfo.skullTokenCount & SKULL_TOKEN_MASK;
}

static void set_skull_tokens(u32 swamp, u32 ocean) {
    gSaveContext.save.saveInfo.skullTokenCount =
        ((swamp & SKULL_TOKEN_MASK) << SKULL_TOKEN_SWAMP_SHIFT) | (ocean & SKULL_TOKEN_MASK);
}

static s32 bottle_content_index(s32 item) {
    for (s32 i = 0; i < ARRAY_COUNT(s_bottle_contents); i++) {
        if (s_bottle_contents[i] == item) {
            return i;
        }
    }
    return 0;
}

static s32 bottle_content_from_index(s32 index) {
    return s_bottle_contents[clamp_s32(index, 0, ARRAY_COUNT(s_bottle_contents) - 1)];
}

static void set_tingle_map(const SaveEditorTingleMap* map, bool enabled) {
    if (enabled) {
        Inventory_SetWorldMapCloudVisibility(map->map);
        SET_WEEKEVENTREG(map->week_event_flag);
    } else {
        gSaveContext.save.saveInfo.worldMapCloudVisibility &= (u32)~map->cloud_mask;
        CLEAR_WEEKEVENTREG(map->week_event_flag);
    }
}

static void get_bomber_code(char* text) {
    for (s32 i = 0; i < 5; i++) {
        text[i] = (char)('0' + clamp_s32(gSaveContext.save.saveInfo.bomberCode[i], 1, 5));
    }
    text[5] = '\0';
}

static bool parse_bomber_code(const char* text, s8 code[5]) {
    if (text == NULL) {
        return false;
    }

    for (s32 i = 0; i < 5; i++) {
        if (text[i] < '1' || text[i] > '5') {
            return false;
        }
        code[i] = (s8)(text[i] - '0');
    }

    return text[5] == '\0';
}

static void set_upgrade_value(s32 upgrade, u32 value) {
    gSaveContext.save.saveInfo.inventory.upgrades =
        (GET_SAVE_INVENTORY_UPGRADES & gUpgradeNegMasks[upgrade]) | (value << gUpgradeShifts[upgrade]);
}

static void set_quest_flag(s32 quest, bool enabled) {
    if (enabled) {
        SET_QUEST_ITEM(quest);
    } else {
        REMOVE_QUEST_ITEM(quest);
    }
}

static void set_week_flag(s32 flag, bool enabled) {
    if (enabled) {
        SET_WEEKEVENTREG(flag);
    } else {
        CLEAR_WEEKEVENTREG(flag);
    }
}

static void set_dungeon_flag(s32 dungeon, s32 item, bool enabled) {
    if (enabled) {
        SET_DUNGEON_ITEM(item, dungeon);
    } else {
        gSaveContext.save.saveInfo.inventory.dungeonItems[dungeon] &= (u8)~gBitFlags[item];
    }
}

static void set_heart_piece_count(u32 count) {
    gSaveContext.save.saveInfo.inventory.questItems &= ~0xF0000000;
    gSaveContext.save.saveInfo.inventory.questItems |= (clamp_s32(count, 0, 3) << QUEST_HEART_PIECE_COUNT);
}

static void give_item_with_ammo(s32 item, s32 ammo) {
    INV_CONTENT(item) = item;
    AMMO(item) = ammo;
}

static void clamp_live_save(void) {
    s32 max_rupees = wallet_cap();
    s32 magic_level;

    gSaveContext.save.day = clamp_s32(gSaveContext.save.day, 0, 3);
    gSaveContext.save.eventDayCount = clamp_s32(gSaveContext.save.eventDayCount, 0, 3);
    gSaveContext.save.timeSpeedOffset = clamp_s32(gSaveContext.save.timeSpeedOffset, -2, 18);
    gSaveContext.save.saveInfo.playerData.healthCapacity =
        clamp_s32(gSaveContext.save.saveInfo.playerData.healthCapacity, MIN_HEARTS * 0x10, MAX_HEARTS * 0x10);
    gSaveContext.save.saveInfo.playerData.health =
        clamp_s32(gSaveContext.save.saveInfo.playerData.health, 0, gSaveContext.save.saveInfo.playerData.healthCapacity);
    gSaveContext.save.saveInfo.playerData.rupees =
        clamp_s32(gSaveContext.save.saveInfo.playerData.rupees, 0, max_rupees);
    magic_level = current_magic_level();
    gSaveContext.save.saveInfo.playerData.magicLevel = magic_level;
    gSaveContext.magicCapacity = magic_level * SAVE_EDITOR_MAGIC_SINGLE_METER;
    gSaveContext.save.saveInfo.playerData.magic =
        clamp_s32(gSaveContext.save.saveInfo.playerData.magic, 0, gSaveContext.magicCapacity);
}

static void sync_editor_from_save(void) {
    char name[9];
    char bomber_code[6];

    clamp_live_save();
    get_player_name(name);
    get_bomber_code(bomber_code);
    recompui_set_input_text(name_input, name);
    recompui_set_input_text(bomber_code_input, bomber_code);
    recompui_set_input_value_u32(day_slider, save_day_to_display_day(gSaveContext.save.day));
    recompui_set_input_value_u32(time_slider, gSaveContext.save.time);
    recompui_set_input_value_float(time_speed_slider, (float)gSaveContext.save.timeSpeedOffset);
    recompui_set_input_value_u32(wallet_radio, GET_CUR_UPG_VALUE(UPG_WALLET));
    recompui_set_input_value_u32(rupees_slider, gSaveContext.save.saveInfo.playerData.rupees);
    recompui_set_input_value_u32(bank_slider, HS_GET_BANK_RUPEES());
    recompui_set_input_value_u32(hearts_slider, gSaveContext.save.saveInfo.playerData.healthCapacity / 0x10);
    recompui_set_input_value_u32(health_slider, gSaveContext.save.saveInfo.playerData.health);
    recompui_set_input_value_u32(double_defense_radio, gSaveContext.save.saveInfo.playerData.doubleDefense ? 1 : 0);
    recompui_set_input_value_u32(magic_level_radio, current_magic_level());
    recompui_set_input_value_u32(magic_slider, gSaveContext.save.saveInfo.playerData.magic);
    recompui_set_input_value_u32(spin_attack_radio, CHECK_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_GREAT_SPIN_ATTACK) ? 1 : 0);
    recompui_set_input_value_u32(chateau_radio, CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI) ? 1 : 0);
    recompui_set_input_value_u32(tatl_radio, gSaveContext.save.hasTatl ? 1 : 0);
    recompui_set_input_value_u32(intro_radio, gSaveContext.save.isFirstCycle ? 1 : 0);
    recompui_set_input_value_u32(owl_save_radio, gSaveContext.save.isOwlSave ? 1 : 0);
    recompui_set_input_value_u32(
        sword_radio,
        clamp_s32(GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD), EQUIP_VALUE_SWORD_NONE, EQUIP_VALUE_SWORD_GILDED));
    recompui_set_input_value_u32(shield_radio, GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD));
    recompui_set_input_value_u32(quiver_radio, GET_CUR_UPG_VALUE(UPG_QUIVER));
    recompui_set_input_value_u32(bomb_bag_radio, GET_CUR_UPG_VALUE(UPG_BOMB_BAG));
    recompui_set_input_value_u32(stick_upgrade_radio, GET_CUR_UPG_VALUE(UPG_DEKU_STICKS));
    recompui_set_input_value_u32(nut_upgrade_radio, GET_CUR_UPG_VALUE(UPG_DEKU_NUTS));
    recompui_set_input_value_u32(arrows_slider, AMMO(ITEM_BOW));
    recompui_set_input_value_u32(bombs_slider, AMMO(ITEM_BOMB));
    recompui_set_input_value_u32(bombchus_slider, AMMO(ITEM_BOMBCHU));
    recompui_set_input_value_u32(sticks_slider, AMMO(ITEM_DEKU_STICK));
    recompui_set_input_value_u32(nuts_slider, AMMO(ITEM_DEKU_NUT));
    recompui_set_input_value_u32(powder_keg_radio, GET_INV_CONTENT(ITEM_POWDER_KEG) == ITEM_POWDER_KEG ? 1 : 0);
    recompui_set_input_value_u32(magic_beans_radio, GET_INV_CONTENT(ITEM_MAGIC_BEANS) == ITEM_MAGIC_BEANS ? 1 : 0);
    recompui_set_input_value_u32(magic_beans_count_slider, AMMO(ITEM_MAGIC_BEANS));
    for (s32 i = 0; i < ARRAY_COUNT(s_tingle_maps); i++) {
        recompui_set_input_value_u32(tingle_map_radios[i], CHECK_WEEKEVENTREG(s_tingle_maps[i].week_event_flag) ? 1 : 0);
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_bottle_slots); i++) {
        s32 item = gSaveContext.save.saveInfo.inventory.items[s_bottle_slots[i].slot];
        recompui_set_input_value_u32(bottle_radios[i], bottle_content_index(item));
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_item_toggles); i++) {
        recompui_set_input_value_u32(item_radios[i], GET_INV_CONTENT(s_item_toggles[i].item) == s_item_toggles[i].item ? 1 : 0);
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_mask_toggles); i++) {
        recompui_set_input_value_u32(mask_radios[i], GET_INV_CONTENT(s_mask_toggles[i].item) == s_mask_toggles[i].item ? 1 : 0);
    }
    recompui_set_input_value_u32(heart_piece_slider, GET_QUEST_HEART_PIECE_COUNT);
    recompui_set_input_value_u32(swamp_tokens_slider, swamp_skull_tokens());
    recompui_set_input_value_u32(ocean_tokens_slider, ocean_skull_tokens());
    recompui_set_input_value_u32(notebook_radio, CHECK_QUEST_ITEM(QUEST_BOMBERS_NOTEBOOK) ? 1 : 0);

    for (s32 i = 0; i < ARRAY_COUNT(s_remains); i++) {
        recompui_set_input_value_u32(remains_radios[i], CHECK_QUEST_ITEM(s_remains[i]) ? 1 : 0);
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_songs); i++) {
        recompui_set_input_value_u32(song_radios[i], CHECK_QUEST_ITEM(s_songs[i]) ? 1 : 0);
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_dungeons); i++) {
        s32 dungeon = s_dungeons[i];
        recompui_set_input_value_u32(dungeon_map_radios[i], CHECK_DUNGEON_ITEM(DUNGEON_MAP, dungeon) ? 1 : 0);
        recompui_set_input_value_u32(dungeon_compass_radios[i], CHECK_DUNGEON_ITEM(DUNGEON_COMPASS, dungeon) ? 1 : 0);
        recompui_set_input_value_u32(dungeon_boss_key_radios[i], CHECK_DUNGEON_ITEM(DUNGEON_BOSS_KEY, dungeon) ? 1 : 0);
        recompui_set_input_value_u32(dungeon_key_sliders[i], DUNGEON_KEY_COUNT(dungeon));
        recompui_set_input_value_u32(dungeon_fairy_sliders[i], gSaveContext.save.saveInfo.inventory.strayFairies[dungeon]);
    }
}

static void sync_editor_from_save_closed_context(void) {
    recompui_open_context(editor_context);
    sync_editor_from_save();
    recompui_close_context(editor_context);
}

static void apply_editor_to_save(void) {
    char* name = recompui_get_input_text(name_input);
    char* bomber_code_text = recompui_get_input_text(bomber_code_input);
    s8 bomber_code[5];
    s32 hearts = recompui_get_input_value_u32(hearts_slider);
    s32 magic_level = recompui_get_input_value_u32(magic_level_radio);
    s32 i;

    if (name != NULL) {
        set_player_name(name);
        recomp_free(name);
    }

    if (parse_bomber_code(bomber_code_text, bomber_code)) {
        for (i = 0; i < ARRAY_COUNT(bomber_code); i++) {
            gSaveContext.save.saveInfo.bomberCode[i] = bomber_code[i];
        }
    }
    if (bomber_code_text != NULL) {
        recomp_free(bomber_code_text);
    }

    apply_live_day_time(recompui_get_input_value_u32(day_slider), recompui_get_input_value_u32(time_slider),
                        recompui_get_input_value_u32(time_speed_slider));
    set_upgrade_value(UPG_WALLET, recompui_get_input_value_u32(wallet_radio));
    gSaveContext.save.saveInfo.playerData.rupees = recompui_get_input_value_u32(rupees_slider);
    HS_SET_BANK_RUPEES(clamp_s32(recompui_get_input_value_u32(bank_slider), 0, MAX_BANK_RUPEES));
    gSaveContext.save.saveInfo.playerData.healthCapacity = hearts * 0x10;
    gSaveContext.save.saveInfo.playerData.health = recompui_get_input_value_u32(health_slider);
    gSaveContext.save.saveInfo.playerData.doubleDefense = recompui_get_input_value_u32(double_defense_radio) != 0;
    gSaveContext.save.saveInfo.inventory.defenseHearts =
        gSaveContext.save.saveInfo.playerData.doubleDefense ? MAX_HEARTS : 0;
    gSaveContext.save.saveInfo.playerData.magicLevel = magic_level;
    gSaveContext.save.saveInfo.playerData.magic = recompui_get_input_value_u32(magic_slider);
    gSaveContext.save.saveInfo.playerData.isMagicAcquired = magic_level >= 1;
    gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = magic_level >= 2;
    gSaveContext.magicCapacity = magic_level * SAVE_EDITOR_MAGIC_SINGLE_METER;
    gSaveContext.magicFillTarget = gSaveContext.magicCapacity;
    gSaveContext.save.hasTatl = recompui_get_input_value_u32(tatl_radio) != 0;
    gSaveContext.save.isFirstCycle = recompui_get_input_value_u32(intro_radio) != 0;
    gSaveContext.save.isOwlSave = recompui_get_input_value_u32(owl_save_radio) != 0;
    set_week_flag(WEEKEVENTREG_RECEIVED_GREAT_SPIN_ATTACK, recompui_get_input_value_u32(spin_attack_radio) != 0);
    set_week_flag(WEEKEVENTREG_DRANK_CHATEAU_ROMANI, recompui_get_input_value_u32(chateau_radio) != 0);

    SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, clamp_s32(recompui_get_input_value_u32(sword_radio), EQUIP_VALUE_SWORD_NONE,
                                                EQUIP_VALUE_SWORD_GILDED));
    SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, recompui_get_input_value_u32(shield_radio));
    set_upgrade_value(UPG_QUIVER, recompui_get_input_value_u32(quiver_radio));
    set_upgrade_value(UPG_BOMB_BAG, recompui_get_input_value_u32(bomb_bag_radio));
    set_upgrade_value(UPG_DEKU_STICKS, recompui_get_input_value_u32(stick_upgrade_radio));
    set_upgrade_value(UPG_DEKU_NUTS, recompui_get_input_value_u32(nut_upgrade_radio));
    if (recompui_get_input_value_u32(arrows_slider) > 0) {
        give_item_with_ammo(ITEM_BOW, recompui_get_input_value_u32(arrows_slider));
    }
    if (recompui_get_input_value_u32(bombs_slider) > 0) {
        give_item_with_ammo(ITEM_BOMB, recompui_get_input_value_u32(bombs_slider));
    }
    if (recompui_get_input_value_u32(bombchus_slider) > 0) {
        give_item_with_ammo(ITEM_BOMBCHU, recompui_get_input_value_u32(bombchus_slider));
    }
    if (recompui_get_input_value_u32(sticks_slider) > 0) {
        give_item_with_ammo(ITEM_DEKU_STICK, recompui_get_input_value_u32(sticks_slider));
    }
    if (recompui_get_input_value_u32(nuts_slider) > 0) {
        give_item_with_ammo(ITEM_DEKU_NUT, recompui_get_input_value_u32(nuts_slider));
    }
    apply_ui_item(powder_keg_radio, ITEM_POWDER_KEG);
    if (recompui_get_input_value_u32(magic_beans_radio) != 0) {
        give_item_with_ammo(ITEM_MAGIC_BEANS, clamp_s32(recompui_get_input_value_u32(magic_beans_count_slider), 1, 20));
    } else {
        INV_CONTENT(ITEM_MAGIC_BEANS) = ITEM_NONE;
        AMMO(ITEM_MAGIC_BEANS) = 0;
    }
    for (i = 0; i < ARRAY_COUNT(s_tingle_maps); i++) {
        set_tingle_map(&s_tingle_maps[i], recompui_get_input_value_u32(tingle_map_radios[i]) != 0);
    }
    for (i = 0; i < ARRAY_COUNT(s_bottle_slots); i++) {
        s32 item_index = recompui_get_input_value_u32(bottle_radios[i]);
        gSaveContext.save.saveInfo.inventory.items[s_bottle_slots[i].slot] = bottle_content_from_index(item_index);
    }
    for (i = 0; i < ARRAY_COUNT(s_item_toggles); i++) {
        apply_ui_item(item_radios[i], s_item_toggles[i].item);
    }
    for (i = 0; i < ARRAY_COUNT(s_mask_toggles); i++) {
        apply_ui_item(mask_radios[i], s_mask_toggles[i].item);
    }

    set_heart_piece_count(recompui_get_input_value_u32(heart_piece_slider));
    set_skull_tokens(recompui_get_input_value_u32(swamp_tokens_slider), recompui_get_input_value_u32(ocean_tokens_slider));
    set_quest_flag(QUEST_BOMBERS_NOTEBOOK, recompui_get_input_value_u32(notebook_radio) != 0);
    for (i = 0; i < ARRAY_COUNT(s_remains); i++) {
        set_quest_flag(s_remains[i], recompui_get_input_value_u32(remains_radios[i]) != 0);
    }
    for (i = 0; i < ARRAY_COUNT(s_songs); i++) {
        set_quest_flag(s_songs[i], recompui_get_input_value_u32(song_radios[i]) != 0);
    }
    for (i = 0; i < ARRAY_COUNT(s_dungeons); i++) {
        s32 dungeon = s_dungeons[i];
        set_dungeon_flag(dungeon, DUNGEON_MAP, recompui_get_input_value_u32(dungeon_map_radios[i]) != 0);
        set_dungeon_flag(dungeon, DUNGEON_COMPASS, recompui_get_input_value_u32(dungeon_compass_radios[i]) != 0);
        set_dungeon_flag(dungeon, DUNGEON_BOSS_KEY, recompui_get_input_value_u32(dungeon_boss_key_radios[i]) != 0);
        gSaveContext.save.saveInfo.inventory.strayFairies[dungeon] =
            clamp_s32(recompui_get_input_value_u32(dungeon_fairy_sliders[i]), 0, STRAY_FAIRY_SCATTERED_TOTAL);
        set_dungeon_flag(dungeon, DUNGEON_STRAY_FAIRIES,
                         gSaveContext.save.saveInfo.inventory.strayFairies[dungeon] >= STRAY_FAIRY_SCATTERED_TOTAL);
        DUNGEON_KEY_COUNT(dungeon) =
            clamp_s32(recompui_get_input_value_u32(dungeon_key_sliders[i]), 0, s_dungeon_key_max[i]);
    }

    clamp_live_save();
    gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));
    sync_editor_from_save();
}

static void set_all_quest_items(bool enabled) {
    set_quest_flag(QUEST_BOMBERS_NOTEBOOK, enabled);
    for (s32 i = 0; i < ARRAY_COUNT(s_remains); i++) {
        set_quest_flag(s_remains[i], enabled);
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_songs); i++) {
        set_quest_flag(s_songs[i], enabled);
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_tingle_maps); i++) {
        set_tingle_map(&s_tingle_maps[i], enabled);
    }
    set_heart_piece_count(enabled ? 3 : 0);
    set_skull_tokens(enabled ? 30 : 0, enabled ? 30 : 0);
}

static void set_all_dungeon_items(bool enabled) {
    for (s32 i = 0; i < ARRAY_COUNT(s_dungeons); i++) {
        s32 dungeon = s_dungeons[i];
        set_dungeon_flag(dungeon, DUNGEON_MAP, enabled);
        set_dungeon_flag(dungeon, DUNGEON_COMPASS, enabled);
        set_dungeon_flag(dungeon, DUNGEON_BOSS_KEY, enabled);
        set_dungeon_flag(dungeon, DUNGEON_STRAY_FAIRIES, enabled);
        gSaveContext.save.saveInfo.inventory.strayFairies[dungeon] = enabled ? STRAY_FAIRY_SCATTERED_TOTAL : 0;
        DUNGEON_KEY_COUNT(dungeon) = enabled ? s_dungeon_key_max[i] : 0;
    }
}

static RecompuiResource add_row(void) {
    RecompuiResource row = recompui_create_element(editor_context, content_parent);
    recompui_set_display(row, DISPLAY_FLEX);
    recompui_set_flex_direction(row, FLEX_DIRECTION_ROW);
    recompui_set_align_items(row, ALIGN_ITEMS_CENTER);
    recompui_set_gap(row, 16.0f, UNIT_DP);
    recompui_set_width(row, 100.0f, UNIT_PERCENT);
    recompui_set_min_height(row, 54.0f, UNIT_DP);
    recompui_set_flex_shrink(row, 0.0f);
    return row;
}

static void add_section_label(const char* label) {
    RecompuiResource section = recompui_create_label(editor_context, content_parent, label, LABELSTYLE_NORMAL);
    recompui_set_margin_top(section, 14.0f, UNIT_DP);
    recompui_set_min_height(section, 48.0f, UNIT_DP);
    recompui_set_flex_shrink(section, 0.0f);
}

static RecompuiResource add_page(void) {
    RecompuiResource page = recompui_create_element(editor_context, panel);
    recompui_set_display(page, DISPLAY_FLEX);
    recompui_set_flex_direction(page, FLEX_DIRECTION_COLUMN);
    recompui_set_width(page, 100.0f, UNIT_PERCENT);
    recompui_set_height(page, 520.0f, UNIT_DP);
    recompui_set_min_height(page, 520.0f, UNIT_DP);
    recompui_set_overflow_y(page, OVERFLOW_AUTO);
    recompui_set_gap(page, 8.0f, UNIT_DP);
    recompui_set_flex_shrink(page, 0.0f);
    return page;
}

static RecompuiResource add_labeled_slider(const char* label, float min, float max, float step, float initial) {
    RecompuiResource row = add_row();
    RecompuiResource label_res = recompui_create_label(editor_context, row, label, LABELSTYLE_SMALL);
    RecompuiResource slider = recompui_create_slider(editor_context, row, SLIDERTYPE_INTEGER, min, max, step, initial);
    recompui_set_width(label_res, 220.0f, UNIT_DP);
    recompui_set_min_width(label_res, 220.0f, UNIT_DP);
    recompui_set_flex_shrink(label_res, 0.0f);
    recompui_set_width(slider, 460.0f, UNIT_DP);
    recompui_set_min_width(slider, 420.0f, UNIT_DP);
    recompui_set_flex_shrink(slider, 1.0f);
    return slider;
}

static RecompuiResource add_labeled_radio(const char* label, const char** options, unsigned long option_count) {
    RecompuiResource row = add_row();
    RecompuiResource label_res = recompui_create_label(editor_context, row, label, LABELSTYLE_SMALL);
    RecompuiResource radio = recompui_create_labelradio(editor_context, row, options, option_count);
    recompui_set_width(label_res, 220.0f, UNIT_DP);
    recompui_set_min_width(label_res, 220.0f, UNIT_DP);
    recompui_set_flex_shrink(label_res, 0.0f);
    recompui_set_flex_shrink(radio, 0.0f);
    return radio;
}

static RecompuiResource add_page_button_row(void) {
    RecompuiResource row = recompui_create_element(editor_context, content_parent);
    recompui_set_display(row, DISPLAY_FLEX);
    recompui_set_flex_direction(row, FLEX_DIRECTION_ROW);
    recompui_set_align_items(row, ALIGN_ITEMS_CENTER);
    recompui_set_gap(row, 16.0f, UNIT_DP);
    recompui_set_width(row, 100.0f, UNIT_PERCENT);
    recompui_set_min_height(row, 72.0f, UNIT_DP);
    recompui_set_flex_shrink(row, 0.0f);
    return row;
}

static void editor_button_pressed(RecompuiResource resource, const RecompuiEventData* data, void* userdata) {
    if (data->type != UI_EVENT_CLICK) {
        return;
    }

    if (resource == general_page_button) {
        set_active_page(0);
    } else if (resource == stats_page_button) {
        set_active_page(1);
    } else if (resource == equipment_page_button) {
        set_active_page(2);
    } else if (resource == quest_page_button) {
        set_active_page(3);
    } else if (resource == dungeons_page_button) {
        set_active_page(4);
    } else if (resource == apply_button) {
        apply_editor_to_save();
    } else if (resource == give_quest_button) {
        set_all_quest_items(true);
        gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));
        sync_editor_from_save();
    } else if (resource == reset_quest_button) {
        set_all_quest_items(false);
        gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));
        sync_editor_from_save();
    } else if (resource == give_dungeons_button) {
        set_all_dungeon_items(true);
        gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));
        sync_editor_from_save();
    } else if (resource == reset_dungeons_button) {
        set_all_dungeon_items(false);
        gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));
        sync_editor_from_save();
    } else if (resource == close_button) {
        recompui_hide_context(editor_context);
        editor_shown = false;
    }
}

RECOMP_CALLBACK("*", recomp_on_init)
void save_editor_on_init(void) {
    RecompuiColor bg_color = { 255, 255, 255, 26 };
    RecompuiColor border_color = { 255, 255, 255, 51 };
    RecompuiColor panel_color = { 8, 7, 13, 230 };
    const char* bool_options[] = { "No", "Yes" };
    const char* magic_options[] = { "None", "Single", "Double" };
    const char* spin_options[] = { "Level 1", "Level 2" };
    const char* wallet_options[] = { "Child", "Adult", "Giant" };
    const char* sword_options[] = { "None", "Kokiri", "Razor", "Gilded" };
    const char* shield_options[] = { "None", "Hero", "Mirror" };
    const char* quiver_options[] = { "None", "30", "40", "50" };
    const char* bomb_bag_options[] = { "None", "20", "30", "40" };
    const char* stick_options[] = { "None", "10", "20", "30" };
    const char* nut_options[] = { "None", "20", "30", "40" };
    const char* remain_names[] = { "Odolwa", "Goht", "Gyorg", "Twinmold" };
    const char* song_names[] = {
        "Sonata", "Lullaby", "Bossa Nova", "Elegy", "Oath", "Saria",
        "Time", "Healing", "Epona", "Soaring", "Storms", "Sun"
    };
    const char* dungeon_names[] = { "Woodfall", "Snowhead", "Great Bay", "Stone Tower" };
    const char* bottle_content_names[] = {
        "None", "Empty", "Red Potion", "Green Potion", "Blue Potion", "Chateau",
        "Spring Water", "Hot Spring Water", "Milk", "Half Milk", "Fairy", "Fish",
        "Bugs", "Poe", "Big Poe", "Zora Egg", "Deku Princess", "Gold Dust",
        "Mushroom", "Seahorse", "Blue Fire", "Hylian Loach"
    };

    editor_context = recompui_create_context();
    recompui_open_context(editor_context);
    recompui_set_context_captures_input(editor_context, true);
    recompui_set_context_captures_mouse(editor_context, true);

    root = recompui_context_root(editor_context);
    recompui_set_position(root, POSITION_ABSOLUTE);
    recompui_set_top(root, 0, UNIT_DP);
    recompui_set_right(root, 0, UNIT_DP);
    recompui_set_bottom(root, 0, UNIT_DP);
    recompui_set_left(root, 0, UNIT_DP);
    recompui_set_width_auto(root);
    recompui_set_height_auto(root);
    recompui_set_padding(root, 64.0f, UNIT_DP);
    recompui_set_background_color(root, &bg_color);
    recompui_set_display(root, DISPLAY_FLEX);
    recompui_set_flex_direction(root, FLEX_DIRECTION_COLUMN);
    recompui_set_justify_content(root, JUSTIFY_CONTENT_CENTER);
    recompui_set_align_items(root, ALIGN_ITEMS_CENTER);

    panel = recompui_create_element(editor_context, root);
    recompui_set_display(panel, DISPLAY_FLEX);
    recompui_set_flex_direction(panel, FLEX_DIRECTION_COLUMN);
    recompui_set_width(panel, 100.0f, UNIT_PERCENT);
    recompui_set_max_width(panel, 1160.0f, UNIT_DP);
    recompui_set_height(panel, 900.0f, UNIT_DP);
    recompui_set_overflow_y(panel, OVERFLOW_HIDDEN);
    recompui_set_padding(panel, 20.0f, UNIT_DP);
    recompui_set_gap(panel, 12.0f, UNIT_DP);
    recompui_set_border_width(panel, 1.0f, UNIT_DP);
    recompui_set_border_radius(panel, 16.0f, UNIT_DP);
    recompui_set_border_color(panel, &border_color);
    recompui_set_background_color(panel, &panel_color);

    recompui_create_label(editor_context, panel, "Save Editor", LABELSTYLE_LARGE);

    {
        RecompuiResource nav_row = recompui_create_element(editor_context, panel);
        recompui_set_display(nav_row, DISPLAY_FLEX);
        recompui_set_flex_direction(nav_row, FLEX_DIRECTION_ROW);
        recompui_set_gap(nav_row, 12.0f, UNIT_DP);
        recompui_set_min_height(nav_row, 72.0f, UNIT_DP);
        recompui_set_flex_shrink(nav_row, 0.0f);
        general_page_button = recompui_create_button(editor_context, nav_row, "Main", BUTTONSTYLE_SECONDARY);
        stats_page_button = recompui_create_button(editor_context, nav_row, "Stats", BUTTONSTYLE_SECONDARY);
        equipment_page_button = recompui_create_button(editor_context, nav_row, "Gear", BUTTONSTYLE_SECONDARY);
        quest_page_button = recompui_create_button(editor_context, nav_row, "Quest", BUTTONSTYLE_SECONDARY);
        dungeons_page_button = recompui_create_button(editor_context, nav_row, "Dungeons", BUTTONSTYLE_SECONDARY);
        recompui_register_callback(general_page_button, editor_button_pressed, NULL);
        recompui_register_callback(stats_page_button, editor_button_pressed, NULL);
        recompui_register_callback(equipment_page_button, editor_button_pressed, NULL);
        recompui_register_callback(quest_page_button, editor_button_pressed, NULL);
        recompui_register_callback(dungeons_page_button, editor_button_pressed, NULL);
    }

    page_general = add_page();
    page_stats = add_page();
    page_equipment = add_page();
    page_quest = add_page();
    page_dungeons = add_page();

    content_parent = page_general;
    add_section_label("General");
    {
        RecompuiResource row = add_row();
        RecompuiResource label = recompui_create_label(editor_context, row, "Name", LABELSTYLE_SMALL);
        recompui_set_width(label, 220.0f, UNIT_DP);
        recompui_set_min_width(label, 220.0f, UNIT_DP);
        recompui_set_flex_shrink(label, 0.0f);
        name_input = recompui_create_textinput(editor_context, row);
        recompui_set_width(name_input, 360.0f, UNIT_DP);
    }
    day_slider = add_labeled_slider("Day", 1.0f, 4.0f, 1.0f, 1.0f);
    time_slider = add_labeled_slider("Time", 0.0f, 65535.0f, 1.0f, 0.0f);
    time_speed_slider = add_labeled_slider("Time Speed Offset", -2.0f, 18.0f, 1.0f, 0.0f);
    {
        RecompuiResource row = add_row();
        RecompuiResource label = recompui_create_label(editor_context, row, "Bombers Code", LABELSTYLE_SMALL);
        recompui_set_width(label, 220.0f, UNIT_DP);
        recompui_set_min_width(label, 220.0f, UNIT_DP);
        recompui_set_flex_shrink(label, 0.0f);
        bomber_code_input = recompui_create_textinput(editor_context, row);
        recompui_set_width(bomber_code_input, 180.0f, UNIT_DP);
    }
    tatl_radio = add_labeled_radio("Has Tatl", bool_options, ARRAY_COUNT(bool_options));
    intro_radio = add_labeled_radio("Intro Complete", bool_options, ARRAY_COUNT(bool_options));
    owl_save_radio = add_labeled_radio("Owl Save", bool_options, ARRAY_COUNT(bool_options));

    add_section_label("Currency");
    wallet_radio = add_labeled_radio("Wallet", wallet_options, ARRAY_COUNT(wallet_options));
    rupees_slider = add_labeled_slider("Rupees", 0.0f, 500.0f, 1.0f, 0.0f);
    bank_slider = add_labeled_slider("Bank Rupees", 0.0f, MAX_BANK_RUPEES, 1.0f, 0.0f);

    content_parent = page_stats;
    add_section_label("Health and Magic");
    hearts_slider = add_labeled_slider("Heart Containers", MIN_HEARTS, MAX_HEARTS, 1.0f, MIN_HEARTS);
    heart_piece_slider = add_labeled_slider("Heart Pieces", 0.0f, 3.0f, 1.0f, 0.0f);
    health_slider = add_labeled_slider("Health", 0.0f, MAX_HEARTS * 0x10, 1.0f, MIN_HEARTS * 0x10);
    double_defense_radio = add_labeled_radio("Double Defense", bool_options, ARRAY_COUNT(bool_options));
    magic_level_radio = add_labeled_radio("Magic Level", magic_options, ARRAY_COUNT(magic_options));
    magic_slider = add_labeled_slider("Magic", 0.0f, SAVE_EDITOR_MAGIC_DOUBLE_METER, 1.0f, 0.0f);
    spin_attack_radio = add_labeled_radio("Spin Attack", spin_options, ARRAY_COUNT(spin_options));
    chateau_radio = add_labeled_radio("Chateau Romani", bool_options, ARRAY_COUNT(bool_options));

    content_parent = page_equipment;
    add_section_label("Equipment and Ammo");
    sword_radio = add_labeled_radio("Sword", sword_options, ARRAY_COUNT(sword_options));
    shield_radio = add_labeled_radio("Shield", shield_options, ARRAY_COUNT(shield_options));
    quiver_radio = add_labeled_radio("Quiver", quiver_options, ARRAY_COUNT(quiver_options));
    arrows_slider = add_labeled_slider("Arrows", 0.0f, 50.0f, 1.0f, 0.0f);
    bomb_bag_radio = add_labeled_radio("Bomb Bag", bomb_bag_options, ARRAY_COUNT(bomb_bag_options));
    bombs_slider = add_labeled_slider("Bombs", 0.0f, 40.0f, 1.0f, 0.0f);
    bombchus_slider = add_labeled_slider("Bombchus", 0.0f, 40.0f, 1.0f, 0.0f);
    stick_upgrade_radio = add_labeled_radio("Stick Upgrade", stick_options, ARRAY_COUNT(stick_options));
    sticks_slider = add_labeled_slider("Deku Sticks", 0.0f, 30.0f, 1.0f, 0.0f);
    nut_upgrade_radio = add_labeled_radio("Nut Upgrade", nut_options, ARRAY_COUNT(nut_options));
    nuts_slider = add_labeled_slider("Deku Nuts", 0.0f, 40.0f, 1.0f, 0.0f);
    powder_keg_radio = add_labeled_radio("Powder Keg", bool_options, ARRAY_COUNT(bool_options));
    magic_beans_radio = add_labeled_radio("Magic Beans", bool_options, ARRAY_COUNT(bool_options));
    magic_beans_count_slider = add_labeled_slider("Bean Count", 0.0f, 20.0f, 1.0f, 0.0f);
    add_section_label("Tingle Maps");
    for (s32 i = 0; i < ARRAY_COUNT(s_tingle_maps); i++) {
        tingle_map_radios[i] = add_labeled_radio(s_tingle_maps[i].label, bool_options, ARRAY_COUNT(bool_options));
    }
    add_section_label("Bottles");
    for (s32 i = 0; i < ARRAY_COUNT(s_bottle_slots); i++) {
        bottle_radios[i] = add_labeled_radio(s_bottle_slots[i].label, bottle_content_names, ARRAY_COUNT(bottle_content_names));
    }
    add_section_label("Items");
    for (s32 i = 0; i < ARRAY_COUNT(s_item_toggles); i++) {
        item_radios[i] = add_labeled_radio(s_item_toggles[i].label, bool_options, ARRAY_COUNT(bool_options));
    }
    add_section_label("Masks");
    for (s32 i = 0; i < ARRAY_COUNT(s_mask_toggles); i++) {
        mask_radios[i] = add_labeled_radio(s_mask_toggles[i].label, bool_options, ARRAY_COUNT(bool_options));
    }

    content_parent = page_quest;
    add_section_label("Quest Status");
    {
        RecompuiResource quest_actions = add_page_button_row();
        give_quest_button = recompui_create_button(editor_context, quest_actions, "Max Quest", BUTTONSTYLE_SECONDARY);
        reset_quest_button = recompui_create_button(editor_context, quest_actions, "Reset Quest", BUTTONSTYLE_SECONDARY);
    }
    notebook_radio = add_labeled_radio("Bombers Notebook", bool_options, ARRAY_COUNT(bool_options));
    swamp_tokens_slider = add_labeled_slider("Swamp Skull Tokens", 0.0f, 30.0f, 1.0f, 0.0f);
    ocean_tokens_slider = add_labeled_slider("Ocean Skull Tokens", 0.0f, 30.0f, 1.0f, 0.0f);
    for (s32 i = 0; i < ARRAY_COUNT(s_remains); i++) {
        remains_radios[i] = add_labeled_radio(remain_names[i], bool_options, ARRAY_COUNT(bool_options));
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_songs); i++) {
        song_radios[i] = add_labeled_radio(song_names[i], bool_options, ARRAY_COUNT(bool_options));
    }

    content_parent = page_dungeons;
    add_section_label("Dungeons");
    {
        RecompuiResource dungeon_actions = add_page_button_row();
        give_dungeons_button = recompui_create_button(editor_context, dungeon_actions, "All Dungeons", BUTTONSTYLE_SECONDARY);
        reset_dungeons_button = recompui_create_button(editor_context, dungeon_actions, "Reset Dungeons", BUTTONSTYLE_SECONDARY);
    }
    for (s32 i = 0; i < ARRAY_COUNT(s_dungeons); i++) {
        add_section_label(dungeon_names[i]);
        dungeon_map_radios[i] = add_labeled_radio("Map", bool_options, ARRAY_COUNT(bool_options));
        dungeon_compass_radios[i] = add_labeled_radio("Compass", bool_options, ARRAY_COUNT(bool_options));
        dungeon_boss_key_radios[i] = add_labeled_radio("Boss Key", bool_options, ARRAY_COUNT(bool_options));
        dungeon_key_sliders[i] = add_labeled_slider("Small Keys", 0.0f, s_dungeon_key_max[i], 1.0f, 0.0f);
        dungeon_fairy_sliders[i] = add_labeled_slider("Stray Fairies", 0.0f, 15.0f, 1.0f, 0.0f);
    }

    set_active_page(0);

    button_row = recompui_create_element(editor_context, panel);
    recompui_set_display(button_row, DISPLAY_FLEX);
    recompui_set_flex_direction(button_row, FLEX_DIRECTION_ROW);
    recompui_set_align_items(button_row, ALIGN_ITEMS_CENTER);
    recompui_set_gap(button_row, 16.0f, UNIT_DP);
    recompui_set_width(button_row, 100.0f, UNIT_PERCENT);
    recompui_set_min_height(button_row, 72.0f, UNIT_DP);
    recompui_set_flex_shrink(button_row, 0.0f);
    apply_button = recompui_create_button(editor_context, button_row, "Apply", BUTTONSTYLE_PRIMARY);
    close_button = recompui_create_button(editor_context, button_row, "Close", BUTTONSTYLE_SECONDARY);

    recompui_register_callback(apply_button, editor_button_pressed, NULL);
    recompui_register_callback(close_button, editor_button_pressed, NULL);
    recompui_register_callback(give_quest_button, editor_button_pressed, NULL);
    recompui_register_callback(reset_quest_button, editor_button_pressed, NULL);
    recompui_register_callback(give_dungeons_button, editor_button_pressed, NULL);
    recompui_register_callback(reset_dungeons_button, editor_button_pressed, NULL);

    recompui_close_context(editor_context);
}

RECOMP_HOOK("Play_UpdateMain")
void save_editor_on_play_update(PlayState* play) {
    Input* input = CONTROLLER1(&play->state);
    u16 hotkey = editor_hotkey_combo();

    current_play_state = play;

    if (editor_hotkey_pressed(input, hotkey)) {
        if (!editor_shown) {
            sync_editor_from_save_closed_context();
            recompui_show_context(editor_context);
            editor_shown = true;
        } else {
            recompui_hide_context(editor_context);
            editor_shown = false;
        }
    }
}
