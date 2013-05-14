/*
  +----------------------------------------------------------------------+
  | See COPYING file for further copyright information                   |
  +----------------------------------------------------------------------+ 
  | Author: Oleg Grenrus <oleg.grenrus@dynamoid.com>                     |
  | See CREDITS for contributors                                         |
  +----------------------------------------------------------------------+ 
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_odus.h"
#include "odwrapper.h"
#include "od_igbinary.h"

ZEND_EXTERN_MODULE_GLOBALS(odus);

/* Static Variables. */

/* Static strings. */
static struct hash_si od_static_strings_hash;	// For serialize
static int od_static_strings_count = 0;

static char* od_static_strings[] = {
		"x",
		"y",
		"z",
		"static",
		"Road",
		"road",
		"Sidewalk",
		"grown",
		"deco_citysidewalk",
		"material_landfill_grass",
		"material_landfill_sand",
		"Decoration",
		"viral",
		"limitPosts",
		"undefined",
		"Residence",
		"population_value_increase",
		"name",
		"time",
		"type",
		"cnt",
		"money_type",
		"cost",
		"build",
		"ts",
		"cash",
		"Municipal",
		"Business",
		"feed",
		"count",
		"dynamic_tuning_tier",
		"width",
		"height",
		"ConstructionSite",
		"farm",
		"stamp",
		"data",
		"expand_12_12",
		"pending",
		"level",
		"senderID",
		"recipientID",
		"dynamic",
		"inventory",
		"goods",
		"amount",
		"status",
		"closed",
		"order_visitor_help",
		"sent",
		"bonusMultiplier",
		"orderType",
		"timeSent",
		"transmissionStatus",
		"orderState",
		"helpTargets",
		"Residence or Crop",
		"Instant Ready",
		"num_click",
		"Plot",
		"macroObject",
		"parentMacroObjectId",
		"deco_gardensidewalk",
		"EnvironmentTile",
		"request",
		"gaveMastery",
		"key",
		"zid",
		"item",
		"plot_crop",
		"start",
		"socialInventory",
		"landfill",
		"plowed",
		"closedHarvestable",
		"rejected",
		"coin",
		"construction_6x6_4stage_tile",
		"itemName",
		"harvestState",
		"Wilderness",
		"state",
		"construction_1x1_4stage_tile",
		"coins",
		"null_FeatureData",
		"sig",
		"expiration_date",
		"exp_date",
		"accepted",
		"deco_cobblestonesidewalk",
		"upgradeHelpers",
		"deco_italianpiazza_tile",
		"road-2",
		"slots",
		"deco_redsidewalk",
		"buildables",
		"xp",
		"construction_3x3_4stage_tile",
		"state_patrolling",
		"targetObjectName",
		"id",
		"className",
		"state",
		"itemName",
		"direction",
		"position",
		"mechanicData",
		"itemOwner",
		"upgradeActionCount",
		"upgradeTime",
		"currentPopulation",
		"crewReplacements",
		"bonusCrewConversions",
		"plantTime",
		"buildTime",
		"buildPowerups",
		"harvestCounter",
		"name",
		"giftSenderId",
		"lastModifiedGlobalEngineTime",
		"m_value",
		"isConnectedToRoad",
		"isRoadVerifiable",
		"heliportActive",
		"visits",
		"downgradeFromItemName",
		"neverOpened",
		"lastSavedMax",
		"type",
		"keys",
		"virals",
		"loadType",
		"removeAllOnComplete",
		"removeAllOnCompleteSet",
		"keepOnComplete",
		"removeOnCompleteItems",
		"targetObjectId",
		"unlocked",
		"objectName",
		"overrideItemName",
		"overrideItemValidatorName",
		"masteryLevel",
		"validator",
		"cashCost",
		"overrideTables",
		"dynamicTuning",
		"disableSaleDiscount",
		"gates",
		"builds",
		"placementTime",
		"finishedBuilds",
		"stage",
		"buildStartTime",
		"buildsQueued",
		"currentState",
		"targetBuildingName",
		"targetBuildingClass",
		"isAccelerated",
		"numBoosts",
		"refund",
		"contractName",
		"rewards",
		"storage",
		"supplyStorage",
		"m_contractName",
		"isMoveLocked",
		"canBeOwned",
		"mapOwner",
		"userId",
		"helpers",
		"timeRequested",
		"rewardType",
		"rewardHandler",
		"rewardAltType",
		"trackFriends",
		"lastStage",
		"lastVisitDeduction",
		"clearanceCustomers",
		"featureData",
		"maxObjectID",
		"franchise_info",
		"objects",
		"objectsIndexByGroup",
		"macroObjects",
		"macroObjectIdCounter",
		"sizeX",
		"sizeY",
		"gridSize",
		"mapRects",
		"lastExpansionTier",
		"mostFrequentHelpers",
		"visitorCenterMessage",
		"tileSet",
		"versionFromDB",
		"currentThemes",
		"currentThemeCollections",
		"storageData",
		"loadedEmpty",
		"worldCreated",
		"tempToIdMap",
		"tempToIdMapTime",
		"v",
		"wildernessSim",
		"citySim",
		"populationSummary",
		"appraisalSummary",
		"world_id",
		"ownerId",
		"lastDirtyTimestamp",
		"m_storage",
		"segments",
		"yield",
		"capacity",
		"potential",
		"nextAppraisalRequirement",
		"nextAppraisalRequirementBase",
		"level",
		"shamrockList",
		"upgradeActionCount2",
		"m_capacity",
		"m_maxCapacity",
		"viralType",
		"originTime",
		"timesPosted",
		"hostRewardsGiven",
		"recipients",
		"ver",
		"senderId",
		"senderName",
		"object",
		"timestamp",
		"Road",
		"Sidewalk",
		"Flag",
		"Decoration",
		"Residence",
		"Municipal",
		"Business",
		"ConstructionSite",
		"InventoryGate",
		"Plot",
		"SupplyStorage",
		"Wilderness",
		"FriendReward",
		"Storage",
		"Ship",
		"MechanicMapResource",
		"Neighborhood",
		"EnvironmentTile",
		"CrewGate",
		"PartnerBuild",
		"World",
		"CitySim",
		"PopulationSummary",
		"AppraisalSummary",
		"Mall",
		"Skyscraper",
		"HarvestableShip",
		"BridgePart",
		"Airplane",
		"RainbowDecoration",
		"MasteryGate",
		"CompositeGate",
		"GreenHouse",
		"MunicipalCenter",
		"BaseStorageUnit",
		"Viral",
		"PlayerNews",
		"Landmark",
		"Hotel",
		"ZooEnclosure",
		"Gate",
		"PowerStation",
		"Pier",
		"TrainTracks",
		"Truck",
		"TimedBusiness",
		"Storage",
		"complete",
		"planted",
		"no_table",
		"lastRequest",
		"lastLevel",
		"lastCount",
		"materials",
		"deco_sidewalk_greydark",
		"mastery_crop",
		"id",
		"scid",
		"plot_pumpkin_unwither",
		"collected",
		"checkedIn",
		"purchasedCheckIn",
		"feedSent",
		"mastery_business",
		"residence",
		"reward",
		"timeHelped",
		"endTS",
		"progress",
		"prize",
		"index",
		"loot",
		"handler",
		"deco_courtlight",
		"item_quantity",
		"energy",
		"MechanicMapResource",
		"tool",
		"resources",
		"deco_bikepath",
		"score",
		"claim",
		"rank",
		"highest",
		"purchased",
		"extraData",
		"activatedTime",
		"expired",
		"expireDate",
		"tree_shade",
		"workers",
		"cops",
		"attributes",
		"members",
		"timeLastRoll",
		"banditQueue",
		"banditsSpawned",
		"numBanditsCaptured",
		"typesBanditsCaptured",
		"gateUnlockLevel",
		"staffs",
		"expand_12_12_special",
		"grindable",
		"deco_citysidewalk_district",
		"Ship",
		"material_lakefront_rivet",
		"plot_strawberries",
		"table",
		"keyword",
		"terra_tile_grass_land_6x6",
		"material_lakefront_axle",
		"permits",
		"TRUE",
		"material_lakefront_porticullus",
		"material_lakefront_windlass",
		"material_lakefront_deck",
		"deco_flowerPatchBlue",
		"material_lakefront_counterweight",
		"deco_cityfence01",
		"terra_tile_grass_land_6x6_reskin",
		"deco_walkoffame",
		"citizen",
		"openTS",
		"res_france_modern_bordeaux_house",
		"terra_tile_grass_land_1x1",
		"deco_greek_sidewalk",
		"isNew",
		"remodel",
		"terra_tile_grass_land_1x1_reskin",
		"withered",
		"Neighborhood",
		"plot_water",
		"resourceType",
		"constructionCount",
		"orderResourceName",
		"lotId",
		"offsetX",
		"offsetY",
		"res_beachfrontapt_a",
		"crew_bonus",
		"customers",
		"customersReq",
		"contractName",
		"premium_goods",
		"none",
		"toaster",
		"parentMacroItemName",
		"children",
		"rewardGranted",
		"macro_airport_0",
		"required_crew",
		"crew",
		"harvest_bonus",
		"deco_flowerPatch",
		"plot_peagold",
		"capacity",
		"material_floodgate",
		"material_generator",
		"material_hydraulicturbine",
		"500",
		"material_canallock",
		"material_rotor",
		"contract",
		"terra_tile_grass_land_3x3",
		"mastery",
		"helpers",
		"material_alps_signal_lights",
		"material_alps_rail_track",
		"material_alps_tourist_train_ticket",
		"material_alps_ticket_punch",
		"res_german_marketplatzhouse02",
		"res_scottishtownhouse",
		"terra_tile_grass_land_3x3_reskin",
		"PeepCapacityResource",
		"material_alps_tourist_platform",
		"material_alps_schedule_board",
		"material_green_river_shamrock_dust",
		"material_green_river_arch_stones",
		"expansion",
		"deco_greek_stonesidewalk",
		"material_green_river_magic_shamrock",
		"player_love",
		"material_green_river_wedge_stones",
		"material_green_river_gold_pots",
		"state_sleeping",
		"24",
		"material_green_river_green_ivy",
		"plot_corn",
		"awardUserCoins",
		"deco_braziliansidewalk",
		"isCandidate",
		"Mall",
		"material_green_shamrock",
		"upgrade_status",
		"PartnerBuild",
		"upgrade_buildable",
		"awardUserCommodities",
		"60",
		"ship_boat",
		"world_main",
		"deco_amusementsidewalk",
		"material_majestic_roomkey",
		"deco_metro_mapleamur",
		"municipal",
		"deco_cityfence02",
		"harvestEndTS",
		"plot_peanuts",
		"material_red_shamrock",
		"business",
		"minimum",
		"yield",
		"maximum",
		"potential",
		"res_bnb",
		"obj_id",
		"res_hobbithouse",
		"material_majestic_waterfall",
		"energy_1",
		"star_rating",
		"commodity_left",
		"money_collected",
		"time_last_collected",
		"customers_served",
		"franchise_name",
		"deco_clover",
		"time_last_operated",
		"timestamp",
		"material_orange_shamrock",
		"material_majestic_velvetrope",
		"material_majestic_icebucket",
		"res_cliffhousesmall",
		"material_majestic_pillowchocolate",
		"deco_sweetgumtree",
		"copsNBandits",
		"material_majestic_sharktank",
		"buildingLevel",
		"open",
		"wilderness_28",
		"material_dtwn_building_permit",
		"deco_killerwhale",
		"alwaysCheck",
		"road-4",
		"material_yellow_shamrock",
		"received",
		"material_dtwn_press_pass",
		"material_dtwn_computer",
		"material_dtwn_notepad",
		"res_countrystucco",
		"storage_shack",
		"balls_tossed",
		"time_last_supplied",
		"plot_cranberries",
		"PowerStation",
		"deco_wackysidewalk_blue",
		"res_brownstone",
		"material_blue_shamrock",
		"material_dtwn_printer",
		"res_simpsonmegabalcony",
		"deco_farmers_sidewalk",
		"material_purple_shamrock",
		"Hotel",
		"giftSenders",
		"smartnpcs",
		"upgradeActionCount",
		"missions",
		"completed",
		"upgrade_inventory",
		"deco_wackysidewalk_teal",
		"ZooEnclosure",
		"plot_blackberries",
		"animalRescue",
		"municipal_material_landfill_grass",
		"material_waterfall_powerstation_turbine",
		"material_waterfall_powerstation_condenser",
		"material_waterfall_powerstation_heater",
		"res_spanishapt2",
		"plot_grapescabernet",
		"res_italianapts",
		"Skyscraper",
		"material_snow_frozenLandfill",
		"material_waterfall_powerstation_generator",
		"res_cottage3",
		"airports_v4_repair",
		"gas",
		"version",
		"material_waterfall_powerstation_transformer",
		"HarvestableShip",
		"justdessertscollection",
		"28",
		"wilderness_29",
		"peacefullivingcollection",
		"deco_flowerPatchOrange",
		"MunicipalCenter",
		"visitors",
		"BridgePart",
		"deco_2",
		"GreenHouse",
		"Airplane",
		"get_user_preferred_currency",
		"material_ice_frozenLandfill",
		"res_mansion",
		"cars",
		"RainbowDecoration",
		"deco_sandwalk",
		"plot_carrots",
		"deco_boardwalk",
		"material_icekey_frozenLandfill",
		"res_apartments",
		"ruralcollection",
		"storage",
		"world_lakefront",
		"house3",
		"material_marble",
		"deco_blueblossomtree",
		"material_metal_rebar",
		"material_solar_street_light",
		"deco_winter11_redwoodbigtree",
		"completed_tutorial",
		"plot_carrotgold",
		"boardState",
		"rewardsEarned",
		"numPlays",
		"freeTurns",
		"strawberrycollection",
		"gardens",
		"qf_embassy_spain",
		"pre_build",
		"completed_bridge",
		"heart",
		"deco_coconuttree",
		"res_villa",
		"deco_fencecracker",
		"deco_ironfence04",
		"plot_pumpkin",
		"reward_pending",
		"mun_policestation",
		"closeTime",
		"composite",
		"gamecollection",
		"construction_5x5_12stage",
		"w115",
		"material_shrinkray_charge",
		"deco_clover_flowers",
		"w112",
		"w114",
		"suburbiacollection",
		"w111",
		"deco_plazaflowers",
		"res_springsiding",
		"w113",
		"deco_wackysidewalk_gold",
		"w110",
		"drawbridge_construction",
		"w109",
		"w116",
		"plot_crab",
		"material_santasleigh_sleigh_skids",
		"w108",
		"earlyrisercollection",
		"material_city_seal",
		"w106",
		"macro_truckingdepot_0",
		"greenHouseStorage",
		"w107",
		"w92",
		"w94",
		"w89",
		"w91",
		"storage_silo",
		"w105",
		"res_aptcomplex",
		"w90",
		"w100",
		"construction_5x5_10stage",
		"w96",
		"w93",
		"w104",
		"w117",
		"w95",
		"res_upscalebrick",
		"mun_policestation_7",
		"mun_area51",
		"area51",
		"w101",
		"w99",
		"w97",
		"material_shrinkray_reflector",
		"multiplier",
		"w102",
		"globalTable",
		"material_toiletries",
		"w98",
		"plot_candycanes",
		"w103",
		"construction_4x4_7stage",
		"deco_deciduous_tree_A",
		"w120",
		"goal",
		"w118",
		"deco_springflowerpatch04",
		"plot_grapesgreen",
		"tree_willow",
		"quest_item_holiday_spirit",
		"material_ironing_board",
		"material_clothing_iron",
		"41",
		"construction_5x5_8stage",
		"fake_loot_table_limit_hit",
		"greenHouseHarvest",
		"material_shrinkray_batteries",
		"w119",
		"material_santasleigh_present_sacks",
		"w88",
		"res_stiltbungalow",
		"lastSpawnTime",
		"decoration",
		"Garden",
		"unclaimed",
		"material_shrinkray_dish",
		"material_shrinkray_capacitor",
		"deco_springflowerpatch03",
		"deco_zoojungletree",
		"material_santasleigh_reindeer_harness",
		"material_shrinkray_resistor",
		"tickets",
		"material_pool_towel",
		"material_golden_ticket",
		"material_pool_chair",
		"Landmark",
		"plot_coinpineapple",
		"material_dtwn_umbrella_table",
		"garden_roses_4x4",
		"w87",
		"class",
		"material_santasleigh_sleigh_frame",
		"plot_raspberries",
		"itemCounts",
		"w86",
		"matchmaking",
		"material_dtwn_statue_riderhorse",
		"fastfoodcollection",
		"world_downtown",
		"material_dtwn_ticket_booth",
		"material_beachtowel",
		"gardencollection",
		"material_dtwn_park_shrubs",
		"plot_sugarcane",
		"material_shrinkray_beam",
		"PopulationValue",
		"Pier",
		"corncollection",
		"stpattyscollection",
		"v",
		"material_shrinkray_wiring",
		"plot_prem_rasberies",
		"q_international_saga_act2_1_1",
		"material_santasleigh_gps",
		"plot_babycarrot",
		"deco_baretree",
		"w121",
		"active_value",
		"upgrade_aoe",
		"material_shrinkray_inductor",
		"startTime",
		"bus_bakery",
		"material_santasleigh_reins",
		"wilderness_09",
		"material_dtwn_palm_tree",
		"treasureHunts",
		"material_honeymoon_rigging",
		"w85",
		"IceSculpturesLeaderboard",
		"samObjectIds",
		"initialized",
		"w172",
		"w47",
		"mun_downtownpolicestation",
		"w173",
		"citylifecollection",
		"w179",
		"w180",
		"w168",
		"MostActive",
		"plantTime",
		"plot_watermelon",
		"w174",
		"incentivizedExpansions",
		"w181",
		"hotelVisitClosed",
		"w177",
		"w184",
		"w84",
		"w186",
		"w167",
		"w170",
		"w169",
		"w163",
		"w161",
		"w164",
		"w166",
		"w182",
		"w178",
		"expansions",
		"cellToId",
		"quest_item_peach_bubbly",
		"w171",
		"w165",
		"active",
		"w175",
		"w183",
		"w192",
		"w185",
		"w162",
		"-24",
		"xpromo",
		"w189",
		"material_ribbon",
		"w187",
		"deco_sidewalk_bamboo",
		"w191",
		"sunset2_qf_gondola_1",
		"material_watersports_lifevest",
		"w188",
		"material_pneumatic_drill",
		"14",
		"w176",
		"w83",
		"res_frenchaptbuilding_3",
		"5",
		"w193",
		"mainstreetcollection",
		"material_permit",
		"storage_barn",
		"material_honeymoon_sign",
		"w190",
		"crafting",
		"tree_poinciana",
		"mun_night_civic_center_1",
		"ship_cruise",
		"bus_german_brewery",
		"q_expansion_shamrock",
		"qf_rainbow",
		"w194",
		"crewPos",
		"w122",
		"mun_policestation_6",
		"w160",
		"material_honeymoon_hull",
		"w44",
		"w48",
		"hood_night_1",
		"res_housegarage",
		"dock_house",
		"construction_4x4_10stage",
		"sendTS",
		"helpTS",
		"giftTS",
		"w43",
		"w195",
		"construction_4x4_5stage",
		"w196",
		"mun_policestation_5",
		"w82",
		"mun_notre_dame",
		"presents",
		"w159",
		"w81",
		"construction_10x10_12stage",
		"res_missionmini",
		"plot_coinsunflowers",
		"w197",
		"material_fishing_pole_yellow",
		"material_fishing_pole_blue",
		"mun_policestation_2",
		"sunset2_qf_rainbow",
		"material_fishing_pole_red",
		"w39",
		"plot_grapesglera",
		"w198",
		"w41",
		"italiancollection",
		"w40",
		"mun_policestation_3",
		"w42",
		"highsocietycollection",
		"mun_policestation_4",
		"leprechaun_bridge_construction",
		"w199",
		"w36",
		"plot_eggplants",
		"w35",
		"w241",
		"w38",
		"carrotcollection",
		"w243",
		"w200",
		"w37",
		"deco_winter11_candysidewalk",
		"material_honeymoon_sail",
		"w242",
		"w238",
		"32",
		"w201",
		"w249",
		"w245",
		"w246",
		"res_londonflat",
		"w239",
		"w247",
		"w244",
		"plot_peas",
		"deco_coastalseagrapetree",
		"wilderness_17",
		"w248",
		"w240",
		"w202",
		"w123",
		"material_wooden_beams",
		"res_fall_grandmahouse",
		"material_subway_turnstile",
		"w250",
		"qf_districts_1",
		"w254",
		"partnerbuild_snow_man",
		"w124",
		"holidaytown_unlock_explodable_holidaytown_gate4",
		"res_loft",
		"openTime",
		"material_dry_ice",
		"40",
		"material_honeymoon_chain",
		"material_ice_pick",
		"qf_gondola_1",
		"w256",
		"plot_wheat",
		"w252",
		"SlottedContainer",
		"w253",
		"FastestGrowing",
		"w158",
		"w255",
		"w260",
		"deco_picnictable",
		"w204",
		"w251",
		"flower_rose_red",
		"construction_8x8_12stage",
		"w237",
		"w233",
		"w236",
		"deco_community_garden",
		"w34",
		"customerSources",
		"w203",
		"material_honeymoon_light",
		"w235",
		"w257",
		"w33",
		"w226",
		"baby_whale_expansion_4",
		"w259",
		"storage_outskirtsfarm",
		"baby_whale_expansion_3",
		"heavydutycollection",
		"w234",
		"w32",
		"w232",
		"baby_whale_expansion_2",
		"MatchUpOccupancyValue",
		"streakData",
		"activationTime",
		"inactiveTime",
		"streakLength",
		"w258",
		"w223",
		"w157",
		"acts",
		"mg_holidays1",
		"w265",
		"w261",
		"w225",
		"w205",
		"w220",
		"w231",
		"w227",
		"w263",
		"sunset2_q_double_rainbow_1",
		"material_fishing_pole_green",
		"w224",
		"w264",
		"w222",
		"w269",
		"mun_subway",
		"w270",
		"material_fishing_pole_purple",
		"w229",
		"plot_no_wither",
		"1000",
		"w80",
		"TrainTracks",
		"w218",
		"w262",
		"w219",
		"holidaytown_unlock_explodable_holidaytown_gate1",
		"material_ice_chainsaw",
		"w230",
		"sunset2_qf_rollercoaster",
		"w206",
		"w271",
		"w266",
		"w221",
		"w272",
		"baby_whale_expansion",
		"w217",
		"res_forest_house_02",
		"Truck",
		"material_gold_plating",
		"w228",
		"w273",
		"w216",
		"130",
		"w209",
		"w268",
		"w267",
		"w275",
		"w211",
		"plot_rice",
		"res_duplex",
		"w215",
		"w214",
		"w208",
		"spendEnergy",
		"tree_hanging_flower",
		"downtownpolice",
		"w207",
		"res_beachhut",
		"w276",
		"w213",
		"sunset2_q_intl_2013_saga_act1_2",
		"sunset2_q_intl_2013_saga_act1_1",
		"w212",
		"deco_streetclock",
		"w156",
		"w282",
		"75",
		"w277",
		"w274",
		"material_majestic_disco_ball",
		"sunset2_q_intl_2013_saga_act1_3",
		"w210",
		"deco_scene_euro_cobblestone",
		"mg_halloween1",
		"mun_drawbridge",
		"sunset2_q_expansion_double_rainbow",
		"material_UFO_parts",
		"w278",
		"w125",
		"w155",
		"w284",
		"w283",
		"material_UFO_sample",
		"pumpkincollection",
		"w305",
		"sleepState",
		"consumables",
		"alien_family_expansion_4",
		"w319",
		"w320",
		"material_ice_spray_bottle",
		"material_UFO_reports",
		"maintenance",
		"w280",
		"material_weather_rainhat",
		"material_ice_gloves_yellow",
		"plot_mushroom",
		"w317",
		"w20",
		"Farming",
		"w324",
		"mun_downtownpolicestation_nopop",
		"w321",
		"unlocks",
		"w279",
		"w281",
		"w323",
		"material_weather_rainboots",
		"w325",
		"w16",
		"w287",
		"num",
		"w285",
		"mystery_business",
		"sunset2_q_ice_factory_1",
		"alien_family_expansion_3",
		"mun_animalrescue",
		"w302",
		"w310",
		"w24",
		"w329",
		"w303",
		"bus_bedandbreakfast",
		"w315",
		"w14",
		"material_majestic_glow_sticks",
		"w286",
		"w316",
		"w306",
		"w288",
		"cranberrycollection",
		"w322",
		"w318",
		"w328",
		"w12",
		"w331",
		"fitnesscollection",
		"w17",
		"w292",
		"sunset2_q_expansion_gondola",
		"w13",
		"w326",
		"res_winter11_newenglandholidayhouse02",
		"w18",
		"w291",
		"TimedBusiness",
		"alien_family_expansion_2",
		"w19",
		"CommodityCapacity",
		"w304",
		"asn",
		"dsTimestamp",
		"w312",
		"w330",
		"w301",
		"deco_candycanefence02",
		"w327",
		"w300",
		"w295",
		"38",
		"w15",
		"w296",
		"q_expansion_gondola",
		"w21",
		"storageUpgrades_woodLootDropTable",
		"w336",
		"w309",
		"w308",
		"w298",
		"mg_thanksgiving1",
		"w313",
		"w293",
		"filled",
		"w311",
		"quests",
		"w294",
		"w349",
		"w290",
		"w354",
		"w314",
		"deco_zoojunglerock",
		"w289",
		"w307",
		"w332",
		"w350",
		"w352",
		"res_spanishapt3",
		"w351",
		"sleepTS",
		"wakeTS",
		"sunset2_q_rollercoaster_expansion",
		"w338",
		"w335",
		"w362",
		"q_intl_2013_saga_act1_2",
		"w299",
		"res_simpsons",
		"w22",
		"w334",
		"w337",
		"w297",
		"material_UFO_sign",
		"w361",
		"w333",
		"material_weather_puddles",
		"w353",
		"w339",
		"w154",
		"storage_grainsilo",
		"w359",
		"w153",
		"w355",
		"fishing",
		"gondola_expansion",
		"w126",
		"w357",
		"partnerbuild_labrador_balloon",
		"w360",
		"w23",
		"w347",
		"sunset2_q_city2ndbirthday_1",
		"w358",
		"w344",
		"w363",
		"w348",
		"sunset2_qf_chinese_booth",
		"retailcollection",
		"district_0",
		"fishing_hole_uncommon",
		"alien_family_expansion",
		"w356",
		"watermeloncollection",
		"rainbowFeeds",
		"w345",
		"w340",
		"w343",
		"w341",
		"material_UFO_scrambler",
		"w346",
		"deco_mystery_rose_garden_rose",
		"w365",
		"banditmugshotcollection",
		"w366",
		"w378",
		"w377",
		"res_usa_monticello",
		"plot_cabbage",
		"w368",
		"w376",
		"universityLogo",
		"failureCount",
		"mun_embassy_spain",
		"w364",
		"w375",
		"w342",
		"w370",
		"w379",
		"8",
		"sunset2_q_expansion_ny_blowout",
		"sunset2_qf_chinese_arcade",
		"w380",
		"construction_hotel_grand",
		"w393",
		"w373",
		"w382",
		"w367",
		"w371",
		"w381",
		"envelope",
		"res_privateestate",
		"deco_italianpiazza_tilefence",
		"w398",
		"w369",
		"w385",
		"w387",
		"q_cityatnight_saga_act2_1",
		"designercollection",
		"dubai",
		"mun_fishingboat_1",
		"res_winterhousemidcentury",
		"w386",
		"w374",
		"w397",
		"w372",
		"sunset2_q_intl_2013_saga_act2_2",
		"sunset2_q_intl_2013_saga_act2_1",
		"w383",
		"mun_postoffice",
		"w389",
		"w396",
		"greenHouseClear",
		"w407",
		"w395",
		"w403",
		"w405",
		"w388",
		"w401",
		"w394",
		"sunset2_qt_treeoflife",
		"w435",
		"terminal_airportextension",
		"w400",
		"w392",
		"w384",
		"w390",
		"w402",
		"deco_fall_sidewalk01",
		"district_1",
		"w404",
		"w410",
		"material_weather_raindrops",
		"w408",
		"w419",
		"material_weather_rainslicker",
		"w406",
		"w399",
		"deco_whale_rescue_baby",
		"bus_winterclothing",
		"w46",
		"sunset2_q_intl_2013_saga_act2_3",
		"parentExpansions",
		"Animal_Rabbit2",
		"120",
		"q_expansion_mtfourhead",
		"w436",
		"q_football_1",
		"w416",
		"w415",
		"w432",
		"w433",
		"material_bathrobe",
		"w409",
		"w417",
		"partnerbuild_pumpkin_barn",
		"w444",
		"w418",
		"w412",
		"w437",
		"w448",
		"w466",
		"w441",
		"w411",
		"w450",
		"w442",
		"w460",
		"factory_premiumgoods",
		"w421",
		"cruisecollection",
		"w443",
		"w434",
		"w391",
		"w427",
		"w440",
		"w420",
		"bus_bikesharing_3",
		"bus_bikesharing_2",
		"w445",
		"w494",
		"w455",
		"w414",
		"q_cityatnight_saga_act3_1",
		"w439",
		"w426",
		"sunset2_q_opening_ceremony_1",
		"w79",
		"w151",
		"w446",
		"w429",
		"res_simpsonmegabrick",
		"w413",
		"w422",
		"w501",
		"w447",
		"w424",
		"w478",
		"w431",
		"w469",
		"material_welcomechocolates",
		"w149",
		"w454",
		"w438",
		"w477",
		"w425",
		"partnerbuild_t_rex_balloon",
		"w497",
		"w451",
		"w495",
		"w428",
		"res_trailer",
		"w511",
		"w504",
		"w472",
		"w470",
		"w464",
		"w152",
		"w493",
		"w449",
		"w423",
		"w498",
		"w496",
		"material_pillows",
		"w506",
		"w150",
		"w509",
		"w483",
		"w484",
		"w453",
		"w473",
		"w485",
		"w505",
		"sunset2_qf_halloween_graveyard_1",
		"w465",
		"dam_construction_8x12_2stage",
		"w479",
		"w476",
		"w459",
		"w502",
		"w463",
		"w512",
		"w503",
		"w468",
		"BiggestWedding",
		"w488",
		"w461",
		"w515",
		"w430",
		"sunset2_q_intl_2013_saga_act3_2",
		"sunset2_q_intl_2013_saga_act3_1",
		"w480",
		"w499",
		"w513",
		"w456",
		"res_italian_oldapts",
		"mun_downtownpolicestation_2",
		"w457",
		"w489",
		"saga",
		"w452",
		"w471",
		"MostLivable",
		"w474",
		"w462",
		"w514",
		"w508",
		"w517",
		"partnerbuild_gingerbread_home",
		"helperClicks",
		"downtowncollection",
		"mun_alps_station_1",
		"w45",
		"w492",
		"w458",
		"w500",
		"w467",
		"w475",
		"w487",
		"mun_animalrescue_3",
		"plot_ginger",
		"sunset2_q_expansion_pony",
		"w486",
		"endTime",
		"awardChoice",
		"choices",
		"res_st_patrick_housesiding",
		"w510",
		"w524",
		"candy",
		"mun_downtownpolicestation_4",
		"w127",
		"w482",
		"w507",
		"w481",
		"w533",
		"sunset2_q_opening_ceremony_expansion",
		"w490",
		"eggplantcollection",
		"sunset2_q_holiday2012_saga_act1_1",
		"sunset2_q_holiday2012_saga_act1_2",
		"w491",
		"sunset2_q_warehouse_upgrades",
		"deco_evergreen_tree_A",
		"w529",
		"w516",
		"w519",
		"warehouse_deluxe",
		"game_63",
		"w530",
		"deco_coconuttree02",
		"sunset2_q_radio_antenna",
		"mun_leprechaun_bridge",
		"bus_venetianrestaurant_2",
		"sunset2_q_intl_2013_saga_act3_3",
		"mun_downtownpolicestation_3",
		"w518",
		"w525",
		"material_sunglasses",
		"w532",
		"w531",
		"MostHoneymoon",
		"w520",
		"gondola_station",
		"endTimestamp",
		"w521",
		"w527",
		"w528",
		"w522",
		"material_luggagecart",
		"sunset2_q_expansion_space_shuttle",
		"upgradeGate_1",
		"upgradeGate_3",
		"w536",
		"w555",
		"w526",
		"MostHarvested",
		"mun_downtownpolicestation_5",
		"w535",
		"w534",
		"w672",
		"mun_wedding_photographer",
		"w556",
		"q_expansion_mtfuji",
		"w557",
		"w544",
		"w565",
		"district_5",
		"w651",
		"w561",
		"sunset2_q_expansion_natural_wonders",
		"w547",
		"w549",
		"w562",
		"w563",
		"w540",
		"w537",
		"w564",
		"bus_coffeeshop",
		"w548",
		"Bundle",
		"normal",
		"suspension_start_time",
		"suspension_end_time",
		"nameExpires",
		"bookmarkMarker",
		"TrainStation",
		"train_tracks_ne",
		"mun_downtownpolicestation_6",
		"w559",
		"w551",
		"w638",
		"sunset2_q_water_park_1",
		"w636",
		"w523",
		"w545",
		"w560",
		"w632",
		"w550",
		"w567",
		"CustomResidence",
		"w546",
		"w554",
		"w631",
		"started",
		"mun_stadium_footballCV",
		"sunset2_q_newyear_res_go_1",
		"w543",
		"sunset2_q_newyear_res_live_1",
		"cityatnight_expansion_1",
		"w652",
		"w671",
		"quest_item_rocket_fuel",
		"w604",
		"sunset2_q_expansion_shamrock",
		"sunset2_q_newyear_res_help_1",
		"w570",
		"w541",
		"w586",
		"140",
		"w606",
		"w648",
		"w601",
		"Animal_horse1",
		"w594",
		"debugOffset",
		"w587",
		"overrideTimes",
		"overrideTypes",
		"ftueCounter",
		"supportedWeather",
		"activeDay",
		"helpedNpcs",
		"maxNpcs",
		"w592",
		"w571",
		"deco_alps_village_mystery_animal",
		"sunset2_q_ski_lodge_1",
		"w539",
		"w553",
		"w676",
		"w569",
		"w595",
		"w581",
		"w607",
		"w566",
		"MostProductive",
		"w670",
		"bus_venetianrestaurant_3",
		"w635",
		"w650",
		"w552",
		"w668",
		"w542",
		"w589",
		"w673",
		"deco_flowerPatchPurple",
		"w698",
		"w678",
		"plot_tomatoes",
		"material_monorail_turnstile",
		"BasePier",
		"w578",
		"w646",
		"w608",
		"w568",
		"visaSpain",
		"w583",
		"w647",
		"w538",
		"w675",
		"material_monorail_dashboardbuttons",
		"w640",
		"plot_mussel",
		"w639",
		"w626",
		"disableWeatherEffect",
		"w667",
		"w642",
		"w645",
		"w674",
		"w585",
		"wateringholecollection",
		"w649",
		"w669",
		"train_platform",
		"w644",
		"deco_fence01",
		"thirdPartyId",
		"w614",
		"w610",
		"w588",
		"w591",
		"w558",
		"w654",
		"w590",
		"w605",
		"w641",
		"w575",
		"w572",
		"w653",
		"w603",
		"w624",
		"w655",
		"w577",
		"w602",
		"mun_dam",
		"w613",
		"sunset2_q_gala_1",
		"w584",
		"w634",
		"sunset2_q_ponyranch_1",
		"w573",
		"w582",
		"w576",
		"w616",
		"w148",
		"w686",
		"deco_bikerack",
		"w611",
		"w580",
		"w679",
		"w630",
		"w633",
		"w643",
		"w665",
		"qm_sledquest",
		"w694",
		"w612",
		"w637",
		"w745",
		"w695",
		"w579",
		"w574",
		"w693",
		"w629",
		"w597",
		"w600",
		"sunset2_q_dragoncastle",
		"w677",
		"w685",
		"subway_1",
		"w628",
		"mun_weatherforecasttower",
		"w688",
		"installRecorded",
		"w593",
		"w692",
		"w702",
		"neighborhood_part",
		"w700",
		"Unknown",
		"w599",
		"w721",
		"cityatnight_expansion_2",
		"w707",
		"w627",
		"w656",
		"w683",
		"w680",
		"w609",
		"w681",
		"w625",
		"w691",
		"QuestComponent",
		"sunset2_q_lovers_apartment_1",
		"w699",
		"w708",
		"w697",
		"w615",
		"w618",
		"w758",
		"w620",
		"w687",
		"w716",
		"w619",
		"w704",
		"w661",
		"w711",
		"nationalPark_macroObject_repair",
		"w696",
		"Animal_cow1",
		"res_duplex01",
		"zsurvey_called",
		"holidaytown_unlock_explodable_holidaytown_gate2",
		"w744",
		"w746",
		"w663",
		"w737",
		"w710",
		"w709",
		"w621",
		"w747",
		"w684",
		"w622",
		"w706",
		"w712",
		"w659",
		"w690",
		"bus_russia_balalaikashop_2",
		"w749",
		"w752",
		"material_wood",
		"42",
		"res_penthouse",
		"w726",
		"plot_halibut",
		"w743",
		"w147",
		"w701",
		"w755",
		"w705",
		"w666",
		"w598",
		"w722",
		"w596",
		"w738",
		"dragoncastle_repair2",
		"w689",
		"w742",
		"w730",
		"w725",
		"franchise_rebuild1",
		"items",
		"qf_passportoffice",
		"sunset2_q_holiday2012_saga_act2_2",
		"sunset2_q_holiday2012_saga_act2_1",
		"plot_mistletoe",
		"w740",
		"sunset2_q_embassy_suites_quest",
		"wheatcollection",
		"w803",
		"deco_kiddieride_horse",
		"deco_deciduous_tree_B",
		"w682",
		"w748",
		"w617",
		"w739",
		"w703",
		"w731",
		"w657",
		"material_golden_key",
		"w623",
		"w804",
		"w736",
		"w757",
		"w660",
		"w732",
		"w754",
		"w719",
		"w727",
		"w728",
		"peacollection",
		"w723",
		"hometown_launch",
		"17",
		"w734",
		"w658",
		"w718",
		"w733",
		"w720",
		"w750",
		"country_code",
		"w824",
		"sunset2_qf_trick_or_treat",
		"w714",
		"w768",
		"w662",
		"w751",
		"grantInternationalSagaAct2Q1",
		"w778",
		"w828",
		"w814",
		"bus_tikidrivein_3",
		"alfalfacollection",
		"playerLove",
		"w900",
		"res_spanish_castle",
		"w775",
		"w664",
		"completedCount",
		"w724",
		"w807",
		"w756",
		"w717",
		"w802",
		"w753",
		"w810",
		"w771",
		"w797",
		"w788",
		"w827",
		"w735",
		"w791",
		"w773",
		"w830",
		"w715",
		"cv_expansion_retune_post_dt_2",
		"w869",
		"w896",
		"w1",
		"w759",
		"w741",
		"w760",
		"w795",
		"numTotalTickets",
		"numFreeTickets",
		"previousInitTime",
		"numCityCashSpent",
		"grantLoginTicket",
		"cv_expansion_unlock_sale",
		"lastViralTime",
		"w789",
		"w866",
		"w790",
		"frontendInfo",
		"w829",
		"w831",
		"w851",
		"w767",
		"w729",
		"w872",
		"w713",
		"w794",
		"w823",
		"w785",
		"w765",
		"w769",
		"w777",
		"bus_tikidrivein_2",
		"w822",
		"show_energy_modifier",
		"w761",
		"populationVersionedRebalance",
		"w862",
		"santa_sleigh_1",
		"w800",
		"w826",
		"globalTableVersionedPurge",
		"w848",
		"w808",
		"w855",
		"cv_dynamic_tuning_item_gate",
		"w805",
		"w811",
		"w26",
		"w796",
		"w772",
		"plsRewardSeen",
		"w792",
		"qf_CNY2013_1",
		"w871",
		"w849",
		"flashxPromo",
		"w832",
		"w897",
		"w781",
		"w801",
		"w816",
		"w776",
		"w774",
		"sunset2_q_nessi_1",
		"w899",
		"w891",
		"grantedRewardsForVersion",
		"w873",
		"w888",
		"w784",
		"sunset2_qf_mj_1",
		"w786",
		"w782",
		"w847",
		"hadOldFlashVersion",
		"gotReward",
		"w798",
		"city_games_pins",
		"objLevel",
		"w766",
		"w793",
		"cv_merged_population_appraisal",
		"w806",
		"w885",
		"w762",
		"w825",
		"holidaytowncollection",
		"w27",
		"w903",
		"w764",
		"w10",
		"w821",
		"matchup",
		"deco_fountain01",
		"w787",
		"w11",
		"w783",
		"w868",
		"w815",
		"AchievementComponent",
		"w870",
		"w819",
		"w863",
		"w859",
		"w928",
		"w886",
		"w907",
		"w887",
		"xpromoMetaData",
		"sunset2_q_halloween_saga_act1_1",
		"w838",
		"w894",
		"sunset2_q_halloween_saga_act1_2",
		"w920",
		"w889",
		"w901",
		"w820",
		"w925",
		"w904",
		"material_folders",
		"w779",
		"w813",
		"w780",
		"w812",
		"w905",
		"w893",
		"energy_3",
		"w867",
		"w856",
		"w770",
		"w895",
		"w898",
		"cashData",
		"w854",
		"w927",
		"w817",
		"w874",
		"w837",
		"w852",
		"w835",
		"w892",
		"w860",
		"w834",
		"w911",
		"w910",
		"w922",
		"w890",
		"lastCash",
		"firstTransaction",
		"w845",
		"w799",
		"w818",
		"material_hot_air_balloon_gondola",
		"w839",
		"w884",
		"w906",
		"w29",
		"w876",
		"w841",
		"w865",
		"q_double_rainbow_1",
		"w840",
		"w809",
		"w850",
		"HolidayLeaderboard",
		"w883",
		"fixedMajestic",
		"w9",
		"w877",
		"w844",
		"w923",
		"w861",
		"w902",
		"w833",
		"w858",
		"w763",
		"w926",
		"story_general",
		"grantCommunityGarden",
		"sunset2_q_cityatnight_saga_act1_1",
		"sunset2_q_cityatnight_saga_act1_2",
		"sunset2_q_cityatnight_saga_act1_3",
		"w882",
		"w842",
		"w853",
		"w909",
		"w843",
		"w951",
		"w929",
		"w924",
		"parking_lot_gift",
		"w936",
		"qf_deco_mode2",
		"lunarnewyearcollection",
		"w881",
		"w879",
		"w864",
		"w931",
		"w921",
		"w983",
		"w875",
		"w878",
		"w915",
		"Animal_Rabbit1",
		"w857",
		"w912",
		"w836",
		"mun_flying_reindeer_stables",
		"mexicancollection",
		"w972",
		"w846",
		"w916",
		"q_buckingham_palace_1",
		"w917",
		"w880",
		"qf_train_upgrades",
		"w28",
		"w145",
		"qm_coliseum",
		"w973",
		"w908",
		"w971",
		"w962",
		"w30",
		"w937",
		"w980",
		"w25",
		"sunset2_qf_upgrade_residences",
		"w146",
		"bus_tiki_poolbar_3",
		"w934",
		"Animal_moose1",
		"q_international_saga_act3_1_1",
		"w981",
		"mun_chinese_nye_parade",
		"w938",
		"w950",
		"w914",
		"w918",
		"grantNightStorable",
		"bus_tiki_poolbar_2",
		"UserStoryComponent",
		"w970",
		"flower_rose_violet",
		"plot_pinkroses",
		"wilderness_02",
		"w932",
		"w930",
		"w969",
		"w913",
		"w961",
		"w977",
		"w976",
		"bus_hyb_restaurant_apartment",
		"w933",
		"mun_embassy_uk",
		"ParkingLot",
		"wilderness_31",
		"w968",
		"plot_cod",
		"w935",
		"w31",
		"w944",
		"w963",
		"w949",
		"sunset2_qf_mb_1",
		"w943",
		"logo_CVU",
		"w948",
		"w952",
		"w978",
		"w984",
		"w919",
		"w993",
		"Animal_pig1",
		"w945",
		"w964",
		"w988",
		"w965",
		"carnival",
		"w974",
		"w967",
		"w986",
		"w947",
		"japan",
		"sunset2_qf_nyegift_1",
		"sunset2_q_holiday2012_saga_act3_2",
		"sunset2_q_holiday2012_saga_act3_1",
		"w966",
		"foreveryoungcollection",
		"w939",
		"w979",
		"w960",
		"w992",
		"w955",
		"w989",
		"w942",
		"w1016",
		"w975",
		"w953",
		"w958",
		"w982",
		"w991",
		"w990",
		"w1011",
		"us",
		"w956",
		"expansion_vday_2013",
		"material_broadway_sheet_music",
		"w1035",
		"w1008",
		"sunset2_q_halloween_saga_act1_3",
		"garden_roses_2x2",
		"w946",
		"w957",
		"w959",
		"w1022",
		"material_monorail_exitsigns",
		"w1018",
		"w954",
		"w1006",
		"w1013",
		"w1036",
		"w1057",
		"w1019",
		"w1017",
		"mun_duckfactory",
		"w941",
		"w1010",
		"w1012",
		"w940",
		"flower_rose_pinkyellow",
		"w1026",
		"w1001",
		"w1014",
		"material_laptops",
		"w1015",
		"material_inkpad",
		"ship_frieghter",
		"material_monorail_seatcushions",
		"plot_soybean",
		"material_textbooks",
		"w987",
		"mun_soccerstadium",
		"sunset2_q_expansion_2013_game_main",
		"sunset2_q_nighttime_space_needle_1",
		"w1020",
		"w1007",
		"w1030",
		"premium_greenhouse_1",
		"w1056",
		"mun_visitorcenter",
		"w997",
		"material_notebooks",
		"sunset2_q_cityatnight_saga_act2_1",
		"w994",
		"w1009",
		"w1034",
		"w1023",
		"w1042",
		"w1025",
		"w1000",
		"w985",
		"w1040",
		"bus_hardwarestore",
		"w1002",
		"w1004",
		"w1029",
		"w1021",
		"w995",
		"w999",
		"w1039",
		"w1027",
		"Factory",
		"w1038",
		"LESale",
		"qm_stadiums_baseball",
		"w996",
		"plot_potatoes",
		"w1033",
		"w78",
		"w1062",
		"q_intl_2013_saga_act2_2",
		"LakefrontPopulation",
		"sunset2_q_lakefront_saga_act1_1",
		"material_broadway_script",
		"w1005",
		"bridge_standard",
		"w1028",
		"material_hot_air_balloon_burners",
		"w998",
		"w1055",
		"w1024",
		"harvestTime",
		"w1060",
		"w1031",
		"flower_rose_white",
		"compressedPosition",
		"userReward",
		"featureDataMigrationOneOffGenCounter",
		"cruiseShips",
		"MostAdventurous",
		"w1037",
		"w1059",
		"w1058",
		"qf_windfarm_1",
		"w1003",
		"w1063",
		"w1041",
		"w1051",
		"w1052",
		"w1053",
		"w1032",
		"mun_fishingboat_5",
		"w1085",
		"w1044",
		"sunset2_q_expansion_corporations",
		"w1049",
		"material_visor",
		"jetsettercollection",
		"qm_stadiums_soccer",
		"enabled",
		"bus_russia_balalaikashop_3",
		"sunset2_q_st_paddys_leprechaun_bridge_1",
		"w1054",
		"mun_wildcard_1",
		"supplied",
		"deco_flowerpot",
		"w1046",
		"w1064",
		"w1043",
		"w1047",
		"w1082",
		"material_monorail_couplings",
		"sunset2_q_outdoor_theater_1",
		"qf_weather",
		"w1061",
		"w1086",
		"sunset2_q_super_greenhouse",
		"w1066",
		"sunset2_q_expansion_wonder_craft",
		"sunset2_qf_winter_trees",
		"qf_aircraft_carrier_1",
		"w1045",
		"w1069",
		"w128",
		"w1071",
		"w1065",
		"bus_burger",
		"w1068",
		"w1078",
		"res_upscalehouse",
		"w1084",
		"w1067",
		"w1088",
		"w1087",
		"qf_lovecars",
		"deco_flowerCirclePink",
		"w1080",
		"res_beachfrontapartment_b",
		"w1074",
		"w1048",
		"w1050",
		"plot_huckleberry",
		"w1075",
		"mun_civilunionhall",
		"w1089",
		"material_hot_air_balloon_balloon_canvas",
		"w1081",
		"material_monorail_operatorscap",
		"mun_metro_recordingstudio",
		"uk",
		"w1070",
		"w1083",
		"w1079",
		"material_vip_pass",
		"w1077",
		"w1073",
		"q_halloweenneighborhood",
		"holidaytown",
		"material_welding_torch",
		"w1095",
		"w1091",
		"counter",
		"w1101",
		"plot_prem_chilis",
		"mun_chefville_1",
		"sunset2_qt_crooked_house",
		"w1090",
		"material_first_class_seat",
		"mun_winter11_icehotel",
		"w1110",
		"res_winter11_duplex02",
		"bookwormcollection",
		"w1094",
		"w1098",
		"city_at_night_E2",
		"city_at_night_F1",
		"deco_holidaytown_streetlight_lights",
		"q_ice_factory_1",
		"city_at_night_E1",
		"w1072",
		"storage_grain_elevator",
		"city_at_night_D3",
		"frenchcuisinecollection",
		"city_at_night_D2",
		"-24_-96",
		"city_at_night_A6",
		"q_spanish_castle_1",
		"city_at_night_B5",
		"rewardForSpend",
		"city_at_night_D1",
		"w1106",
		"material_carry_on_luggage",
		"deco_metro_modernart",
		"mun_area51_5",
		"city_at_night_C3",
		"w1092",
		"q_intl_2013_saga_act3_2",
		"city_at_night_C2",
		"city_at_night_A5",
		"q_storage_upgrades_1",
		"w1100",
		"w1097",
		"material_ibeam",
		"w1099",
		"city_at_night_B2",
		"city_at_night_B3",
		"quest_item_hotair_protank",
		"w1076",
		"w1096",
		"w1105",
		"city_at_night_C4",
		"w144",
		"mun_majestic_hotel",
		"mun_embassy_us",
		"sunset2_q_polarbear_rescue_1",
		"city_at_night_B4",
		"city_at_night_C1",
		"w1102",
		"w1134",
		"w141",
		"city_at_night_B1",
		"city_at_night_A2",
		"sunset2_qf_monopoly_1",
		"res_ritahouse",
		"city_at_night_A3",
		"w1132",
		"w1093",
		"w1111",
		"w1103",
		"w1128",
		"w1131",
		"w1104",
		"w1147",
		"w1112",
		"w143",
		"city_at_night_A4",
		"w1108",
		"city_at_night_A1",
		"daily_visits_start",
		"w1107",
		"w1127",
		"material_hot_air_balloon_fan",
		"w1120",
		"w1135",
		"w1116",
		"13",
		"material_heli_radio",
		"material_heli_cargo_hook",
		"scratchCard",
		"versionedData",
		"initialAmount",
		"targetAmount",
		"material_heli_flight_stick",
		"Headquarter",
		"w1136",
		"w1133",
		"w1140",
		"w1123",
		"w142",
		"qf_high_click",
		"sunset2_q_cityatnight_saga_act3_1",
		"w1142",
		"qf_embassy_japan",
		"w1130",
		"w1113",
		"mun_embassy_japan",
		"appraisal_value_increase",
		"w1129",
		"base",
		"leaderboards",
		"w1117",
		"mun_italianwinery",
		"w1109",
		"sunset2_q_halloween_saga_act2_1",
		"sunset2_q_expansion_pumpkin",
		"w1114",
		"q_radio_antenna",
		"welcomeTrainOrder",
		"sunset2_q_halloween_saga_act2_2",
		"macro_ice_factory",
		"plot_spinach",
		"w1145",
		"w1137",
		"comfortfoodcollection",
		"w7",
		"bus_exploratorium_3",
		"w1115",
		"w1155",
		"q_wonder_great_pyramid",
		"LESaleRecommendation",
		"Theme_Lake_Grindable",
		"-24_-108",
		"w1146",
		"monorail_station",
		"bus_exploratorium_2",
		"material_hot_air_balloon_parachutes",
		"w1138",
		"w1148",
		"w1121",
		"w135",
		"bus_metro_warehousestore_3",
		"w1143",
		"w1118",
		"bus_metro_warehousestore_2",
		"sunset2_qf_halloween_3_3",
		"w1119",
		"landfill_reskin_tile_expansion_fallback_2",
		"sunset2_q_lakefront_saga_act2_1",
		"w1144",
		"material_truss",
		"q_expansion_ny_blowout",
		"sunset2_qf_solar_greenhouse",
		"w1149",
		"res_lakefront_fisherman",
		"deco_circus_sidewalkstraw",
		"DockHouse",
		"w136",
		"bus_jewelrystore",
		"w1139",
		"mun_radio_antenna",
		"w1122",
		"w1124",
		"w1156",
		"sunset2_q_gymnastics_center_1",
		"w1153",
		"mun_coliseum",
		"deco_clovertopiary",
		"w1125",
		"plot_alfalfa",
		"w1152",
		"landfill_reskin_tile_expansion_fallback_3",
		"material_hot_air_balloon_ballasts",
		"deco_mystery_seedling_nomove",
		"w1154",
		"w1126",
		"mun_animalrescue_2",
		"rose",
		"q_greenhouse3_1",
		"shell",
		"w1141",
		"mun_baseballstadium",
		"sunset2_qm_cityville2_1",
		"w1150",
		"sunset2_q_cityatnight_saga_act2_3",
		"sunset2_q_cityatnight_saga_act2_2",
		"w1162",
		"gXPromo_lastUpdateTime",
		"w1161",
		"w137",
		"sunset2_q_expansion_tree_of_life",
		"w1151",
		"plot_artichoke",
		"material_counterweight",
		"sunset2_qf_halloween_2_3",
		"w1176",
		"landfill_reskin_tile_expansion_fallback_4",
		"w131",
		"q_italian_castle_1",
		"mun_cargarage",
		"deco_flagpole",
		"w1166",
		"material_graduationgowns",
		"summaries",
		"SeenQuestManager",
		"material_space_shuttle_plans",
		"w139",
		"w1157",
		"material_dtwn_cash_register",
		"sunset2_q_halloween_saga_act2_3",
		"material_graduationcaps",
		"w1158",
		"w1168",
		"material_lofts_intercom",
		"w1173",
		"plot_organic_squash",
		"w1171",
		"material_lofts_doorbuzzer",
		"w1177",
		"w1167",
		"helperCount",
		"w1186",
		"hood_solar_super_neighborhood_1",
		"w130",
		"deco_springflowerpatch02",
		"w1160",
		"w1159",
		"w1169",
		"w1163",
		"w1187",
		"w1212",
		"w1185",
		"w1164",
		"awardUserEnergy",
		"w129",
		"res_falleichler",
		"w1191",
		"Wonder",
		"w1195",
		"w1188",
		"w1189",
		"w1207",
		"q_greenhouse2_1",
		"q_lovehotel_1",
		"w140",
		"w1175",
		"q_greenhouse",
		"landfill_reskin_tile_expansion_fallback",
		"sunset2_qf_upgrade_residences_family",
		"mun_area51_4",
		"mun_detective_academy",
		"macro_keys_aloha",
		"w1181",
		"w1199",
		"grantChefvilleDeco",
		"w138",
		"value",
		"w1197",
		"qux_visits",
		"w1174",
		"res_chinese_embassy",
		"q_intl_2013_saga_act1_3",
		"mun_clerkoffice",
		"qf_customres1",
		"w1190",
		"grantflyingReindeerStable",
		"w132",
		"w1184",
		"w1205",
		"mun_farmville2_1",
		"w1194",
		"univ_library",
		"w1193",
		"carsenabled",
		"car_tinycruiser",
		"car_cvbee",
		"w1165",
		"w1170",
		"house2",
		"w1200",
		"w1172",
		"w1178",
		"deco_roadsign1",
		"city_games_pins_1",
		"weather",
		"material_cement",
		"q_lovers_apartment_1",
		"w1206",
		"q_cherry_blossom_park_1",
		"challengeGroups",
		"danish",
		"house1",
		"sunset2_qf_halloween_1_3",
		"w1179",
		"mun_taxi_dispatch",
		"w1220",
		"w1196",
		"monorail_track",
		"w1203",
		"w1210",
		"material_dtwn_counter_top",
		"mun_area51_3",
		"w1180",
		"w1211",
		"w1192",
		"w1182",
		"w1198",
		"w1209",
		"w1215",
		"w1208",
		"sunset2_q_what_city_1",
		"w1201",
		"sunset2_qf_halloween_2_2",
		"cacheTS",
		"w1202",
		"ClerkOfficeUpgrade2",
		"w134",
		"w1183",
		"res_hotel",
		"31",
		"mun_NYE2012_museumparty",
		"w133",
		"sunset2_qt_waterfall_timed",
		"w1214",
		"tomatocollection",
		"mun_area51_2",
		"q_chinese_newyear_2012_1",
		"w1213",
		"sunset2_q_cityatnight_saga_act3_3",
		"sunset2_q_cityatnight_saga_act3_2",
		"res_mansion02",
		"mun_cvg_ceremony_hall",
		"w1223",
		"w1216",
		"w1226",
		"w1238",
		"w1237",
		"w1204",
		"w1236",
		"w77",
		"res_kinkakuji",
		"q_expansion_brazil_waterfall",
		"w1219",
		"sam",
		"material_dtwn_glass_cubicle",
		"q_winter_castle",
		"w1234",
		"grantStadium",
		"sunset2_qm_lovestory_act3_1",
		"w1225",
		"w1246",
		"w1218",
		"deco_birthdaycake",
		"w1222",
		"material_dtwn_window_maker",
		"w1217",
		"material_dtwn_file_organizer",
		"mun_staffingagency",
		"mun_color_arch",
		"sunset2_qf_international_hood",
		"w1227",
		"w1221",
		"w1242",
		"mun_bubblesafaristatue",
		"w1228",
		"lootTables",
		"w1241",
		"brazil",
		"hotel_grand_1",
		"easter",
		"cards",
		"material_supersolarmuni_bluerobot",
		"sunset2_qf_halloween_3_2",
		"MonorailLeaderboard",
		"material_dtwn_carpet_rack",
		"w1231",
		"w1235",
		"w1256",
		"w1253",
		"grantMun_cvg_ceremony_hall",
		"alps_tourist_center",
		"w1239",
		"material_supersolarmuni_greenrobot",
		"w1245",
		"w1229",
		"material_coreloop_cable",
		"bus_duckhq_2",
		"mun_fish_hatchery",
		"wilderness_30",
		"w1232",
		"grantEnvelopeTicketShop",
		"res_strawhouse_house",
		"grantChineseArcadeParade",
		"material_coreloop_mount",
		"w1250",
		"w1224",
		"w1243",
		"material_solar_pipe",
		"lastAnnounceSeen",
		"losingStreak",
		"latestScratches",
		"isNewScratcher",
		"w1247",
		"euro_stadium",
		"gondola_track_grant_count",
		"bus_toystore",
		"sunset2_q_cherry_blossom_park_1",
		"mun_spain_museodelprado",
		"material_solar_heater",
		"material_coreloop_stand",
		"material_solar_boiler",
		"banditmugshot2collection",
		"q_expansion_double_rainbow",
		"mun_townhall",
		"w1240",
		"q_cherry_park_palace",
		"w1251",
		"grantOneYearCake",
		"w1233",
		"w1255",
		"wilderness_11",
		"grantBubbleStatue",
		"freeCards",
		"freeCardResetTime",
		"macro_fishing_1",
		"w1252",
		"permit",
		"qf_storage_facility_1",
		"grantShuttle",
		"macro_fishing_3",
		"grantDetectiveTrainingCenter",
		"grantStaffingAgency",
		"q_intl_super_muni_1",
		"w1230",
		"w1254",
		"bus_sportsstore",
		"w1248",
		"grantSolarSuperNeighborhood",
		"w1244",
		"grantArea51",
		"terra_tile_frozen_land_3x3",
		"macro_fishing_4",
		"grantBabyWhale",
		"grantPartnerTrex",
		"Ship_Contracts_Dubai",
		"q_intnl_saga_act3_questline1_DropTable1",
		"w1249",
		"macro_equality",
		"mun_holidaycarousel",
		"grantPumpkinBarn",
		"res_countryhome",
		"material_space_boots",
		"w1273",
		"sunset2_q_lakefront_saga_act3_1",
		"macro_fishing_5",
		"double_rainbow_expand",
		"w1269",
		"sunset1_qf_rainbow",
		"sunset2_qf_easter",
		"w1268",
		"w1267",
		"macro_district",
		"numHarvestCars",
		"q_intl_2013_saga_act1_1",
		"w1264",
		"w1278",
		"launch_pad_1",
		"w1281",
		"grantPartnerBuildSnowman",
		"w1271",
		"0_0",
		"grantSantaSleigh",
		"w1259",
		"w1260",
		"qm_themepark_1",
		"material_dtwn_glass_tubing",
		"gold",
		"w1284",
		"w1257",
		"artichokecollection",
		"sunset2_qf_govcenter",
		"ezchair",
		"bus_expo_center_level_1",
		"foldingtable",
		"w1274",
		"w1280",
		"w1288",
		"material_natlpark_log",
		"announcements_announce_LocaleSelector",
		"gardengnome",
		"radio",
		"qf_candybooth",
		"w1283",
		"qf_farmers_market",
		"sunset2_q_cherry_park_palace",
		"w1258",
		"res_CNY2013_hutong",
		"w1272",
		"birdhouse",
		"w1289",
		"mun_shrinkray",
		"sunset2_qm_lovestory_act2_1",
		"w1286",
		"credits",
		"w1295",
		"w1276",
		"casino_social_business",
		"w1261",
		"w1287",
		"gondola_track",
		"w1277",
		"material_monorail_maps",
		"w1266",
		"plot_prem_grapes",
		"order_lot",
		"w1275",
		"w1270",
		"w1279",
		"material_dtwn_glass_torch",
		"w1265",
		"lastSlotsAnnounceSeen",
		"res_winter11_cabin",
		"road-init",
		"w1282",
		"w1291",
		"sunset2_qf_super_res_hall_1",
		"w1263",
		"w1262",
		"w1285",
		"qf_carcraft",
		"sunset2_qf_halloween_2",
		"tiki_social_business",
		"birthday_2011_expired",
		"sunset2_q_halloween_skyscraper_1",
		"material_monorail_badge",
		"sunset2_q_pyg_museum",
		"enclosure_jungle",
		"w1296",
		"w1299",
		"material_space_map",
		"w1294",
		"w1303",
		"macro_dream_height",
		"sunset2_qf_CVG_pins_1",
		"material_keycard",
		"qf_halloween_graveyard_1",
		"w1293",
		"q_kaboom_1",
		"w1290",
		"w1298",
		"res_snow_man_house_2",
		"mun_can_parliment",
		"material_airplane_pillow",
		"w1302",
		"mun_fishingboat_3",
		"w1307",
		"material_chandelier",
		"Business_Restaurant_Region_Japan",
		"q_intnl_saga_act2_questline1_DropTable2",
		"w1297",
		"wilderness_14",
		"mun_fishingboat_2",
		"closeTS",
		"qf_airports_v3_1",
		"qm_swissmuseum",
		"material_natlpark_shingle",
		"w1315",
		"w1309",
		"w1306",
		"w1326",
		"w1301",
		"w1304",
		"sunset2_q_expansion_citySign",
		"w1327",
		"bus_cheesesteak",
		"q_intl_2013_saga_act2_3",
		"cookie",
		"w1292",
		"bus_duckhq_3",
		"w1300",
		"mun_fishingboat_4",
		"milk",
		"trackPermissions",
		"sunset2_q_expansion_hedge_fund",
		"grantHolidayCarousel2011",
		"sunset2_q_spanish_castle_1",
		"construction_expo_center",
		"cupcake",
		"factory_textilegoods",
		"mun_embassy_japanPlace",
		"grantUSembassy",
		"grantUKembassy",
		"w1325",
		"grantMacroDistrict",
		"pie",
		"sunset2_q_xmas_saga_act3_1",
		"sunset2_qm_lovestory_act1_1",
		"w1305",
		"mun_conservatory",
		"w1332",
		"qf_airports_v2_1",
		"macro_broadway",
		"w1314",
		"w1336",
		"grantAmphitheater",
		"q_cruiseship_upgrade_1",
		"w1321",
		"material_heli_ear_protectors",
		"viralAck",
		"france",
		"q_expansion_2013_game_main",
		"res_usa_monticelloPlace",
		"material_customs_desk",
		"material_ticket_scanner",
		"w1319",
		"w1335",
		"qf_toysfortots_1",
		"MonorailTrack",
		"material_heli_helmet",
		"Animal_Goat1",
		"doobers",
		"w1312",
		"w1333",
		"res_italian_castle",
		"w1329",
		"w1330",
		"grantColorGeyser",
		"w1328",
		"qf_malls",
		"q_intl_2013_saga_act2_1",
		"w1337",
		"cache_ingame",
		"traincollection1",
		"w1324",
		"bus_world_financial_center",
		"mun_customcarshop",
		"w1308",
		"grantWildcard",
		"q_intl_2013_saga_act3_3",
		"w1323",
		"q_shipping_upgrades_1",
		"material_paparazzi_photo",
		"w1339",
		"bus_hyb_diamond_ring_apartment",
		"w1338",
		"qf_christmasday",
		"cherry_park_expand",
		"plot_salmon",
		"biz_ups_intro",
		"bus_tshirtshop",
		"holiday_tree_2012_1",
		"res_italian_oldaptsB",
		"deco_arrowsplitting",
		"mun_downtownnewspaper",
		"silverscreencollection",
		"material_lofts_soundproofing",
		"material_rivet",
		"w1331",
		"qf_universities",
		"Attraction",
		"sunset2_qf_stjude_quest",
		"qf_business_district_1",
		"bus_appliances",
		"material_gondola_mechanical_lever",
		"material_gondola_ticket",
		"lastVipGift",
		"wilderness_07",
		"q_polarbear_rescue_1",
		"sunset2_q_citymap_saga_act3_3_1",
		"q_intl_2013_saga_act3_1",
		"w1322",
		"w1334",
		"w1311",
		"rollCall",
		"stateChangeTime",
		"numPurchasedCheckins",
		"material_itinerary",
		"material_lofts_mailbox_bank",
		"mun_spy_building",
		"nyeCountDown2012",
		"nyeCountDown2012_day1",
		"nyeCountDown2012_day2",
		"nyeCountDown2012_day3",
		"nyeCountDown2012_day4",
		"nyeCountDown2012_day5",
		"nyeCountDown2012_day6",
		"material_supersolarmuni_silverrobot",
		"res_summerapartment_2",
		"nyeCountDown2012_day7",
		"w1316",
		"plot_kale",
		"w1320",
		"w1317",
		"qux_townhall",
		"zooUnlock_enclosure_jungle",
		"w1341",
		"sunset2_q_newyear_eve_1",
		"w1318",
		"grantNYE2012_day4",
		"grantNYE2012_day5",
		"w1343",
		"macro_glass_factory",
		"biz_ups_visit_toaster",
		"w6",
		"w1340",
		"bus_lovehotel",
		"sunset2_q_xmas_saga_act3_2",
		"deco_mystery_seedling",
		"22",
		"w1310",
		"bbq",
		"sunset2_q_neighborhood_o_hoods",
		"sunset1_qf_mb_1",
		"Animal_chicken3",
		"qf_coreloop_winter2011",
		"sunset2_q_expansion_boathouse",
		"80",
		"material_space_compass",
		"snowed",
		"grantNYE2012_day6",
		"pbus_winebar",
		"diningtable",
		"quilt",
		"w1360",
		"w1357",
		"leathercouch",
		"cairo_plane",
		"seenDecoModeInfographic",
		"w1313",
		"fireplace",
		"i-1",
		"xpromo_anim_promoBookmark",
		"grantNYE2012_day3",
		"bus_donutshop",
		"w1342",
		"district_4",
		"sunset2_q_amphitheater_1",
		"q_expansion_2013_game_3",
		"w1359",
		"w1386",
		"90",
		"sunset2_q_lakefront_neighborhood_1",
		"w1385",
		"material_nyeCountDown2012_key_day1",
		"w1358",
		"q_expansion_2013_game_1",
		"material_nyeCountDown2012_key_day6",
		"w1381",
		"craftenabled",
		"mun_nanotech_school",
		"w1371",
		"theater_amphitheater_1",
		"w1344",
		"q_expansion_2013_game_2",
		"35",
		"parking_lot",
		"material_nyeCountDown2012_key_day2",
		"w1387",
		"mun_hotair_gp_1",
		"deco_roadsign2",
		"material_nyeCountDown2012_key_day3",
		"w1347",
		"w1362",
		"sunset2_q_expansion_waterfall_pow",
		"citycenter_2012_expired",
		"sunset2_q_xmas_saga_act3_3",
		"w1361",
		"construction_4x4_6stage",
		"w1411",
		"material_nyeCountDown2012_key_day4",
		"sunset2_q_buckingham_palace_1",
		"material_nyeCountDown2012_key_day5",
		"w1407",
		"w1376",
		"w1356",
		"bus_tavern",
		"sunset2_q_kaboom_1",
		"150",
		"w1354",
		"w1345",
		"w1352",
		"w1416",
		"w1368",
		"material_zoningpermit_paperthread",
		"qf_honeymoon_1",
		"w1355",
		"w1348",
		"material_zoningpermit_permitpaper",
		"qf_airports_1",
		"res_int13_super_res_1",
		"sunset2_qf_tajmahal",
		"grantIceHotel2011",
		"grantGreenHouse2011",
		"w1414",
		"grantGreenHouse2_2012",
		"grantGreenHouse3_2012",
		"w1365",
		"material_staff_manual",
		"w1400",
		"w1350",
		"w1417",
		"grantNYE2012_day2",
		"grantSolarMall",
		"material_staff_white_board",
		"material_staff_shirt",
		"grantGlassFactory",
		"q_halloween_saga_act2_1",
		"w1372",
		"grantSubwayEntrance",
		"grantSubwayStop",
		"bus_supermarket",
		"construction_10x10_majestic_hotel",
		"w1382",
		"w1403",
		"material_drinkumbrella",
		"sunset2_q_expansion_bullrun",
		"w1399",
		"sunset1_qf_rollercoaster",
		"q_halloween_saga_act2_2",
		"w1364",
		"trucking_truck_SW",
		"grantBroadway",
		"grantGala",
		"sunset2_qf_elder_gala",
		"w1430",
		"w1379",
		"material_opencoconut",
		"bus_deptstore",
		"material_zoningpermit_permitseal",
		"grantColorArch",
		"grantPermits_center",
		"w1349",
		"grantMysteryFamousBusiness",
		"w1374",
		"sunset2_qf_grandcanyon",
		"grantPremBus_Mall",
		"w1346",
		"w1423",
		"grantWaterCropsFishHatchery",
		"w1412",
		"q_expansion_nessi_jr",
		"loveseat",
		"w1384",
		"w1389",
		"w1375",
		"w1366",
		"w1397",
		"material_green_bird",
		"w1410",
		"heliport_pad_harvest",
		"q_intl_super_res_1",
		"w1373",
		"vacuum",
		"w1353",
		"material_rosegarden_castle_spire",
		"computer",
		"w1383",
		"chocolatecoveredstrawberries",
		"w1418",
		"sunset2_q_halloween_saga_act3_1",
		"w1393",
		"w1408",
		"GondolaTrack",
		"w1398",
		"sunset2_q_citymap_saga_act2_2_1",
		"w1390",
		"w1421",
		"redwagon",
		"pre_upgrade",
		"sunset2_q_citymap_saga_act3_2_1",
		"grantNYE2012_day1",
		"deco_whale_rescue_mama_1",
		"material_space_jets",
		"w1370",
		"hood_international_level1",
		"w1420",
		"grantFishingHole5",
		"grantFishingHole4",
		"grantFishingHole3",
		"grantFishingHole",
		"w1363",
		"w1380",
		"tennisrackets",
		"q_wonder_eiffel_tower",
		"w1439",
		"strawberryjam",
		"w1369",
		"w1404",
		"w1395",
		"qf_fireworks_main_1",
		"w1401",
		"w1402",
		"cashNotice",
		"lastPlayed",
		"w1378",
		"w1367",
		"air_cargo_plane",
		"w1388",
		"qf_deco_scenes",
		"res_french_chateau",
		"w1351",
		"monorail_track_grant_count",
		"w1413",
		"strawberryshortcake",
		"virgin",
		"mun_envelopesticketbooth",
		"material_whistle",
		"sunset2_q_citymap_saga_act3_1_1",
		"w1396",
		"w1429",
		"w1394",
		"strawberrylemonade",
		"w1415",
		"strawberrysmoothie",
		"w1422",
		"w1391",
		"q_halloween_saga_act1_2",
		"material_passenger_seat",
		"grant2012chineseEmbassy",
		"w1409",
		"w1406",
		"bus_cassette_shop",
		"surfandturfcollection",
		"w1427",
		"sunset2_q_citymap_saga_act1_3_1",
		"material_dtwn_merch_shelves",
		"w1433",
		"material_dtwn_receipt_printer",
		"sunset2_q_expansion_hotairballoon",
		"w1377",
		"w1405",
		"w1428",
		"q_aloha_island",
		"material_attendant_outfit",
		"mun_permits_center",
		"trainupgradescollection",
		"grantSkiResort2011",
		"crop_shamrock",
		"grantPartnerBuildCandyland",
		"grantHarmonyHeights2011",
		"mall_prembus_1",
		"w1425",
		"material_dtwn_display_case",
		"q_elder3_cookie_baking_1",
		"plot_prem_coffee",
		"construction_grand_concert_hall",
		"w1419",
		"w1432",
		"q_arc_1",
		"sunset2_q_halloween_saga_act3_2",
		"sunset2_q_coreloop_08092012",
		"qt_perfect_tree",
		"w1426",
		"plot_prem_taro",
		"material_siding",
		"material_solar_panel2",
		"q_frozen_landfill_1",
		"bus_featured_scenery_photo_studio_1",
		"w1392",
		"w1440",
		"portal_demo",
		"w1435",
		"qf_enrique_1",
		"sunset2_qf_halloween_3",
		"w1424",
		"w1451",
		"w1438",
		"material_space_suit",
		"bus_leisure_condo_ice_cream_condo_1",
		"w1431",
		"w1463",
		"sunset2_q_xmas_saga_act2_1",
		"poll",
		"material_solar_exhaust",
		"q_ski_lodge_1",
		"w1434",
		"res_usa_cityhouse",
		"macro_dragoncastle",
		"q_earth_day_1",
		"plot_gaoliang",
		"perfectfitcollection",
		"bus_science_center",
		"w1437",
		"q_island_ferry",
		"populationVersionedRepair",
		"spinachcollection",
		"w1464",
		"sunset2_q_xmas_saga_act2_3",
		"grantCustomRes",
		"w1462",
		"w1436",
		"sunset2_q_xmas_saga_act2_2",
		"sunset2_q_greenhouse_water_1",
		"crew_build",
		"sunset2_q_citymap_saga_act2_1_1",
		"sunset2_q_citymap_saga_act1_1_1",
		"material_isle_cart",
		"deco_halloween_sidewalk_1",
		"w1442",
		"mun_tree_of_cherry_blossom",
		"w1474",
		"sailboat_pier",
		"sunset2_q_citymap_saga_act2_3_1",
		"w1465",
		"w1458",
		"grantPassportOffice",
		"material_work_uniforms",
		"bus_web_2",
		"rest_res_metro_building05",
		"q_warehouse_upgrades",
		"w1443",
		"q_dragoncastle",
		"material_nails",
		"comicbooks",
		"grantLoveHotel",
		"actionfigure",
		"grantNewYearMuseum",
		"dead_buyer_sale_two_visits",
		"w1441",
		"paris",
		"deco_satellite_dish",
		"material_deckofcards",
		"new_buyer_sale_two_visits",
		"mun_winter_space_saver_cool_estates_1",
		"sunset2_qf_elder_wondercraft_1",
		"sunset2_q_intl_super_muni_1",
		"getEm",
		"bus_grand_concert_hall_level_1",
		"q_halloween_saga_act1_1",
		"material_honeymoon_visa",
		"w1471",
		"w1453",
		"w1450",
		"res_japan_houseD",
		"sunset2_q_earth_day_1",
		"bus_featured_wildlife_rehabilitation_center_1",
		"qf_game_flying_main",
		"w1452",
		"res_buckingham_palace",
		"sunset2_q_citymap_saga_act1_2_1",
		"grantInternationalSagaAct1Q1",
		"material_supersolarmuni_goldrobot",
		"bus_VIP_vacations_1",
		"res_wintermansion",
		"w1466",
		"w1454",
		"mun_postoffice_3",
		"material_rosegarden_castle_gates",
		"w1456",
		"material_supersolarmuni_redrobot",
		"w1455",
		"material_dice",
		"w1457",
		"w5",
		"flower_rose_lavender",
		"w1461",
		"material_dtwn_office_chair",
		"w1459",
		"grantCityJail2011",
		"grantSnowManHouse",
		"alps_muni",
		"w1460",
		"mun_downtownsquare",
		"w1479",
		"material_dtwn_glass_desk",
		"dead_buyer_discount",
		"w1467",
		"w1484",
		"q_holiday2012_saga_act1_2",
		"w1468",
		"res_japan_houseB",
		"material_pokerchips",
		"world_alps",
		"res_france_cottage_4",
		"q_holiday2012_saga_act1_1",
		"sunset2_q_skyway_gardens_1",
		"material_dtwn_glass_tile",
		"w4",
		"material_coreloop_meter",
		"w1475",
		"grantSolarPowerPlant",
		"w1449",
		"material_dtwn_glass_roofing",
		"res_skyscraper2",
		"material_dtwn_filing_cabinet",
		"material_dtwn_insulation",
		"grantIceFactory",
		"sunset2_q_jumbo_cruise_1",
		"qux_upgrade_business",
		"res_partypalace",
		"w1532",
		"SimpleOpenResource",
		"plot_sprouts",
		"qf_bullscountdown_1",
		"w1498",
		"w1445",
		"w1490",
		"bus_chocolate_fountain_1",
		"bus_cosmeticstore",
		"w1444",
		"BusStop",
		"sunset2_qf_citygames_escapade_1",
		"material_dtwn_glass_mold",
		"material_coreloop_dish",
		"useNewExpansionPopData",
		"res_portal1",
		"sunset2_qf_toysfortots_1",
		"dock_piece",
		"w1480",
		"w1508",
		"w1494",
		"w1503",
		"wilderness_16",
		"mun_downtowncourthouse",
		"w1476",
		"w1448",
		"w1473",
		"mun_operahouse",
		"mun_downtownbaseballstadium",
		"material_two_by_fours",
		"w1499",
		"material_shopping_bag",
		"w1495",
		"w1536",
		"qm_attractionTimed",
		"q_city_of_the_year_1",
		"w1506",
		"material_solar_panel",
		"mun_winter_space_saver_cool_center_1",
		"w1507",
		"w1504",
		"material_deck_chair",
		"w1472",
		"qt_superdeco",
		"sunset2_q_expansion_broadway",
		"q_holiday2012_saga_act1_3",
		"res_russian",
		"res_condo",
		"bus_featured_ski_equipment_store_1",
		"w1487",
		"w1470",
		"q_halloween_saga_act1_3",
		"mun_outdoor_theater",
		"w1530",
		"w1531",
		"w1485",
		"w1526",
		"res_portal2",
		"q_citymap_saga_act3_1_1",
		"q_keys4_quest",
		"w1481",
		"w1447",
		"w1488",
		"qf_upgrade_residences",
		"w1482",
		"macro_airport",
		"deco_doghouse01",
		"w1502",
		"w1446",
		"mun_newspaperhq",
		"w1478",
		"windfarm",
		"w1525",
		"w1469",
		"w3",
		"qm_spyagency",
		"theme_park",
		"w1527",
		"w1501",
		"anniversary_b_day_crate_blue",
		"w2",
		"w1486",
		"bus_magicshop",
		"w1500",
		"sunset2_q_summer_adventure_expansion",
		"anniversary_b_day_crate_orange",
		"w1541",
		"w1533",
		"anniversary_b_day_crate_purple",
		"anniversary_b_day_crate_red",
		"anniversary_b_day_crate_green",
		"w1489",
		"photo_gallery",
		"w76",
		"w1522",
		"material_expo_center_name_badges",
		"material_expo_center_expo_chairs",
		"material_expo_center_display_stands",
		"qm_bridge_v2_1",
		"q_holiday2012_saga_act2_1",
		"sunset1_q_rollercoaster_expansion",
		"base_population_value_percent_multiplier",
		"max_population_value_percent_multiplier",
		"upgradableresidences_level",
		"rent_value_percent_multiplier",
		"q_keys3_quest",
		"w1493",
		"qux_sportsstore",
		"res_houseshingle",
		"w1491",
		"w1523",
		"macro_keys_land_hillside",
		"handheldgame",
		"w1540",
		"Community",
		"w1496",
		"w1497",
		"new_buyer_discount",
		"w1542",
		"qm_skyscrapers1",
		"material_gondola_lift_door",
		"w1535",
		"w1492",
		"q_holiday2012_saga_act2_3",
		"sunset2_q_webcompany",
		"w1517",
		"gameconsole",
		"boardgame",
		"w1483",
		"q_holiday2012_saga_act2_2",
		"mall_flagship_1",
		"mun_hillside_museum",
		"bus_electronicsstore",
		"w1543",
		"petrescuecollection",
		"material_lofts_foodie",
		"w1528",
		"w1552",
		"q_monsterdentist",
		"sunset2_q_expansion_space_shuttle_v2",
		"w1545",
		"w1518",
		"w1565",
		"q_holiday2012_saga_act3_2",
		"w75",
		"w1538",
		"w1521",
		"w1520",
		"w1561",
		"w1529",
		"w1537",
		"w1511",
		"w1477",
		"w1563",
		"macro_farmersMarket",
		"w1513",
		"w1514",
		"1331967599",
		"w1516",
		"SocialBusiness",
		"mun_rentcollectordepot",
		"w1566",
		"q_opening_ceremony_1",
		"w1550",
		"mun_pony_circus",
		"deco_zooplainstree01",
		"w1555",
		"tower_of_terror_regular",
		"w1569",
		"macro_aircraft_carrier",
		"w1509",
		"w1577",
		"w1539",
		"w1559",
		"w1544",
		"w1534",
		"w1551",
		"w1519",
		"enclosure_arctic",
		"Business_City_At_Night",
		"q_cityatnight_saga_2_1DropTable",
		"material_security_dog",
		"w1510",
		"w1554",
		"sunset2_q_frozen_landfill_1",
		"w1568",
		"musicDisabled",
		"w1567",
		"w1562",
		"sunset1_qf_govcenter",
		"sunset2_q_expansion_fireworks_barge",
		"w1505",
		"w1549",
		"greenhouse_grassroots",
		"mun_appleorchard",
		"w1515",
		"w1560",
		"q_holiday2012_saga_act3_1",
		"material_dtwn_cooling_rack",
		"sunset2_q_morecars_1",
		"w1547",
		"sunset1_qf_international_hood",
		"grantShanghaiCenter2012",
		"grantWinterCastle",
		"expansion_antarctic",
		"grantInternationalSagaAct3Q1",
		"grantKinkakuji",
		"w1571",
		"material_upgraderesidence_seal",
		"biz_ups_halfway",
		"grantF1Track",
		"w1546",
		"q_new_years_core_loop_1",
		"grantClocktower",
		"material_dtwn_air_tank",
		"w1587",
		"sunset2_q_summer_adventure_1",
		"w1576",
		"construction_hotel_sailboat",
		"w1595",
		"zooUnlock_enclosure_arctic",
		"w1548",
		"w1605",
		"w1581",
		"w1586",
		"material_fishfountain",
		"w1616",
		"w1524",
		"w1580",
		"mun_color_geyser_1",
		"w1512",
		"w1564",
		"sunset2_q_expansion_national_park",
		"res_lakefront_lodge",
		"w1556",
		"w1603",
		"res_brazilian_house_B",
		"sunset2_q_xmas_saga_act1_2",
		"w1570",
		"deco_roadsign4",
		"material_farmers_fruitbasket",
		"sunset2_qf_game_flying_main",
		"w1602",
		"w1617",
		"res_wintercastle",
		"sunset2_qf_super_pier_1",
		"w1578",
		"w1553",
		"sunset2_q_famous_residences_1",
		"material_farmers_foodscoop",
		"sunset2_q_expansion_game_flying_3",
		"grantbus_VIP_vacations_1",
		"material_gondola_lift_gear",
		"mall_solar",
		"w1584",
		"w1594",
		"mun_greenhouse_high_2",
		"coffee",
		"santas_workshop",
		"w1579",
		"w1557",
		"w1599",
		"w1601",
		"w1593",
		"district_3",
		"w1574",
		"material_farmers_picklejars",
		"w1573",
		"gala_level_1",
		"latte",
		"doughnut",
		"mun_greenhouse_medium_2",
		"w1596",
		"qt_russianresidence",
		"w1615",
		"res_candle_light_cottage",
		"sugar",
		"govcenterQuest",
		"grantSkyscraperNeighborhood",
		"sunset2_q_xmas_saga_act1_3",
		"material_expo_center_card_scanners",
		"w1582",
		"material_expo_center_gift_bags",
		"cream",
		"q_metro_elder_one_1",
		"w1590",
		"w1591",
		"uflag_clerkoffice_upgrades",
		"w1589",
		"w1585",
		"q_citymap_saga_act3_3_1",
		"solar_plant_1",
		"w1604",
		"w1558",
		"w1588",
		"bus_tall_casino",
		"sunset2_q_expansion_game_flying_2",
		"mystery_restaurant",
		"sunset2_q_expansion_weather",
		"w1575",
		"material_taxi_fare_meter",
		"material_switchboard",
		"grantAirport",
		"mun_metro_jail",
		"sunset2_q_summer_adventure_elder_1",
		"skyscraper_neighborhood_1",
		"w1572",
		"w1592",
		"material_satellite",
		"construction_5x5_4stage",
		"material_phone",
		"w1600",
		"w1611",
		"w1612",
		"material_robotic_arm",
		"q_citymap_saga_act3_2_1",
		"w1606",
		"material_tomatogrower",
		"subscription_first_var4_8",
		"w1583",
		"res_govmansion",
		"subscription_var4_8",
		"w1597",
		"res_metro_superegg",
		"grantSailboat",
		"deco_mystery_seedling_winter",
		"qux_icecreamshop",
		"material_self_check",
		"qux_chinesefireworks",
		"qux_bbqrestaurant",
		"material_sprinklersystem",
		"48",
		"sunset2_q_expansion_nessi_jr",
		"mun_brazillian_F1_track",
		"material_gardeningtools",
		"w1633",
		"material_fiber_optic_wire",
		"sunset1_q_ice_factory_1",
		"material_headsets",
		"sunset2_qf_bullscountdown_1",
		"w1621",
		"interstitial_v2",
		"w1614",
		"material_hard_hat",
		"sunset1_q_halloween_saga_act2_1",
		"q_halloween_saga_act3_1",
		"q_city2ndbirthday_1",
		"sunset1_q_halloween_saga_act2_2",
		"appliedCoupon",
		"ski_equipmeent_store",
		"sunset2_q_govt_building_1",
		"w1628",
		"bus_flowerkiosk",
		"mun_townhall_3",
		"rest_mun_pond_retreat",
		"w1668",
		"mun_insurancebuilding",
		"w1608",
		"permits_2",
		"material_arches",
		"sunset1_q_polarbear_rescue_1",
		"w1598",
		"w1631",
		"material_safety_goggles",
		"material_leveler",
		"ship_junkboat",
		"w1607",
		"holidaytown_1",
		"w1622",
		"qf_winter_trees",
		"w1610",
		"honeymoonhotelcollection",
		"w1613",
		"w1609",
		"XPromoBuilding",
		"material_duct_tape_blue",
		"holidaytown_2",
		"material_protractor",
		"w1627",
		"soda",
		"holidaytown_3",
		"w1669",
		"w1620",
		"w1623",
		"grantNewYear2012Res",
		"holidaytown_4",
		"q_minigolf",
		"w1619",
		"inplay",
		"canRedeemCount",
		"bus_shoestore",
		"material_lunchbox",
		"w1618",
		"bus_spa",
		"res_dub_clock_tower",
		"sunset2_q_alien_rescue_1",
		"res_night_apartment",
		"w1650",
		"expansion_cityatnight",
		"w1629",
		"qf_rollercoaster",
		"w1626",
		"mun_waterpark_a",
		"w1667",
		"plot_heirloom_tomatoes",
		"sunset2_qf_elder_weddingplanner_1",
		"q_halloween_saga_act3_2",
		"citySign_2x12",
		"material_gondola_steel_line",
		"deco_lakefront_scene",
		"w1634",
		"wilderness_21",
		"lootOnce_city_mysterygift",
		"q_halloween_saga_act2_3",
		"w1640",
		"w1624",
		"material_gondola_skyclamp",
		"res_usa_capecodhouseB",
		"w1649",
		"w1673",
		"traincollection2",
		"w1630",
		"w1662",
		"w1652",
		"material_grand_concert_hall_projector_bulbs",
		"material_expo_center_thumb_tacks",
		"material_duct_tape_purple",
		"date",
		"pandacollection",
		"q_holiday2012_saga_act3_3",
		"deco_rosegarden_redroseshrubs",
		"w1666",
		"Residence_City_At_Night",
		"q_cityatnight_saga_3_1DropTable",
		"hood_agnostic_level1",
		"material_grand_concert_hall_theatre_chairs",
		"w1678",
		"mun_bank_3",
		"material_slipper",
		"deco_nightlamp_a",
		"w1644",
		"material_grand_concert_hall_floor_microphones",
		"qm_farmville2_1",
		"material_expo_center_table_tents",
		"w1647",
		"bird_sanctuary",
		"sunset2_q_expansion_game_flying_1",
		"spooky_graveyard_1",
		"w1676",
		"w1677",
		"w1632",
		"q_expansion_pumpkin",
		"mun_arcdetriomphe",
		"w1665",
		"CityBDay2012Sunset",
		"w1643",
		"lootOnce_mysterygift_holiday",
		"qf_cars",
		"w1651",
		"sunset1_q_expansion_tree_of_life",
		"mun_alps_station_2",
		"w1638",
		"mun_communitytower",
		"mumbai_plane",
		"lake_res_hermit_hill_house",
		"w1656",
		"w1670",
		"w1636",
		"w1639",
		"w1645",
		"deco_rosegarden_whiteroseshrubs",
		"sunset1_qf_halloween_3_3",
		"qm_chefville_1",
		"w1642",
		"w1672",
		"q_expansion_monorail",
		"material_pb_sunglasses",
		"w1641",
		"material_beachcabana",
		"bus_web_3",
		"sunset1_q_halloween_saga_act1_2",
		"w1661",
		"cp_tinted_windows",
		"w1625",
		"w1663",
		"mun_clinic_3",
		"w1646",
		"w1648",
		"w1635",
		"w1655",
		"w1637",
		"w1710",
		"bus_tikibirdshop_3",
		"w1671",
		"w1703",
		"task",
		"bus_ski_lodge_1",
		"w1664",
		"material_duct_tape_red",
		"bus_watchshop",
		"w1675",
		"premium_animal_hospital",
		"w1654",
		"w1682",
		"orchids",
		"tulips",
		"lastUnlockTime",
		"unlockedWonders",
		"daisies",
		"frenchfries",
		"material_slotmachine",
		"bus_internetprovider_2",
		"expansion_world_union",
		"material_pb_helmet",
		"roses",
		"macro_keys_aloha_1",
		"w1705",
		"lilies",
		"w1699",
		"qt_goforloot",
		"-48_-72",
		"sunset1_q_warehouse_upgrades",
		"w1657",
		"bandit_mugshot_5",
		"energy_5",
		"w1658",
		"copsNBandits_Appeared",
		"w1653",
		"copsNBandits_Capture",
		"enable",
		"foundEggCount",
		"material_hoodporchlight",
		"w1720",
		"cheeseburger",
		"material_petrifiedwood",
		"mech_stockexchange",
		"sunset2_q_expansion_UFO_cropcircle",
		"grantSkyscraerPlatinum",
		"w1659",
		"w8",
		"w1685",
		"material_hoodshutters",
		"ketchup",
		"grantCorporation",
		"w1683",
		"sunset1_qf_halloween_2_3",
		"sunset2_qf_elder_skyscraper",
		"w1702",
		"material_roulette",
		"material_lantern",
		"chickenstrips",
		"w1680",
		"w1674",
		"announcements_civilunionhall_upgrades",
		"sunset2_q_expansion_dragonboat",
		"material_creakingtiles",
		"w1706",
		"w1684",
		"w1681",
		"mun_grant_office_1",
		"construction_3x3_4stage_frozen_tile",
		"w1698",
		"qm_windfarm_again",
		"sunset2_qf_country_club",
		"material_crumblebricks",
		"material_intlsaga_camcorder2",
		"hood_skiresort_level1",
		"w1719",
		"material_hoodcobblestones",
		"w1721",
		"w1679",
		"qux_premiumGoodsBar",
		"announcements_newperms",
		"w1733",
		"qf_monopoly_1",
		"w1689",
		"mun_super_solar_center",
		"w1700",
		"w1724",
		"lake_mun_historical_lake_society",
		"w1712",
		"fireworks_barge",
		"material_coreloop_microscope",
		"qf_citygames_escapade_1",
		"qf_minicoreloop_gov",
		"w1722",
		"w1723",
		"w1695",
		"q_halloween_skyscraper_1",
		"sfxDisabled",
		"material_greenbulb",
		"w1696",
		"w1711",
		"material_hoodtrellis",
		"startTS",
		"cornchowder",
		"material_hoodironfence",
		"w1707",
		"lm_venetianpalace",
		"ship_brazilcargoboat",
		"material_coreloop_pencil",
		"material_coreloop_pen",
		"grits",
		"q_alien_rescue_1",
		"w1694",
		"w1697",
		"w1732",
		"bus_icecreamshop_2",
		"w1701",
		"res_forest_apartment_02",
		"w1660",
		"cornonthecobb",
		"straight_SW_NE",
		"w1709",
		"w1714",
		"cornfritters",
		"w1704",
		"q_st_paddys_leprechaun_bridge_1",
		"sunset1_q_halloween_saga_act2_3",
		"cornbread",
		"material_grow_light",
		"w1715",
		"w1687",
		"material_district_key",
		"w1725",
		"w1727",
		"bridge_center",
		"material_district_display",
		"w1713",
		"w1787",
		"qt_chinesebusiness",
		"w1692",
		"material_fertilizer_injector",
		"sunset2_q_expansion_famous_businesses",
		"w1750",
		"announcements_cv_weather_announcement",
		"farmerscollection",
		"w1688",
		"w1729",
		"expansion_2013_item_1",
		"material_humidity_controller",
		"w1716",
		"bus_internetprovider_3",
		"w1772",
		"w1737",
		"w1736",
		"w1726",
		"w1743",
		"construction_6x6_10stage_waterfall_powerstation",
		"material_passenger_window",
		"w1690",
		"w1708",
		"material_wedding_rose",
		"w1730",
		"w1755",
		"sunset1_q_expansion_ny_blowout",
		"q_rec_center_1",
		"w1734",
		"sunset2_q_expansion_fireworks_factory",
		"material_taxi_roof_sign",
		"w1788",
		"sunset2_q_rec_center_1",
		"previousScratches",
		"w1752",
		"cp_car_stereo",
		"material_upgraderesidence_stencil",
		"w1731",
		"w1735",
		"qm_remodeling",
		"plot_flax",
		"sunset2_q_expansion_roadside_hotairballoon",
		"announcements_cv_dt_factory1_wing1_master",
		"material_floatation_device",
		"material_taxi_license",
		"qf_crop_mastery",
		"w1786",
		"w1693",
		"res_NYE2012_house01",
		"sunset1_qf_halloween_1_3",
		"w1718",
		"deco_polarbear_mama_1",
		"sunset1_q_halloween_saga_act1_1",
		"w1748",
		"w1717",
		"res_winter11_menorah02",
		"deco_northpole",
		"macro_business_district",
		"Region_Japan",
		"q_intnl_saga_act2_questline1_DropTable1",
		"q_citymap_saga_act1_3_1",
		"w1742",
		"sunset2_qf_fireworks_main_1",
		"qm_retail1",
		"grantFireworksMain",
		"w1728",
		"w1760",
		"sunset2_qf_halloween_space_savers_1",
		"w1789",
		"w1691",
		"w1756",
		"w1749",
		"w1785",
		"w1747",
		"w1784",
		"w1740",
		"w1744",
		"bus_mexicanrest",
		"6",
		"w1761",
		"hood_skyrise_level1",
		"expansion_nationalPark",
		"mun_swiss_museum",
		"sunset2_q_elder_dragon_castle_1",
		"w1745",
		"w1790",
		"material_rain_barrel",
		"w1792",
		"w1686",
		"material_expo_center_expo_kiosks",
		"w1769",
		"material_round_composter",
		"w1754",
		"qf_wedding_1",
		"material_expo_center_exhibit_brochures",
		"grantSpookyGraveyard",
		"expansion_2013_item_3",
		"grantmun_youth_athletic_center",
		"expansion_2013_item_2",
		"sunset1_q_expansion_hedge_fund",
		"material_tray_insert",
		"qm_bubblesafari_1",
		"bus_arcade",
		"w1739",
		"qf_trick_or_treat",
		"w1773",
		"q_citymap_saga_act2_2_1",
		"mun_spring_space_saver_cool_estates_1",
		"w1741",
		"q_citymap_saga_act1_1_1",
		"i-2",
		"grantItalianCastle",
		"sunset2_qf_halloween_1_2",
		"w1758",
		"res_customresidences_v2",
		"mun_centralpark_7",
		"w1757",
		"qf_embassy_uk",
		"q_expansion_fireworks_factory",
		"ny_blowout_disco_expand",
		"sunset2_q_football_1",
		"sunset2_qf_elder_tinsel",
		"w1738",
		"expansion_world_traveler",
		"res_NYE2012_house03",
		"qt_venetianresidence",
		"w1793",
		"w1746",
		"q_neighborhood_o_hoods",
		"w1759",
		"material_zoningpermit_stamp",
		"w1770",
		"sunset1_q_outdoor_theater_1",
		"material_staff_phone",
		"w1799",
		"w1830",
		"w1798",
		"mun_irish_ruins_lv1",
		"w1751",
		"material_farmers_butterchurn",
		"grantBuckinghamPalace",
		"q_citymap_saga_act1_2_1",
		"w1771",
		"w1794",
		"w1765",
		"sunset2_q_arc_1",
		"w1802",
		"w1806",
		"w1808",
		"w1825",
		"StPaddys2013",
		"mun_constructioncompany",
		"w1775",
		"w1791",
		"mall_international_1",
		"sunset1_q_ski_lodge_1",
		"q_bookchapter2",
		"w1795",
		"sunset1_q_super_greenhouse",
		"locked",
		"w1763",
		"rainbow_1",
		"bridalcollection",
		"material_airport_info_desk",
		"material_subway_map",
		"material_zoningpermit_permitstencil",
		"w1812",
		"q_rollercoaster_expansion",
		"qf_wonder_craft",
		"w1774",
		"w1779",
		"mun_metro_downtowncablecompany",
		"q_wildcard_1",
		"res_solarhouse_A",
		"w1762",
		"w1766",
		"material_staff_computer",
		"sunset1_qm_lovestory_act3_1",
		"w1753",
		"q_citymap_saga_act2_1_1",
		"material_baggage_xray",
		"awardUserXP",
		"sunset1_q_expansion_2013_game_main",
		"bus_shopping_street",
		"q_newyear_res_go_1_flag",
		"q_newyear_res_go_2_flag",
		"q_newyear_res_go_3_flag",
		"q_newyear_res_live_1_flag",
		"q_newyear_res_live_2_flag",
		"q_newyear_res_live_3_flag",
		"cp_custom_paintjob",
		"w1868",
		"w1800",
		"quest_item_gold_energy_bulb",
		"q_newyear_res_help_1_flag",
		"q_newyear_res_help_2_flag",
		"q_newyear_res_help_3_flag",
		"material_air_traffic_display",
		"w1811",
		"dtgala_rm",
		"w1807",
		"q_expansion_natural_wonders",
		"q_lakefront_saga_act2_1",
		"qt_waterfall_timed",
		"w1780",
		"polar_bear_rescue_expansion",
		"q_xmas_saga_act3_2",
		"city_mysterygift",
		"w1826",
		"w1833",
		"bandit_mugshot_4",
		"qux_summer_adventure_1",
		"qux_summer_adventure_2",
		"w1783",
		"w1810",
		"announcements_lovestory2012_announce",
		"q_lakefront_civic_center_1",
		"qt_halloween2012",
		"deco_zooplainstree02",
		"q_expansion_pony",
		"mun_shellcarnivalsmall",
		"w1764",
		"w1805",
		"w1781",
		"w1801",
		"material_grand_concert_hall_stage_pullies",
		"w1803",
		"q_lakefront_saga_act1_1",
		"material_broadway_push_broom",
		"sunset2_q_expansion_gala",
		"seoul_plane",
		"material_grand_concert_hall_stage_ropes",
		"w1832",
		"w1767",
		"w1804",
		"bus_petstore",
		"qf_winter_coreloop_1",
		"ItemStorage",
		"material_grand_concert_hall_conductor_baton",
		"material_nessi_whiteflower",
		"WhatCity",
		"awardedGrandPrize",
		"questions",
		"accessTime",
		"mun_mad_scientist_lab",
		"polar_bear_rescue_expansion_4",
		"polar_bear_rescue_expansion_3",
		"cp_car_alarm",
		"deco_mystery_animal",
		"material_business_tower_table",
		"w1814",
		"w1797",
		"w1813",
		"material_business_tower_poolbar",
		"polar_bear_rescue_expansion_2",
		"macro_trucking_depot",
		"w1777",
		"w1768",
		"w1782",
		"material_grand_concert_hall_curtain_tassels",
		"w1816",
		"sunset1_q_halloween_skyscraper_1",
		"w1821",
		"material_blue_bird",
		"w1776",
		"w1819",
		"w1867",
		"w1834",
		"material_business_tower_barseat",
		"cp_leather_seats",
		"w1820",
		"w1824",
		"mun_endangered_species_lv1",
		"w1837",
		"w1840",
		"w1828",
		"w1809",
		"q_winter_wonderland_1",
		"w1817",
		"w1831",
		"bus_crabshack",
		"qm_upgrade_business",
		"w1778",
		"w1848",
		"w1844",
		"w1829",
		"w1865",
		"w1823",
		"q_citymap_saga_act2_3_1",
		"material_broadway_star_room",
		"q_expansion_space_shuttle_v2",
		"q_business_tower_1",
		"w1836",
		"w1847",
		"w1796",
		"q_expansion_game_flying_3",
		"CITY-29558_free_stuff",
		"w1849",
		"grantBusDistrict",
		"w1869",
		"w1815",
		"announce_cityhall_upgrades",
		"mall_world_gourmet_center_1",
		"deco_bus_station_level1",
		"w1920",
		"construction_2x2_5stage",
		"w1839",
		"w1843",
		"w1866",
		"w1841",
		"AlpsTrainLeaderboard",
		"sunset1_q_spanish_castle_1",
		"material_red_bird",
		"glassornament",
		"contractPlant",
		"w1871",
		"w1855",
		"w1862",
		"sunset1_qt_treeoflife",
		"sunset1_q_halloween_saga_act1_3",
		"w1827",
		"w1818",
		"material_district_display_gold",
		"qm_vacation",
		"q_expansion_game_flying_2",
		"material_district_key_gold",
		"w1853",
		"w1845",
		"w1835",
		"bus_icecreamshop_3",
		"sunset2_q_expansion_monorail",
		"w1863",
		"qf_super_res_hall_1",
		"mun_country_club",
		"sunset2_q_intl_super_res_1",
		"sunset1_q_expansion_space_shuttle_v2",
		"w1850",
		"stpatricks_fiddle",
		"sunset1_qf_stjude_quest",
		"sunset1_q_halloween_saga_act3_1",
		"material_broadway_backstage_pass",
		"w1864",
		"material_watercrops_net",
		"w1822",
		"w1872",
		"w1859",
		"snow_flake_bakery_1",
		"weather_balloon",
		"w1919",
		"sunset1_q_kaboom_1",
		"material_broadway_lit_mirror",
		"deco_mangrove",
		"w1876",
		"w1858",
		"w1842",
		"stpatricks_leprechaunhat",
		"qm_lovestory_act1_1",
		"w1897",
		"deco_forest_stone_sidewalk_1",
		"houseplant",
		"stpatricks_rainbow",
		"stpatricks_potofgold",
		"mun_trickortreathouse",
		"material_nessi_redflower",
		"w1896",
		"mun_passportoffice",
		"stpatricks_clover",
		"w1846",
		"res_france_cottage_blue",
		"sunset2_qf_halloween_1",
		"w1851",
		"w1857",
		"res_cultural_house_1",
		"sunset1_q_expansion_corporations",
		"w1930",
		"announcements_warehouse_upgrade",
		"w1854",
		"deskfan",
		"w1899",
		"cdplayer",
		"mun_grand_canyon",
		"deco_tikibowlingalley",
		"w1898",
		"skyscraper_cetronas_center_6",
		"sunset2_qf_stock_exchange",
		"w1838",
		"mun_youth_athletic_center",
		"contractHarvest",
		"sunset1_qf_mj_1",
		"coffeemachine",
		"miniblinds",
		"material_hotelrobe",
		"sunset2_q_broadway_actingschool_1",
		"bridge_center_4",
		"grantSkyScraperJapanese",
		"material_marblesinktop",
		"w1852",
		"w1877",
		"w1856",
		"bus_italrestbs",
		"res_adventure_house_1",
		"w1960",
		"bus_diner",
		"w1874",
		"w1878",
		"w1894",
		"w1906",
		"qf_stjude_quest",
		"sunset1_qm_lovestory_act2_1",
		"w1880",
		"w1900",
		"res_customresidences",
		"swing_ride",
		"sunset1_q_expansion_hotairballoon",
		"w1875",
		"qt_treeoflife",
		"w1905",
		"announcements_bridge",
		"q_lakefront_saga_act3_1",
		"res_st_patrick_housestone",
		"q_expansion_game_flying_1",
		"vacationDestination",
		"chess",
		"w1861",
		"w1907",
		"w1908",
		"material_hotelbell",
		"friend_family_point",
		"w1884",
		"q_natural_wonder_craft_1",
		"satisfyHotelAOEGate",
		"q_skyway_gardens_1",
		"material_threePigs_straw_green",
		"w1909",
		"w1860",
		"w1931",
		"sunset1_qf_tajmahal",
		"w1885",
		"w1895",
		"qf_space_shuttle",
		"material_threePigs_straw_black",
		"q_xmas_saga_act3_3",
		"q_expansion_UFO_cropcircle",
		"q_pyg_museum",
		"w1883",
		"w1903",
		"material_threePigs_straw_blue",
		"q_whale_rescue_1",
		"bus_tikibirdshop_2",
		"w1873",
		"w1918",
		"qf_birthday_2011",
		"w1879",
		"w1881",
		"q_ponyranch_1",
		"w1959",
		"w1921",
		"laptop",
		"sunset2_q_windfarm_upgrade_1",
		"deco_reindeer_icesculpture",
		"enclosure_savannah",
		"q_elder_dragon_castle_1",
		"w1892",
		"grantNatureHood",
		"w1902",
		"sunset1_q_halloween_saga_act3_2",
		"q_xmas_saga_act2_1",
		"sunset1_qf_halloween_3_2",
		"sunset1_q_buckingham_palace_1",
		"w1870",
		"w1891",
		"robotvacuum",
		"sunset1_qf_solar_greenhouse",
		"microwave",
		"mall_skygarden_1",
		"w1957",
		"w1961",
		"satellitedish",
		"w1886",
		"beanbag",
		"out_of_cash_visit",
		"out_of_cash_visit_two",
		"w1911",
		"w1955",
		"sunset1_qf_halloween_2_2",
		"w1904",
		"nyeCountDown2012_day8",
		"w1929",
		"w1942",
		"expansion_dragonCastle",
		"w1889",
		"w1882",
		"w1887",
		"w1917",
		"deco_flowerPatchPink",
		"w1910",
		"w1914",
		"q_opening_ceremony_expansion",
		"w1962",
		"sunset2_q_expansion_rotonda",
		"qt_supermuni_weatherballoon",
		"w1888",
		"Heliport",
		"w1890",
		"carrotsalad",
		"car_propel",
		"w1913",
		"zooUnlock_enclosure_savannah",
		"hood_superhood_neighborhood_level1",
		"w1963",
		"land_tile_expansion_fallback_2",
		"w1965",
		"deco_halloween_sidewalk_2",
		"carrotsoup",
		"mun_cityworks",
		"res_intl_denmark_building",
		"bus_bikeshop_2",
		"w1901",
		"mun_centralpark",
		"w1941",
		"bus_burgertofu",
		"hood_halloween_1",
		"q_xmas_saga_act1_2",
		"carrotcake",
		"w1916",
		"sunset1_qt_crooked_house",
		"sunset1_q_dragoncastle",
		"sunset1_qt_waterfall_timed",
		"carrotsticks",
		"w1974",
		"material_duct_tape_brown",
		"babycarrots",
		"grantLoveStory2012Adventure",
		"grantLoveStory2012Gym",
		"grantLoveStory2012Cultural",
		"w1981",
		"heliport_pad_tourist",
		"dtwn_mun_fancy_conventioncenter",
		"sunset2_q_clocktower",
		"q_solar_super_civic_center_1",
		"sunset1_qf_halloween_2",
		"sunset2_q_coal_mine_1",
		"w1893",
		"shamrock_pond",
		"w1912",
		"w1915",
		"w1949",
		"hasPlacedAnimalsInDisplay",
		"w1968",
		"animal_jaguar",
		"heliport_pad_supply",
		"w1964",
		"w1980",
		"w1926",
		"w1973",
		"w1958",
		"announcements_cv_premiumbandits_main",
		"w1935",
		"sunset1_q_coreloop_08092012",
		"hall_mun_haunted_schoolhouse",
		"material_badge",
		"w1939",
		"material_coreloop_pipettes",
		"material_siren",
		"q_santa_sleigh_1",
		"frozen_land_tile_expansion",
		"mun_ghost_civic_center",
		"w1979",
		"w1922",
		"bus_sunglasses_3",
		"w1976",
		"material_coreloop_sciencegoggles",
		"sunset1_qm_lovestory_act1_1",
		"district_2",
		"qf_premium_bandits",
		"sunset2_qf_lovecars",
		"w1938",
		"w1953",
		"w1943",
		"cp_spoiler",
		"fleur_de_lis_doober_drop",
		"w1923",
		"mun_lakefront_civic_center",
		"contract_hawaii_vacation",
		"material_equalityplaza_glass_elevator",
		"hotel_sailboat_low",
		"secret_cave",
		"w1936",
		"grantHalloweenNeighborhood",
		"grantHalloweenCivicCenter",
		"w1975",
		"w1977",
		"-36_-72",
		"material_outfit",
		"w1947",
		"Theme_Alps_Grindable",
		"w2005",
		"4",
		"w1928",
		"material_famous_restaurant_cookbook",
		"sunset2_q_winter_wonderland_1",
		"w1925",
		"material_coreloop_testtube",
		"43",
		"sunset1_q_expansion_nessi_jr",
		"bus_frenchrest_2",
		"w1944",
		"w2023",
		"trucking_truck_SE",
		"w1956",
		"grantNessi",
		"material_equalityplaza_lobby_mailbox",
		"q_grow_pumpkin_1",
		"hedge_fund",
		"w2056",
		"material_ukulele",
		"bus_sunglasses_2",
		"w1924",
		"w1966",
		"w2058",
		"bus_irishrestaurant",
		"w1984",
		"mun_stjude_hospital",
		"expansion_roseGarden",
		"w2026",
		"w1972",
		"mun_italian_milan_opera",
		"w1937",
		"flower_rose_peach",
		"w1927",
		"qux_faeriefragrances",
		"w2053",
		"w1978",
		"material_ball_and_chain",
		"w1983",
		"w1946",
		"bus_web",
		"w1934",
		"w1985",
		"w1932",
		"mun_travel_agency",
		"qt_coreloop_52912",
		"w1967",
		"w2004",
		"deco_chinesetheater",
		"energy_12_buy",
		"w1954",
		"asiancuisinecollection",
		"w2020",
		"q_xmas_saga_act2_2",
		"q_what_city_1",
		"cp_truck_lights",
		"grantSuperPier",
		"w1982",
		"w1994",
		"mun_firehouse",
		"w1995",
		"w2008",
		"material_duct_tape_yellow",
		"land_tile_expansion_fallback_3",
		"w2052",
		"w2016",
		"sunset1_qf_elder_wondercraft_1",
		"bus_poolhall",
		"w1950",
		"sunset1_q_pyg_museum",
		"w1945",
		"w2057",
		"material_tablemat",
		"w2038",
		"w1933",
		"mun_family_office_1",
		"w2040",
		"bus_walmart_intl",
		"material_grand_concert_hall_theatre_screes",
		"qf_detective_game_1",
		"sunset2_qm_skyscrapers2_1",
		"w1948",
		"w1952",
		"w1970",
		"w2044",
		"bus_bikeshop_3",
		"qf_snowman_1",
		"npc_check_in_tutorial",
		"material_grand_concert_hall_door_hinges",
		"w1971",
		"Animal_horsewhite",
		"bus_robotstore_2",
		"115",
		"mun_streetcarnivalsmall",
		"w2051",
		"w2037",
		"w1969",
		"w2019",
		"announcements_downtown_subway",
		"w2024",
		"w1940",
		"qf_equality_district",
		"w2025",
		"qf_halloween_space_savers_1",
		"w2000",
		"w2039",
		"w2041",
		"cp_floor_mats",
		"w2059",
		"mun_metro_symphonyhall",
		"q_nighttime_space_needle_1",
		"w2009",
		"w2048",
		"q_expansion_tree_of_life",
		"qux_irishrestaurant",
		"bus_irishrestaurant_3",
		"w1997",
		"hall_mun_ghoul_school",
		"w2047",
		"w2049",
		"qf_nyegift_1",
		"q_expansion_space_shuttle",
		"Farmers_Grindable",
		"picklejarstable",
		"resort_hotel_low",
		"w2055",
		"w1998",
		"w1991",
		"w2102",
		"w2045",
		"marblebust",
		"waterfall",
		"addLootToInventory",
		"irongate",
		"w2003",
		"qf_chinese_booth",
		"mun_museum_3",
		"w2042",
		"w2017",
		"w1999",
		"cp_sparewheel",
		"q_expansion_landfill",
		"q_xmas_saga_act3_1",
		"w2007",
		"material_watercrops_bait",
		"w1988",
		"qm_cincodemayo_1",
		"w1993",
		"water_helicopter",
		"mun_candyticketbooth",
		"w1987",
		"birdfountain",
		"w2050",
		"lastAnnouncementSeenAt",
		"sunset1_q_xmas_saga_act3_1",
		"w2027",
		"q_xmas_saga_act1_3",
		"deco_ATM",
		"sunset1_q_amphitheater_1",
		"sunset1_q_holiday2012_saga_act3_2",
		"w2081",
		"material_trains_coalcar",
		"w1951",
		"w74",
		"chandelier",
		"w2061",
		"material_watercrops_anchor",
		"w2028",
		"deco_giraffe_icesculpture",
		"mun_spring_space_saver_cool_center_1",
		"announcements_cv_dt_factory1_wing2_master",
		"sunset1_q_expansion_broadway",
		"w2060",
		"downtown_portal_a_f1",
		"w2021",
		"qf_CVG_pins_1",
		"w1992",
		"sunset1_qf_elder_gala",
		"deco_parkingmeter",
		"q_outdoor_theater_1",
		"w2022",
		"w2029",
		"w2043",
		"w2097",
		"Bridge",
		"mun_rollercoaster_1",
		"w1989",
		"w2012",
		"w2006",
		"w2030",
		"w1990",
		"w2062",
		"sunset1_qf_grandcanyon",
		/* Add more static strings here.
		   Note: for compatible consideration, this table should be only appended. DON'T modify/delete any exsiting string. */
	};

static char* EMPTY_STRING="";

extern void normal_od_wrapper_serialize(od_igbinary_serialize_data* igsd, zval* obj, uint8_t is_root);

/* {{{ Serializing functions prototypes */

inline static int od_igbinary_serialize64(od_igbinary_serialize_data *igsd, uint64_t i TSRMLS_DC);

inline static int od_igbinary_serialize_null(od_igbinary_serialize_data *igsd TSRMLS_DC);
inline static int od_igbinary_serialize_bool(od_igbinary_serialize_data *igsd, int b TSRMLS_DC);


inline static int od_igbinary_serialize_double(od_igbinary_serialize_data *igsd, double d TSRMLS_DC);

inline static int od_igbinary_serialize_chararray(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC);

inline static int od_igbinary_serialize_chararray_ex(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC);

inline static int od_igbinary_unserialize_string_from_table(od_igbinary_unserialize_data *igsd, char **s, uint32_t *len, uint32_t pos TSRMLS_DC);

inline static int od_igbinary_serialize_array_ref(od_igbinary_serialize_data *igsd, zval *z, bool object TSRMLS_DC);
inline static int od_igbinary_serialize_array_sleep(od_igbinary_serialize_data *igsd, zval *z, HashTable *ht, zend_class_entry *ce, bool incomplete_class TSRMLS_DC);
inline static int od_igbinary_serialize_object_name(od_igbinary_serialize_data *igsd, const char *name, uint32_t name_len TSRMLS_DC);
inline static int od_igbinary_serialize_object(od_igbinary_serialize_data *igsd, zval *z TSRMLS_DC);

/* }}} */
/* {{{ Unserializing functions prototypes */
inline static int od_igbinary_unserialize_string(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC);

inline static int od_igbinary_unserialize_array(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z, int object TSRMLS_DC);
inline static int od_igbinary_unserialize_object(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z TSRMLS_DC);
inline static int od_igbinary_unserialize_object_ser(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z, zend_class_entry *ce TSRMLS_DC);

//odus doesn't allow object references
//inline static int od_igbinary_unserialize_ref(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z TSRMLS_DC);

/* }}} */

inline static void adjust_len_info(od_igbinary_serialize_data *igsd, uint32_t n, uint32_t new_n, uint old_len_bytes, uint new_len_bytes, uint32_t old_len_pos);

inline int od_igbinary_init(TSRMLS_D) {
	int i;
	int len;

	od_static_strings_count = sizeof(od_static_strings) / sizeof(char*);
	
	if (!od_static_strings) {
		od_error(E_ERROR, "od_igbinary_init: fail to alloc memory");
		return 1;
	}

	hash_si_init(&od_static_strings_hash, 16);
	for (i = 0; i < od_static_strings_count; i++) {
		len = strlen(od_static_strings[i]);

		hash_si_insert(&od_static_strings_hash, od_static_strings[i], len, i);
	}

	return 0;
}

inline int od_igbinary_shutdown(TSRMLS_D) {
	hash_si_deinit(&od_static_strings_hash);

	return 0;
}

/* {{{ int od_igbinary_serialize(uint8_t**, uint32_t*, zval*) */
int od_igbinary_serialize(uint8_t **ret, uint32_t *ret_len, zval *z TSRMLS_DC) {
	od_igbinary_serialize_data igsd;

	if (od_igbinary_serialize_data_init(&igsd, Z_TYPE_P(z) != IS_OBJECT && Z_TYPE_P(z) != IS_ARRAY TSRMLS_CC)) {
		od_error(E_ERROR, "od_igbinary_serialize: cannot init igsd");
		return 1;
	}

	if (od_igbinary_serialize_header(&igsd TSRMLS_CC) != 0) {
		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	if (od_igbinary_serialize_zval(&igsd, z TSRMLS_CC) != 0) {
		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	if (igsd.compact_strings) {
		if (od_igbinary_serialize_string_table(&igsd TSRMLS_CC) != 0) {
			od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
			return 1;
		}
	}

	*ret_len = igsd.buffer_size;

	/* Explicit nul termination */
	if(od_igbinary_serialize8(&igsd,0)!=0)
	{
		od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	*ret = igsd.buffer;

	igsd.buffer = NULL;

	od_igbinary_serialize_data_deinit(&igsd TSRMLS_CC);

	return 0;
}
/* }}} */
/* {{{ int od_igbinary_unserialize(const uint8_t *, uint32_t, zval **) */
int od_igbinary_unserialize(const uint8_t *buf, uint32_t buf_len, zval **z TSRMLS_DC) {
	od_igbinary_unserialize_data igsd;
	uint32_t header = -1;

	od_igbinary_unserialize_data_init(&igsd TSRMLS_CC);

	igsd.buffer = (uint8_t *) buf;
	igsd.buffer_size = buf_len;
	igsd.original_buffer = igsd.buffer;

	if (od_igbinary_unserialize_header(&igsd, &header TSRMLS_CC)) {
		od_igbinary_unserialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	if (igsd.compact_strings) {
		if (od_igbinary_unserialize_init_string_table(&igsd TSRMLS_CC)) {
			od_igbinary_unserialize_data_deinit(&igsd TSRMLS_CC);
			return 1;
		}
	}

	if (od_igbinary_unserialize_zval(&igsd, z TSRMLS_CC)) {
		od_igbinary_unserialize_data_deinit(&igsd TSRMLS_CC);
		return 1;
	}

	od_igbinary_unserialize_data_deinit(&igsd TSRMLS_CC);

	return 0;
}
/* }}} */
/* {{{ proto string od_igbinary_unserialize(mixed value) */
PHP_FUNCTION(od_unserialize) {
	char *string;
	int string_len;

	(void) return_value_ptr;
	(void) this_ptr;
	(void) return_value_used;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &string, &string_len) == FAILURE) {
		RETURN_NULL();
	}

	if (string_len <= 0) {
		RETURN_NULL();
	}

	if (od_igbinary_unserialize((uint8_t *) string, string_len, &return_value TSRMLS_CC)) {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ od_igbinary_serialize_data_init */
/** Inits od_igbinary_serialize_data. */
inline int od_igbinary_serialize_data_init(od_igbinary_serialize_data *igsd, bool scalar TSRMLS_DC) {
	int r = 0;
	uint32_t format_version = (uint32_t)ODUS_G(format_version);

	igsd->buffer = NULL;
	igsd->buffer_size = 0;
	//igsd->buffer_capacity = 32;
	igsd->buffer_capacity = OD_RESERVED_BUFFER_LEN;

	igsd->buffer = (uint8_t *) emalloc(igsd->buffer_capacity);
	if (igsd->buffer == NULL) {
		return 1;
	}

	igsd->scalar = scalar;
	if (!igsd->scalar) {
		hash_si_init(&igsd->strings, 16);
		hash_si_init(&igsd->objects, 16);
	}

	if (format_version == OD_IGBINARY_FORMAT_VERSION_02) {
		igsd->compact_strings = true;
		igsd->compress_value_len = true;
	} else if (format_version == OD_IGBINARY_FORMAT_VERSION_01) {
		igsd->compact_strings = false;
		igsd->compress_value_len = false;
	} else {
		// Default, enable both.
		debug("od_igbinary_serialize_data_init: wrong format_version configured.");
		igsd->compact_strings = true;
		igsd->compress_value_len = true;
	}

	igsd->strings_count = 0;
	igsd->string_table_update = false;

	igsd->root_id = 0;
	return r;
}
/* }}} */
/* {{{ od_igbinary_serialize_data_deinit */
/** Deinits od_igbinary_serialize_data. */
inline void od_igbinary_serialize_data_deinit(od_igbinary_serialize_data *igsd TSRMLS_DC) {
	if (igsd->buffer) {
		efree(igsd->buffer);
	}

	if (!igsd->scalar) {
		hash_si_deinit(&igsd->strings);
		hash_si_deinit(&igsd->objects);
	}
}
/* }}} */

/* {{{ od_igbinary_serialize_resize */
/** Expandes od_igbinary_serialize_data. */
inline static int od_igbinary_serialize_resize(od_igbinary_serialize_data *igsd, uint32_t size TSRMLS_DC) {
	if (igsd->buffer_size + size < igsd->buffer_capacity) {
		return 0;
	}

	while (igsd->buffer_size + size >= igsd->buffer_capacity) {
		igsd->buffer_capacity *= 2;
	}

	igsd->buffer = (uint8_t *) erealloc(igsd->buffer, igsd->buffer_capacity);
	if (igsd->buffer == NULL)
		return 1;

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize_header */
/** Serializes header. */
inline int od_igbinary_serialize_header(od_igbinary_serialize_data *igsd TSRMLS_DC) {
	uint32_t format_version = (uint32_t)ODUS_G(format_version);
	od_igbinary_serialize32(igsd, format_version | OD_IGBINARY_FORMAT_FLAG TSRMLS_CC); /* version */

	if (igsd->compact_strings) {
		// Jump over the string table info.
		if(od_igbinary_serialize_resize(igsd, OD_IGBINARY_STRING_TABLE_INFO_LEN)) {
			return 1;
		}
		igsd->buffer_size += OD_IGBINARY_STRING_TABLE_INFO_LEN;
	}

	return 0;
}
/* }}} */

inline void od_igbinary_serialize_append_zero(od_igbinary_serialize_data *igsd) {
	if(igsd->buffer_size >= igsd->buffer_capacity) {
		igsd->buffer = (uint8_t *) erealloc(igsd->buffer, igsd->buffer_size+1);
		igsd->buffer_capacity = igsd->buffer_size+1;
	}

	igsd->buffer[igsd->buffer_size]=0;
}

inline int od_igbinary_serialize_memcpy(od_igbinary_serialize_data *igsd, uint8_t* s, uint32_t len) {
	if(s == NULL || len == 0) return 1;

	if(od_igbinary_serialize_resize(igsd,len)) {
		return 1;
	}

	memcpy(igsd->buffer + igsd->buffer_size, s, len);

	igsd->buffer_size += len;

	return 0;
}


/* {{{ od_igbinary_serialize8 */
/** Serialize 8bit value. */
inline int od_igbinary_serialize8(od_igbinary_serialize_data *igsd, uint8_t i TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, 1 TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer[igsd->buffer_size++] = i;
	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize8_at */
/** Serialize 8bit value at specified position.. */
inline int od_igbinary_serialize8_at(od_igbinary_serialize_data *igsd, uint8_t i, uint32_t pos TSRMLS_DC) {
	igsd->buffer[pos++] = i;
	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize16 */
/** Serialize 16bit value. */
inline int od_igbinary_serialize16(od_igbinary_serialize_data *igsd, uint16_t i TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, 2 TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize16_at */
/** Serialize 32bit value at specified position. */
inline int od_igbinary_serialize16_at(od_igbinary_serialize_data *igsd, uint16_t i, uint32_t pos TSRMLS_DC) {
	igsd->buffer[pos++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[pos++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize32 */
/** Serialize 32bit value. */
inline int od_igbinary_serialize32(od_igbinary_serialize_data *igsd, uint32_t i TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, 4 TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 24 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 16 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */

inline int od_igbinary_serialize_skip_n(od_igbinary_serialize_data *igsd, int n TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, n TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer_size += n;

	return 0;
}

/* {{{ od_igbinary_serialize32_at */
/** Serialize 32bit value at specified position. */
inline int od_igbinary_serialize32_at(od_igbinary_serialize_data *igsd, uint32_t i, uint32_t pos TSRMLS_DC) {
	igsd->buffer[pos++] = (uint8_t) (i >> 24 & 0xff);
	igsd->buffer[pos++] = (uint8_t) (i >> 16 & 0xff);
	igsd->buffer[pos++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[pos++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize_value_len */
/** Serialize 32bit value. */
inline int od_igbinary_serialize_value_len(od_igbinary_serialize_data *igsd, uint32_t len, uint32_t pos TSRMLS_DC) {
	if (igsd->compress_value_len) {
		/** Compress size, use the first two bits to indicate length:
		 * 01: Len will occupy 1 byte;
		 * 10: Len will occupy 2 bytes in total;
		 * 11: Not used for now;
		 * 00: Len will occupy 4 bytes in total. This will be compatible with the old format (always be 4 bytes.) */
		uint32_t encode_len = 0;
		if (len <= 0x3f) {
			encode_len = len | 0x40;
			od_igbinary_serialize8_at(igsd, (uint8_t) encode_len, pos TSRMLS_CC);
			memmove(igsd->buffer + pos + 1, igsd->buffer + pos + OD_IGBINARY_VALUE_LEN_SIZE, igsd->buffer_size - OD_IGBINARY_VALUE_LEN_SIZE - pos);
			igsd->buffer_size -= 3;
		} else if (len <= 0x3fff) {
			encode_len = len | 0x8000;
			od_igbinary_serialize16_at(igsd, (uint16_t) encode_len, pos TSRMLS_CC);
			memmove(igsd->buffer + pos + 2, igsd->buffer + pos + OD_IGBINARY_VALUE_LEN_SIZE, igsd->buffer_size - OD_IGBINARY_VALUE_LEN_SIZE - pos);
			igsd->buffer_size -= 2;
		} else if (len <= 0x3fffffff) {
			encode_len = len;
			od_igbinary_serialize32_at(igsd, (uint32_t) encode_len, pos TSRMLS_CC);
		} else {
			od_error(E_ERROR, "od_igbinary_serialize_value_len: len is too large");
			return 1;
		}
		return 0;
	} else {
		return od_igbinary_serialize32_at(igsd, len, pos TSRMLS_CC);
	}
}
/* }}} */

/* {{{ od_igbinary_serialize64 */
/** Serialize 64bit value. */
inline static int od_igbinary_serialize64(od_igbinary_serialize_data *igsd, uint64_t i TSRMLS_DC) {
	if (od_igbinary_serialize_resize(igsd, 8 TSRMLS_CC)) {
		return 1;
	}

	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 56 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 48 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 40 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 32 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 24 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 16 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i >> 8 & 0xff);
	igsd->buffer[igsd->buffer_size++] = (uint8_t) (i & 0xff);

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_null */
/** Serializes null. */
inline static int od_igbinary_serialize_null(od_igbinary_serialize_data *igsd TSRMLS_DC) {
	return od_igbinary_serialize8(igsd, od_igbinary_type_null TSRMLS_CC);
}
/* }}} */
/* {{{ od_igbinary_serialize_bool */
/** Serializes bool. */
inline static int od_igbinary_serialize_bool(od_igbinary_serialize_data *igsd, int b TSRMLS_DC) {
	return od_igbinary_serialize8(igsd, (uint8_t) (b ? od_igbinary_type_bool_true : od_igbinary_type_bool_false) TSRMLS_CC);
}
/* }}} */
/* {{{ od_igbinary_serialize_long */
/** Serializes long. */
inline int od_igbinary_serialize_long(od_igbinary_serialize_data *igsd, long l TSRMLS_DC) {
	long k = l >= 0 ? l : -l;
	bool p = l >= 0 ? true : false;

	/* -LONG_MIN is 0 otherwise. */
	if (l == LONG_MIN) {
#if SIZEOF_LONG == 8
		od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_long64n TSRMLS_CC);
		od_igbinary_serialize64(igsd, (uint64_t) 0x8000000000000000 TSRMLS_CC);
#elif SIZEOF_LONG == 4
		od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_long32n TSRMLS_CC);
		od_igbinary_serialize32(igsd, (uint32_t) 0x80000000 TSRMLS_CC);
#else
#error "Strange sizeof(long)."
#endif
		return 0;
	}

	if (k <= 0xff) {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long8p : od_igbinary_type_long8n) TSRMLS_CC);
		od_igbinary_serialize8(igsd, (uint8_t) k TSRMLS_CC);
	} else if (k <= 0xffff) {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long16p : od_igbinary_type_long16n) TSRMLS_CC);
		od_igbinary_serialize16(igsd, (uint16_t) k TSRMLS_CC);
#if SIZEOF_LONG == 8
	} else if (k <= 0xffffffff) {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long32p : od_igbinary_type_long32n) TSRMLS_CC);
		od_igbinary_serialize32(igsd, (uint32_t) k TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long64p : od_igbinary_type_long64n) TSRMLS_CC);
		od_igbinary_serialize64(igsd, (uint64_t) k TSRMLS_CC);
	}
#elif SIZEOF_LONG == 4
	} else {
		od_igbinary_serialize8(igsd, (uint8_t) (p ? od_igbinary_type_long32p : od_igbinary_type_long32n) TSRMLS_CC);
		od_igbinary_serialize32(igsd, (uint32_t) k TSRMLS_CC);
	}
#else
#error "Strange sizeof(long)."
#endif

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_double */
/** Serializes double. */
inline static int od_igbinary_serialize_double(od_igbinary_serialize_data *igsd, double d TSRMLS_DC) {
	union {
		double d;
		uint64_t u;
	} u;

	od_igbinary_serialize8(igsd, od_igbinary_type_double TSRMLS_CC);

	u.d = d;

	od_igbinary_serialize64(igsd, u.u TSRMLS_CC);

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_string */
/** Serializes string.
 * Serializes each string once, after first time uses pointers.
 */
inline int od_igbinary_serialize_string(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC) {
	uint32_t t;
	uint32_t *i = &t;

	if (len == 0) {
		od_igbinary_serialize8(igsd, od_igbinary_type_string_empty TSRMLS_CC);
		return 0;
	}

	if (igsd->scalar || !igsd->compact_strings) {
		if (od_igbinary_serialize_chararray(igsd, s, len TSRMLS_CC) != 0) {
			return 1;
		}
	} else {	/* !igsd->scalar && igsd->compact_strings */
		if (hash_si_find(&od_static_strings_hash, s, len, i) == 0) {
			if (*i <= 0xff) {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id8 TSRMLS_CC);
				od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
			} else if (*i <= 0xffff) {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id16 TSRMLS_CC);
				od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
			} else {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id32 TSRMLS_CC);
				od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
			}
			return 0;
		}

		if (hash_si_find(&igsd->strings, s, len, i) == 1) {
			hash_si_insert(&igsd->strings, s, len, igsd->strings_count);
			*i = igsd->strings_count;
			igsd->strings_count += 1;
		}

		if (*i <= 0xff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_string_id8 TSRMLS_CC);
			od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
		} else if (*i <= 0xffff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_string_id16 TSRMLS_CC);
			od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
		} else {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_string_id32 TSRMLS_CC);
			od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
		}
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_chararray */
/** Serializes string data. */
inline static int od_igbinary_serialize_chararray(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC) {

	if (len <= 0xff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_string8 TSRMLS_CC);
		od_igbinary_serialize8(igsd, len TSRMLS_CC);
	} else if (len <= 0xffff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_string16 TSRMLS_CC);
		od_igbinary_serialize16(igsd, len TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, od_igbinary_type_string32 TSRMLS_CC);
		od_igbinary_serialize32(igsd, len TSRMLS_CC);
	}

	if (od_igbinary_serialize_resize(igsd, len +1 TSRMLS_CC)) {
		return 1;
	}

	memcpy(igsd->buffer+igsd->buffer_size, s, len);
	igsd->buffer_size += len;

	//XXX
	// odus need '\0' at the end of string for lazy reading
	igsd->buffer[igsd->buffer_size++]='\0';

	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize_chararray_ex */
/** Serializes string data, enhanced to work with static string table. */
inline static int od_igbinary_serialize_chararray_ex(od_igbinary_serialize_data *igsd, const char *s, uint32_t len TSRMLS_DC) {
	uint32_t t;
	uint32_t *i = &t;

	if (igsd->compact_strings && hash_si_find(&od_static_strings_hash, s, len, i) == 0) {
		if (*i <= 0xff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id8 TSRMLS_CC);
			od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
		} else if (*i <= 0xffff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id16 TSRMLS_CC);
			od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
		} else {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_static_string_id32 TSRMLS_CC);
			od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
		}
		return 0;
	} else {
		return od_igbinary_serialize_chararray(igsd, s, len TSRMLS_CC);
	}	
}
/* }}} */

/* {{{ od_igbinary_serialize_array_key */
/** Serializes a key of array. */
inline static int od_igbinary_serialize_array_key(od_igbinary_serialize_data *igsd, bool object, int key_type, ulong key_index, char *key, uint key_len TSRMLS_DC) {
	if (key_type==HASH_KEY_IS_LONG) {
		return od_igbinary_serialize_long(igsd, key_index TSRMLS_CC);
	} else if (object) {
		// We enable string compact only for object properties name.
		return od_igbinary_serialize_string(igsd, key, key_len TSRMLS_CC);
	} else {
		return od_igbinary_serialize_chararray_ex(igsd, key, key_len TSRMLS_CC);
	}
}
/* }}} */


/* {{{ igbinay_serialize_array */
/** Serializes array or objects inner properties. */
inline int od_igbinary_serialize_array(od_igbinary_serialize_data *igsd, zval *z, zend_class_entry* ce, bool object, bool incomplete_class, bool in_od_serialize TSRMLS_DC) {
	HashTable *h;
	HashPosition pos;
	uint32_t n;
	zval **d;

	char *key;
	uint key_len;
	int key_type;
	ulong key_index;

	int r = 0;

	/* hash */
	h = object ? Z_OBJPROP_P(z) : HASH_OF(z);

	/* hash size */
	n = h ? zend_hash_num_elements(h) : 0;

	/* incomplete class magic member */
	if (n > 0 && incomplete_class) {
		--n;
	}

	if (!object && od_igbinary_serialize_array_ref(igsd, z, object TSRMLS_CC) == 0) {
			return 1; // if we see that we serialized a ref then return error in ODUS 
	}

	//for removiing default properties
	uint num_defaults = 0;
	uint old_len_bytes;
	uint32_t old_len_pos = igsd->buffer_size;

	if (n <= 0xff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_array8 TSRMLS_CC);
		od_igbinary_serialize8(igsd, n TSRMLS_CC);
	} else if (n <= 0xffff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_array16 TSRMLS_CC);
		od_igbinary_serialize16(igsd, n TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, od_igbinary_type_array32 TSRMLS_CC);
		od_igbinary_serialize32(igsd, n TSRMLS_CC);
	}

	old_len_bytes = igsd->buffer_size - old_len_pos;

	//XXX
	// odus will save the length of array at the buffer now
	if (od_igbinary_serialize_resize(igsd, OD_IGBINARY_VALUE_LEN_SIZE TSRMLS_CC)) {
		return 1;
	}

	if (n == 0) {
		igsd->buffer_size += OD_IGBINARY_VALUE_LEN_SIZE;
		od_igbinary_serialize_value_len(igsd,0,igsd->buffer_size - OD_IGBINARY_VALUE_LEN_SIZE);
		return 0;
	}

	igsd->buffer_size += OD_IGBINARY_VALUE_LEN_SIZE;
	uint32_t value_start = igsd->buffer_size;

	/* serialize properties. */
	zend_hash_internal_pointer_reset_ex(h, &pos);
	for (;; zend_hash_move_forward_ex(h, &pos)) {
		key_type = zend_hash_get_current_key_ex(h, &key, &key_len, &key_index, 0, &pos);

		/* last */
		if (key_type == HASH_KEY_NON_EXISTANT) {
			break;
		}

		/* skip magic member in incomplete classes */
		if (incomplete_class && strcmp(key, MAGIC_MEMBER) == 0) {
			continue;
		}

		if(key_type != HASH_KEY_IS_LONG && key_type != HASH_KEY_IS_STRING) {
			od_error(E_ERROR, "od_igbinary_serialize_array: key is not string nor long");
			/* not reached */
			return 1;
		}

		/* we should still add element even if it's not OK,
		 * since we already wrote the length of the array before */
		if (zend_hash_get_current_data_ex(h, (void *) &d, &pos) != SUCCESS || d == NULL) {
			r += od_igbinary_serialize_array_key(igsd, object, key_type, key_index, key, key_len - 1 TSRMLS_CC);

			if (od_igbinary_serialize_null(igsd TSRMLS_CC)) {
				return 1;
			}
		} else {
			if(in_od_serialize) {
				r += od_igbinary_serialize_array_key(igsd, object, key_type, key_index, key, key_len - 1 TSRMLS_CC);

				normal_od_wrapper_serialize(igsd, *d, 0);
			} else {

				if(object && ODUS_G(remove_default) && !incomplete_class && ce && pos && is_default(pos->arKey,pos->nKeyLength,pos->h,*d,&ce->default_properties)) {
					num_defaults ++;
				} else {
					r += od_igbinary_serialize_array_key(igsd, object, key_type, key_index, key, key_len - 1 TSRMLS_CC);

					if (od_igbinary_serialize_zval(igsd, *d TSRMLS_CC)) {
						return 1;
					}
				}

			}
		}

	}

	uint32_t len = igsd->buffer_size - value_start;

	od_igbinary_serialize_value_len(igsd,len,value_start-OD_IGBINARY_VALUE_LEN_SIZE);

	if(num_defaults > 0) {

		uint32_t new_n = (num_defaults<=n)?(n-num_defaults):0;
		uint new_len_bytes = (new_n <= 0xff)? 2 : ((new_n <= 0xffff)? 3 : 5);

		adjust_len_info(igsd, n, new_n, old_len_bytes, new_len_bytes, old_len_pos);
	}

	return r;
}
/* }}} */

/* {{{ od_igbinary_serialize_string_table */
/** Serializes the string table. */
inline int od_igbinary_serialize_string_table(od_igbinary_serialize_data *igsd TSRMLS_DC) {
	int i = 0;
	struct hash_si *h = &igsd->strings;
	uint32_t *indexes = (int*)emalloc(sizeof(int) * igsd->strings_count);
	uint32_t string_table_start = igsd->buffer_size;

	if (!indexes) {
		return 1;
	}

	od_igbinary_serialize32_at(igsd, string_table_start, OD_IGBINARY_STRING_TABLE_START_OFFSET	TSRMLS_CC);

	// strings
	for (i = 0; i < h->size; i++) {
		if (h->data[i].key != NULL) {
			indexes[h->data[i].value] = igsd->buffer_size - string_table_start;
			od_igbinary_serialize_chararray(igsd, h->data[i].key, h->data[i].key_len TSRMLS_CC);
		}
	}

	od_igbinary_serialize32(igsd, igsd->strings_count TSRMLS_CC);

	od_igbinary_serialize32_at(igsd, igsd->buffer_size, OD_IGBINARY_STRING_TABLE_INDEX_OFFSET	TSRMLS_CC);

	// indexes
	for (i = 0; i < igsd->strings_count; i++) {
		od_igbinary_serialize32(igsd, indexes[i] TSRMLS_CC);
	}

	efree(indexes);

	return 0;
}
/* }}} */

inline int od_igbinary_clone_string_table(od_igbinary_serialize_data *igsd, od_igbinary_unserialize_data *orig TSRMLS_DC) {
	int32_t string_table_start = 0;
	int32_t index_offset = 0;
	uint32_t strings_count = 0;
	int32_t i = 0;
	char *str = NULL;
	uint32_t str_len = 0;
	uint32_t string_offset = 0;

	char *buffer_backup = orig->buffer;
	orig->buffer = orig->original_buffer;
	string_table_start = (int32_t)od_igbinary_unserialize32_at(orig, OD_IGBINARY_STRING_TABLE_START_OFFSET	TSRMLS_CC);
	index_offset = (int32_t)od_igbinary_unserialize32_at(orig, OD_IGBINARY_STRING_TABLE_INDEX_OFFSET	TSRMLS_CC);
	strings_count = od_igbinary_unserialize32_at(orig, index_offset - 4	TSRMLS_CC);

	for(i = 0; i < strings_count; i++) {
		string_offset = string_table_start + od_igbinary_unserialize32_at(orig, index_offset + 4 * i	TSRMLS_CC);
		od_igbinary_unserialize_string_from_table(orig, &str, &str_len, string_offset TSRMLS_CC);
		hash_si_insert(&igsd->strings, str, str_len, i);
	}
	igsd->strings_count = strings_count;

	orig->buffer = buffer_backup;
	return 0;
}

/* {{{ od_igbinary_serialize_update_string_table */
/** Update offset for the string table. */
inline int od_igbinary_serialize_update_string_table(od_igbinary_serialize_data *igsd, od_igbinary_unserialize_data *orig, int32_t delta TSRMLS_DC) {
	int32_t string_table_start = 0;
	int32_t index_offset = 0;
	uint32_t original_strings_count = 0;

	char *buffer_backup = orig->buffer;
	orig->buffer = orig->original_buffer;
	string_table_start = (int32_t)od_igbinary_unserialize32_at(orig, OD_IGBINARY_STRING_TABLE_START_OFFSET	TSRMLS_CC);
	index_offset = (int32_t)od_igbinary_unserialize32_at(orig, OD_IGBINARY_STRING_TABLE_INDEX_OFFSET	TSRMLS_CC);
	original_strings_count = od_igbinary_unserialize32_at(orig, index_offset - 4	TSRMLS_CC);

	if (igsd->string_table_update) {
		od_igbinary_serialize_string_table(igsd TSRMLS_CC);
	} else {
		// Only update the offset.
		od_igbinary_serialize_memcpy(igsd, orig->buffer + string_table_start, index_offset + original_strings_count * 4 - string_table_start);
		od_igbinary_serialize32_at(igsd, (uint32_t)(string_table_start + delta), OD_IGBINARY_STRING_TABLE_START_OFFSET	TSRMLS_CC);
		od_igbinary_serialize32_at(igsd, (uint32_t)(index_offset + delta), OD_IGBINARY_STRING_TABLE_INDEX_OFFSET	TSRMLS_CC);
	}

	orig->buffer = buffer_backup;
	return 0;
}
/* }}} */

/* {{{ od_igbinary_serialize_array_ref */
/** Serializes array reference. */

// Will always return 1 or trigger fatal error
inline static int od_igbinary_serialize_array_ref(od_igbinary_serialize_data *igsd, zval *z, bool object TSRMLS_DC) {
	uint32_t t = 0;
	uint32_t *i = &t;
	union {
		zval *z;
		struct {
			zend_class_entry *ce;
			zend_object_handle handle;
		} obj;
	} key = { 0 };

	if (object && Z_TYPE_P(z) == IS_OBJECT && Z_OBJ_HT_P(z)->get_class_entry) {
		key.obj.ce = Z_OBJCE_P(z);
		key.obj.handle = Z_OBJ_HANDLE_P(z);
	} else {
		key.z = z;
	}

	if ( (object || Z_ISREF_P(z)) && hash_si_find(&igsd->objects, (char *)&key, sizeof(key), i) == 1) {
		t = hash_si_size(&igsd->objects);
		hash_si_insert(&igsd->objects, (char *)&key, sizeof(key), t);
		return 1;
	} else {

		if(!object && !Z_ISREF_P(z)) {
			//XXX
			// no referred array is valid in odus
			return 1;
		}

		// Depending on what behavior you want out of the extension you can enable this 
		// in /etc/php.d/odus.ini ( set odus.throw_exceptions=1 )
		if(ODUS_G(od_throw_exceptions)) {
				zend_throw_exception(odus_exception_ce, "Odus detected reference to OBJECT/ARRAY in data which is not supported by odus serialization", -1 TSRMLS_CC);
		}

		// Turn reduce php fatals 
		// in /etc/php.d/odus.ini ( set odus.reduce_fatals=1 )
		if(!ODUS_G(od_reduce_fatals)) { // default behavor of ODUS is to have reduce fatals off
				od_error(E_ERROR, "in odus different values could not referring to same OBJECT or ARRAY");
		}

		od_igbinary_type type;
		if (*i <= 0xff) {
			type = object ? od_igbinary_type_objref8 : od_igbinary_type_ref8;
			od_igbinary_serialize8(igsd, (uint8_t) type TSRMLS_CC);
			od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
		} else if (*i <= 0xffff) {
			type = object ? od_igbinary_type_objref16 : od_igbinary_type_ref16;
			od_igbinary_serialize8(igsd, (uint8_t) type TSRMLS_CC);
			od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
		} else {
			type = object ? od_igbinary_type_objref32 : od_igbinary_type_ref32;
			od_igbinary_serialize8(igsd, (uint8_t) type TSRMLS_CC);
			od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
		}

		return 0;
	}

	return 1;
}
/* }}} */

inline static void adjust_len_info(od_igbinary_serialize_data *igsd, uint32_t n, uint32_t new_n, uint old_len_bytes, uint new_len_bytes, uint32_t old_len_pos) {

	assert(old_len_bytes >= new_len_bytes);

	debug("%u properties are defaults and removed out",n - new_n);

	uint32_t diff = old_len_bytes - new_len_bytes;

	uint32_t real_size = igsd->buffer_size - diff;

	if(diff>0) {
		uint32_t pos;

		for(pos = old_len_pos + new_len_bytes; pos < real_size; pos++) {
			igsd->buffer[pos] = igsd->buffer[pos + diff];
		}
	}

	igsd->buffer_size = old_len_pos;

	if (new_n <= 0xff) {
		igsd->buffer[igsd->buffer_size++] = (uint8_t)od_igbinary_type_array8;
		igsd->buffer[igsd->buffer_size++] = (uint8_t)new_n;

	} else if (new_n <= 0xffff) {

		igsd->buffer[igsd->buffer_size++] = (uint8_t)od_igbinary_type_array16;

		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n >> 8 & 0xff);
		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n & 0xff);

	} else {
		igsd->buffer[igsd->buffer_size++] = (uint8_t)od_igbinary_type_array32;

		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n >> 24 & 0xff);
		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n >> 16 & 0xff);
		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n >> 8 & 0xff);
		igsd->buffer[igsd->buffer_size++] = (uint8_t) (new_n & 0xff);
	}

	igsd->buffer_size = real_size;
}

/* {{{ od_igbinary_serialize_array_sleep */
/** Serializes object's properties array with __sleep -function. */
inline static int od_igbinary_serialize_array_sleep(od_igbinary_serialize_data *igsd, zval *z, HashTable *h, zend_class_entry *ce, bool incomplete_class TSRMLS_DC) {
	HashPosition pos;
	uint32_t n = zend_hash_num_elements(h);
	zval **d;
	zval **v;

	char *key;
	uint key_len;
	int key_type;
	ulong key_index;

	//for removiing default properties
	uint num_defaults = 0;
	uint old_len_bytes;
	ulong hash;
	uint32_t old_len_pos = igsd->buffer_size;

	int r = 0;

	/* Decrease array size by one, because of magic member (with class name) */
	if (n > 0 && incomplete_class) {
		--n;
	}

	/* Serialize array id. */
	if (n <= 0xff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_array8 TSRMLS_CC);
		od_igbinary_serialize8(igsd, n TSRMLS_CC);
	} else if (n <= 0xffff) {
		od_igbinary_serialize8(igsd, od_igbinary_type_array16 TSRMLS_CC);
		od_igbinary_serialize16(igsd, n TSRMLS_CC);
	} else {
		od_igbinary_serialize8(igsd, od_igbinary_type_array32 TSRMLS_CC);
		od_igbinary_serialize32(igsd, n TSRMLS_CC);
	}

	old_len_bytes = igsd->buffer_size - old_len_pos;

	//XXX
	// odus will save the length of array at the buffer now
	if (od_igbinary_serialize_resize(igsd, OD_IGBINARY_VALUE_LEN_SIZE TSRMLS_CC)) {
		return 1;
	}

	if (n == 0) {
		igsd->buffer_size += OD_IGBINARY_VALUE_LEN_SIZE;
		od_igbinary_serialize_value_len(igsd,0,igsd->buffer_size - OD_IGBINARY_VALUE_LEN_SIZE);
		return 0;
	}

	igsd->buffer_size += OD_IGBINARY_VALUE_LEN_SIZE;
	uint32_t value_start = igsd->buffer_size;

	zend_hash_internal_pointer_reset_ex(h, &pos);

	for (;; zend_hash_move_forward_ex(h, &pos)) {
		key_type = zend_hash_get_current_key_ex(h, &key, &key_len, &key_index, 0, &pos);

		/* last */
		if (key_type == HASH_KEY_NON_EXISTANT) {
			break;
		}

		/* skip magic member in incomplete classes */
		if (incomplete_class && strcmp(key, MAGIC_MEMBER) == 0) {
			continue;
		}

		if (zend_hash_get_current_data_ex(h, (void *) &d, &pos) != SUCCESS || d == NULL || Z_TYPE_PP(d) != IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "__sleep should return an array only "
					"containing the names of instance-variables to "
					"serialize");

			/* we should still add element even if it's not OK,
			 * since we already wrote the length of the array before
			 * serialize null as key-value pair */
			r += od_igbinary_serialize_null(igsd TSRMLS_CC);
		} else {
			hash = zend_get_hash_value(Z_STRVAL_PP(d), Z_STRLEN_PP(d) + 1);
			if (zend_hash_quick_find(Z_OBJPROP_P(z), Z_STRVAL_PP(d), Z_STRLEN_PP(d) + 1, hash, (void *) &v) == SUCCESS) {
				if(ODUS_G(remove_default) && !incomplete_class && ce && is_default(Z_STRVAL_PP(d),Z_STRLEN_PP(d)+1,hash,*v,&ce->default_properties)) {
					num_defaults ++;
				} else {
					//r += od_igbinary_serialize_string(igsd, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
					r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
					r += od_igbinary_serialize_zval(igsd, *v TSRMLS_CC);
				}
			} else if (ce) {
				char *prot_name = NULL;
				char *priv_name = NULL;
				int prop_name_length;

				do {
					/* try private */
					zend_mangle_property_name(&priv_name, &prop_name_length, ce->name, ce->name_length,
								Z_STRVAL_PP(d), Z_STRLEN_PP(d), ce->type & ZEND_INTERNAL_CLASS);
					hash = zend_get_hash_value(priv_name, prop_name_length+1);
					if (zend_hash_quick_find(Z_OBJPROP_P(z), priv_name, prop_name_length+1, hash, (void *) &v) == SUCCESS) {
						if(ODUS_G(remove_default) && !incomplete_class && ce && is_default(priv_name,prop_name_length+1,hash,*v,&ce->default_properties)) {
							num_defaults ++;
						} else {
							//r += od_igbinary_serialize_string(igsd, priv_name, prop_name_length TSRMLS_CC);
							r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, priv_name, prop_name_length TSRMLS_CC);
							r += od_igbinary_serialize_zval(igsd, *v TSRMLS_CC);
						}

						efree(priv_name);
						break;
					}
					efree(priv_name);

					/* try protected */
					zend_mangle_property_name(&prot_name, &prop_name_length, "*", 1,
								Z_STRVAL_PP(d), Z_STRLEN_PP(d), ce->type & ZEND_INTERNAL_CLASS);
					hash = zend_get_hash_value(prot_name, prop_name_length+1);
					if (zend_hash_quick_find(Z_OBJPROP_P(z), prot_name, prop_name_length+1, hash, (void *) &v) == SUCCESS) {
						if(ODUS_G(remove_default) && !incomplete_class && ce && is_default(prot_name,prop_name_length+1,hash,*v,&ce->default_properties)) {
							num_defaults ++;
						} else {
							//r += od_igbinary_serialize_string(igsd, prot_name, prop_name_length TSRMLS_CC);
							r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, prot_name, prop_name_length TSRMLS_CC);
							r += od_igbinary_serialize_zval(igsd, *v TSRMLS_CC);
						}

						efree(prot_name);
						break;
					}
					efree(prot_name);

					/* no win */
					//r += od_igbinary_serialize_string(igsd, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
					r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
					r += od_igbinary_serialize_null(igsd TSRMLS_CC);
					php_error_docref(NULL TSRMLS_CC, E_NOTICE, "\"%s\" returned as member variable from __sleep() but does not exist", Z_STRVAL_PP(d));
				} while (0);

			} else {
				// if all else fails, just serialize the value in anyway.
				//r += od_igbinary_serialize_string(igsd, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
				r += od_igbinary_serialize_array_key(igsd, true, HASH_KEY_IS_STRING, -1, Z_STRVAL_PP(d), Z_STRLEN_PP(d) TSRMLS_CC);
				r += od_igbinary_serialize_zval(igsd, *v TSRMLS_CC);
			}
		}
	}

	uint32_t len = igsd->buffer_size - value_start;

	od_igbinary_serialize_value_len(igsd,len,value_start-OD_IGBINARY_VALUE_LEN_SIZE);

	if(num_defaults > 0) {

		uint32_t new_n = (num_defaults<=n)?(n-num_defaults):0;
		uint new_len_bytes = (new_n <= 0xff)? 2 : ((new_n <= 0xffff)? 3 : 5);

		adjust_len_info(igsd, n, new_n, old_len_bytes, new_len_bytes, old_len_pos);
	}

	return r;
}
/* }}} */
/* {{{ od_igbinary_serialize_object_name */
/** Serialize object name. */
inline static int od_igbinary_serialize_object_name(od_igbinary_serialize_data *igsd, const char *class_name, uint32_t name_len TSRMLS_DC) {
	uint32_t t;
	uint32_t *i = &t;

	if (igsd->compact_strings) {
		if (hash_si_find(&od_static_strings_hash, class_name, name_len, i) == 0) {
			if (*i <= 0xff) {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_static_string_id8 TSRMLS_CC);
				od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
			} else if (*i <= 0xffff) {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_static_string_id16 TSRMLS_CC);
				od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
			} else {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_static_string_id32 TSRMLS_CC);
				od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
			}
			return 0;
		}

		if (hash_si_find(&igsd->strings, class_name, name_len, i) == 1) {
			hash_si_insert(&igsd->strings, class_name, name_len, igsd->strings_count);
			*i = igsd->strings_count;
			igsd->strings_count += 1;
		}

		if (*i <= 0xff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_id8 TSRMLS_CC);
			od_igbinary_serialize8(igsd, (uint8_t) *i TSRMLS_CC);
		} else if (*i <= 0xffff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_id16 TSRMLS_CC);
			od_igbinary_serialize16(igsd, (uint16_t) *i TSRMLS_CC);
		} else {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_id32 TSRMLS_CC);
			od_igbinary_serialize32(igsd, (uint32_t) *i TSRMLS_CC);
		}
	} else {
		if (name_len <= 0xff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object8 TSRMLS_CC);
			od_igbinary_serialize8(igsd, (uint8_t) name_len TSRMLS_CC);
		} else if (name_len <= 0xffff) {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object16 TSRMLS_CC);
			od_igbinary_serialize16(igsd, (uint16_t) name_len TSRMLS_CC);
		} else {
			od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object32 TSRMLS_CC);
			od_igbinary_serialize32(igsd, (uint32_t) name_len TSRMLS_CC);
		}

		//XXX
		// in odus, string will end with '\0'
		if (od_igbinary_serialize_resize(igsd, name_len + 1 TSRMLS_CC)) {
			return 1;
		}

		memcpy(igsd->buffer+igsd->buffer_size, class_name, name_len);
		igsd->buffer_size += name_len;

		igsd->buffer[igsd->buffer_size++] = '\0';
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_serialize_object */
/** Serialize object.
 * @see ext/standard/var.c
 * */
inline static int od_igbinary_serialize_object(od_igbinary_serialize_data *igsd, zval *z TSRMLS_DC) {
	zend_class_entry *ce;

	zval f;
	zval *h = NULL;

	int r = 0;

	unsigned char *serialized_data = NULL;
	zend_uint serialized_len;

	PHP_CLASS_ATTRIBUTES;

	if (od_igbinary_serialize_array_ref(igsd, z, true TSRMLS_CC) == 0) {
		return 1; // if we see that we serialized a ref then return error in ODUS 
	}

	ce = Z_OBJCE_P(z);

#if ALLOW_CLASS_SERIALIZE
	/* custom serializer */
	if (ce && ce->serialize != NULL) {
		//XXX
		// odus doesn't allow class has its serialize method
		od_error(E_ERROR,"class '%s' could not have its serialize method in odus", ce->name);

		/* TODO: var_hash? */
		if(ce->serialize(z, &serialized_data, &serialized_len, (zend_serialize_data *)NULL TSRMLS_CC) == SUCCESS && !EG(exception)) {
			od_igbinary_serialize_object_name(igsd, ce->name, ce->name_length TSRMLS_CC);

			if (serialized_len <= 0xff) {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_ser8 TSRMLS_CC);
				od_igbinary_serialize8(igsd, (uint8_t) serialized_len TSRMLS_CC);
			} else if (serialized_len <= 0xffff) {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_ser16 TSRMLS_CC);
				od_igbinary_serialize16(igsd, (uint16_t) serialized_len TSRMLS_CC);
			} else {
				od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_object_ser32 TSRMLS_CC);
				od_igbinary_serialize32(igsd, (uint32_t) serialized_len TSRMLS_CC);
			}

			if (od_igbinary_serialize_resize(igsd, serialized_len TSRMLS_CC)) {
				if (serialized_data) {
					efree(serialized_data);
				}
				r = 1;

				return r;
			}

			memcpy(igsd->buffer+igsd->buffer_size, serialized_data, serialized_len);
			igsd->buffer_size += serialized_len;
		} else if (EG(exception)) {
			/* exception, return failure */
			r = 1;
		} else {
			/* Serialization callback failed, assume null output */
			od_igbinary_serialize_null(igsd TSRMLS_CC);
		}

		if (serialized_data) {
			efree(serialized_data);
		}

		return r;
	}

#endif

	/* serialize class name */
	PHP_SET_CLASS_ATTRIBUTES(z);
	od_igbinary_serialize_object_name(igsd, class_name, name_len TSRMLS_CC);
	PHP_CLEANUP_CLASS_ATTRIBUTES();

	if (ce && ce != PHP_IC_ENTRY && zend_hash_exists(&ce->function_table, "__sleep", sizeof("__sleep"))) {
		/* function name string */
		INIT_PZVAL(&f);
		ZVAL_STRINGL(&f, "__sleep", sizeof("__sleep") - 1, 0);

		/* calling z->__sleep */
		r = call_user_function_ex(CG(function_table), &z, &f, &h, 0, 0, 1, NULL TSRMLS_CC);

		if (r == SUCCESS && !EG(exception)) {
			r = 0;

			if (h) {
				if (Z_TYPE_P(h) == IS_ARRAY) {
					r = od_igbinary_serialize_array_sleep(igsd, z, HASH_OF(h), ce, incomplete_class TSRMLS_CC);
				} else {
					od_error(E_ERROR, "__sleep should return an array only "
							"containing the names of instance-variables to "
							"serialize");

					/* empty array */
					od_igbinary_serialize8(igsd, od_igbinary_type_array8 TSRMLS_CC);
					r = od_igbinary_serialize8(igsd, 0 TSRMLS_CC);
				}
			}
		} else {
			r = 1;
		}

		/* cleanup */
		if (h) {
			zval_ptr_dtor(&h);
		}

		return r;
	} else {
		return od_igbinary_serialize_array(igsd, z, ce, true, incomplete_class, false TSRMLS_CC);
	}
}
/* }}} */
/* {{{ od_igbinary_serialize_zval */
/** Serialize zval. */
int od_igbinary_serialize_zval(od_igbinary_serialize_data *igsd, zval *z TSRMLS_DC) {
	if (Z_ISREF_P(z)) {
			// Depending on what behavior you want out of the extension you can enable this 
			// in /etc/php.d/odus.ini ( set odus.throw_exceptions=1 )
			if(ODUS_G(od_throw_exceptions)) {
					zend_throw_exception(odus_exception_ce, "Odus detected reference to VALUE which is not supported by odus serialization", -1 TSRMLS_CC);
					return 1; // return error
			}
			
			// This actually should be taken and put under another setting
			// but since od_reduce_fatals is used to return NULL on error
			// in the ..array_ref's paths continuing with that here.
			//
			// in /etc/php.d/odus.ini ( set odus.reduce_fatals=1 )
			if(ODUS_G(od_reduce_fatals)) { // default behavor of ODUS is to have reduce fatals off
					return 1; // return error
			}

			// DEFAULT behavior of ODUS 1.0.9 and before is to silently convert 
			// the Reference  into a copy of the value (this is what is deployed 
			// to citytc and the ville) so changing this behavior may affect those 
			// games, because of that I'm just leaving the default behavior as is.  
			// It is preferrable not to do though.
			//
			// Demo of the 1.0.9 behavior
			// $o = new stdClass;
			// $o->n = "only_one_string";
			// $o->r = &$o->n;
			//
			// $x = serialize($o);
			// echo "$x\n";
			// $y = od_serialize($o);
			// $z = new ODWrapper($y);
			// $j = serialize($z);
			// echo "$j\n";
			//
			// [schow@city-dev-12 tmp]$ php refref.php
			// O:8:"stdClass":2:{s:1:"n";s:15:"only_one_string";s:1:"r";R:2;}
			// O:8:"stdClass":2:{s:1:"n";s:15:"only_one_string";s:1:"r";s:15:"only_one_string";}
			// 
			// We can see that $z and $j has 2 independent copies of the "only_one_string".
			//
			// However, this is NOT desirable:
			//
			// BEST [Best option]: change ODUS to support references to VALUES (so $x == $j)
			// OK [Ok option]: don't support references but enable reduce_fatals=1 and throw_exceptions=1
			// UNDESIRABLE [Default option]: silently alter data during serialization (citytc,theville)
			//
			// od_igbinary_serialize8(igsd, (uint8_t) od_igbinary_type_ref TSRMLS_CC);
	}

	switch (Z_TYPE_P(z)) {
		case IS_RESOURCE:
			return od_igbinary_serialize_null(igsd TSRMLS_CC);
		case IS_OBJECT:
			return od_igbinary_serialize_object(igsd, z TSRMLS_CC);
		case IS_ARRAY:
			return od_igbinary_serialize_array(igsd, z, NULL, false, false, false TSRMLS_CC);
		case IS_STRING:
			// We enable string compact for class name and property name only,
			// so call serialize_chararray for all literal strings.
			return od_igbinary_serialize_chararray_ex(igsd, Z_STRVAL_P(z), Z_STRLEN_P(z) TSRMLS_CC);
		case IS_LONG:
			return od_igbinary_serialize_long(igsd, Z_LVAL_P(z) TSRMLS_CC);
		case IS_NULL:
			return od_igbinary_serialize_null(igsd TSRMLS_CC);
		case IS_BOOL:
			return od_igbinary_serialize_bool(igsd, Z_LVAL_P(z) ? 1 : 0 TSRMLS_CC);
		case IS_DOUBLE:
			return od_igbinary_serialize_double(igsd, Z_DVAL_P(z) TSRMLS_CC);
		default:
			od_error(E_ERROR, "od_igbinary_serialize_zval: zval has unknown type %d", (int)Z_TYPE_P(z));
			/* not reached */
			return 1;
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_unserialize_data_init */
/** Inits od_igbinary_unserialize_data_init. */
inline int od_igbinary_unserialize_data_init(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	//smart_str empty_str = { 0 };
	uint32_t format_version = (uint32_t)ODUS_G(format_version);

	igsd->buffer = NULL;
	igsd->buffer_size = 0;
	igsd->buffer_offset = 0;

	igsd->strings = NULL;
	igsd->strings_count = 0;
	//igsd->strings_capacity = 4;

	if (format_version == OD_IGBINARY_FORMAT_VERSION_02) {
		igsd->compact_strings = true;
		igsd->compress_value_len = true;
	} else if (format_version == OD_IGBINARY_FORMAT_VERSION_01) {
		igsd->compact_strings = false;
		igsd->compress_value_len = false;
	} else {
		// Default, enable both.
		debug("od_igbinary_serialize_data_init: wrong format_version configured.");
		igsd->compact_strings = true;
		igsd->compress_value_len = true;
	}

	igsd->root_id = 0;

	//odus doesn't allow object references
	//igsd->references = NULL;
	//igsd->references_count = 0;
	//igsd->references_capacity = 4;

	//odus doesn't allow object references
	//igsd->references = (void **) emalloc(sizeof(void *) * igsd->references_capacity);
	//if (igsd->references == NULL) {
	//	return 1;
	//}

	// igsd->strings = (struct od_igbinary_unserialize_string_pair *) emalloc(sizeof(struct od_igbinary_unserialize_string_pair) * igsd->strings_capacity);
	// if (igsd->strings == NULL) {
	// 	//efree(igsd->references);
	// 	return 1;
	// }

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_data_init */
/** Inits od_igbinary_unserialize_data_init. */
inline int od_igbinary_unserialize_data_clone(od_igbinary_unserialize_data *dest, od_igbinary_unserialize_data *src TSRMLS_DC) {
	if (!dest || !src) {
		debug("null pointer!\n");
		return 1;
	}

	dest->buffer = src->buffer;
	dest->buffer_size = src->buffer_size;
	dest->buffer_offset = src->buffer_offset;
	dest->strings = src->strings;
	dest->strings_count = src->strings_count;
	dest->original_buffer = src->original_buffer;
	dest->string_table_start = src->string_table_start;
	dest->strings_index_offset = src->strings_index_offset;
	dest->compact_strings = src->compact_strings;
	dest->compress_value_len = src->compress_value_len;
	dest->root_id = src->root_id;
	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_data_deinit */
/** Deinits od_igbinary_unserialize_data_init. */
inline void od_igbinary_unserialize_data_deinit(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	if (igsd->strings) {
		efree(igsd->strings);
	}

	return;
}
/* }}} */
/* {{{ od_igbinary_unserialize_header */
/** Unserialize header. Check for version. */
inline int od_igbinary_unserialize_header(od_igbinary_unserialize_data *igsd, uint32_t *header TSRMLS_DC) {
	if (igsd->buffer_offset + 4 >= igsd->buffer_size) {
		return 1;
	}

	*header = od_igbinary_unserialize32(igsd TSRMLS_CC);

	/* Support older version 1 and the current format 2 */
	if (*header == (OD_IGBINARY_FORMAT_FLAG | OD_IGBINARY_FORMAT_VERSION_01)) {
		igsd->compact_strings = false;
		igsd->compress_value_len = false;
		return 0;
	} else if (*header == (OD_IGBINARY_FORMAT_FLAG | OD_IGBINARY_FORMAT_VERSION_02)) {
		igsd->compact_strings = true;
		igsd->compress_value_len = true;
		return 0;
	} else {
		od_error(E_ERROR, "od_igbinary_unserialize_header: unsupported version: 0x%x, should be 0x%x", (unsigned int) *header,(unsigned int) OD_IGBINARY_FORMAT_HEADER);
		return 1;
	}
}
/* }}} */

/* {{{ od_igbinary_unserialize_init_string_table */
/** Init string table. */
inline int od_igbinary_unserialize_init_string_table(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	if (igsd->buffer_offset + 8 >= igsd->buffer_size) {
		return 1;
	}

	igsd->string_table_start = od_igbinary_unserialize32(igsd TSRMLS_CC);

	igsd->strings_index_offset = od_igbinary_unserialize32(igsd TSRMLS_CC);

	igsd->strings_count = od_igbinary_unserialize32_at(igsd, igsd->strings_index_offset - sizeof(uint32_t) TSRMLS_CC);

	igsd->strings = (struct od_igbinary_unserialize_string_pair *) emalloc(sizeof(struct od_igbinary_unserialize_string_pair) * igsd->strings_count);

	if (igsd->strings == NULL) {
		od_error(E_ERROR, "Fail to alloc memory for string table");
		return 1;
	}

	// For lazy loading.
	memset(igsd->strings, 0, sizeof(struct od_igbinary_unserialize_string_pair) * igsd->strings_count);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize8_at */
/** Unserialize 8bit value at specified position. */
inline uint8_t od_igbinary_unserialize8_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC) {
	return igsd->buffer[pos];
}
/* }}} */

/* {{{ od_igbinary_unserialize8 */
/** Unserialize 8bit value. */
inline uint8_t od_igbinary_unserialize8(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	return igsd->buffer[igsd->buffer_offset++];
}
/* }}} */

/* {{{ od_igbinary_unserialize16 */
/** Unserialize 16bit value at specified position. */
inline uint16_t od_igbinary_unserialize16_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC) {
	uint16_t ret = 0;
	ret |= ((uint16_t) igsd->buffer[pos++] << 8);
	ret |= ((uint16_t) igsd->buffer[pos++] << 0);
	return ret;
}
/* }}} */

/* {{{ od_igbinary_unserialize16 */
/** Unserialize 16bit value. */
inline uint16_t od_igbinary_unserialize16(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	uint16_t ret = 0;
	ret |= ((uint16_t) igsd->buffer[igsd->buffer_offset++] << 8);
	ret |= ((uint16_t) igsd->buffer[igsd->buffer_offset++] << 0);
	return ret;
}
/* }}} */

/* {{{ od_igbinary_unserialize32 */
/** Unserialize 32bit value at specified position. */
inline uint32_t od_igbinary_unserialize32_at(od_igbinary_unserialize_data *igsd, uint32_t pos TSRMLS_DC) {
	uint32_t ret = 0;
	ret |= ((uint32_t) igsd->buffer[pos++] << 24);
	ret |= ((uint32_t) igsd->buffer[pos++] << 16);
	ret |= ((uint32_t) igsd->buffer[pos++] << 8);
	ret |= ((uint32_t) igsd->buffer[pos++] << 0);
	return ret;
}
/* }}} */

/* {{{ od_igbinary_unserialize32 */
/** Unserialize 32bit value. */
inline uint32_t od_igbinary_unserialize32(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	uint32_t ret = 0;
	ret |= ((uint32_t) igsd->buffer[igsd->buffer_offset++] << 24);
	ret |= ((uint32_t) igsd->buffer[igsd->buffer_offset++] << 16);
	ret |= ((uint32_t) igsd->buffer[igsd->buffer_offset++] << 8);
	ret |= ((uint32_t) igsd->buffer[igsd->buffer_offset++] << 0);
	return ret;
}
/* }}} */

/* {{{ od_igbinary_unserialize64 */
/** Unserialize 64bit value. */
inline uint64_t od_igbinary_unserialize64(od_igbinary_unserialize_data *igsd TSRMLS_DC) {
	uint64_t ret = 0;
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 56);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 48);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 40);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 32);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 24);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 16);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 8);
	ret |= ((uint64_t) igsd->buffer[igsd->buffer_offset++] << 0);
	return ret;
}
/* }}} */
/* {{{ od_igbinary_unserialize_long */
/** Unserializes long */
inline int od_igbinary_unserialize_long(od_igbinary_unserialize_data *igsd, od_igbinary_type t, long *ret TSRMLS_DC) {


	uint32_t tmp32;
#if SIZEOF_LONG == 8
	uint64_t tmp64;
#endif

	if (t == od_igbinary_type_long8p || t == od_igbinary_type_long8n) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: end-of-data");
			return 1;
		}

		*ret = (long) (t == od_igbinary_type_long8n ? -1 : 1) * od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_long16p || t == od_igbinary_type_long16n) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: end-of-data");
			return 1;
		}

		*ret = (long) (t == od_igbinary_type_long16n ? -1 : 1) * od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_long32p || t == od_igbinary_type_long32n) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: end-of-data");
			return 1;
		}

		/* check for boundaries */
		tmp32 = od_igbinary_unserialize32(igsd TSRMLS_CC);
#if SIZEOF_LONG == 4
		if (tmp32 > 0x80000000 || (tmp32 == 0x80000000 && t == od_igbinary_type_long32p)) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: 64bit long on 32bit platform?");
			tmp32 = 0; /* t == od_igbinary_type_long32p ? LONG_MAX : LONG_MIN; */
		}
#endif
		*ret = (long) (t == od_igbinary_type_long32n ? -1 : 1) * tmp32;
	} else if (t == od_igbinary_type_long64p || t == od_igbinary_type_long64n) {
#if SIZEOF_LONG == 8
		if (igsd->buffer_offset + 8 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: end-of-data");
			return 1;
		}

		/* check for boundaries */
		tmp64 = od_igbinary_unserialize64(igsd TSRMLS_CC);
		if (tmp64 > 0x8000000000000000 || (tmp64 == 0x8000000000000000 && t == od_igbinary_type_long64p)) {
			od_error(E_ERROR, "od_igbinary_unserialize_long: too big 64bit long.");
			tmp64 = 0; /* t == od_igbinary_type_long64p ? LONG_MAX : LONG_MIN */;
		}

		*ret = (long) (t == od_igbinary_type_long64n ? -1 : 1) * tmp64;
#elif SIZEOF_LONG == 4
		/* can't put 64bit long into 32bit one, placeholder zero */
		*ret = 0;
		od_igbinary_unserialize64(igsd TSRMLS_CC);
		od_error(E_ERROR, "od_igbinary_unserialize_long: 64bit long on 32bit platform");
#else
#error "Strange sizeof(long)."
#endif
	} else {
		*ret = 0;
		od_error(E_ERROR, "od_igbinary_unserialize_long: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_unserialize_double */
/** Unserializes double. */
inline int od_igbinary_unserialize_double(od_igbinary_unserialize_data *igsd, od_igbinary_type t, double *ret TSRMLS_DC) {


	union {
		double d;
		uint64_t u;
	} u;

	(void) t;

	if (igsd->buffer_offset + 8 > igsd->buffer_size) {
		od_error(E_ERROR, "od_igbinary_unserialize_double: end-of-data");
		return 1;
	}


	u.u = od_igbinary_unserialize64(igsd TSRMLS_CC);

	*ret = u.d;

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_static_string */
/** Unserializes static string. */
inline int od_igbinary_unserialize_static_string(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC) {
	size_t i;

	if (t == od_igbinary_type_static_string_id8 || t == od_igbinary_type_object_static_string_id8) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_static_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_static_string_id16 || t == od_igbinary_type_object_static_string_id16) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_static_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_static_string_id32 || t == od_igbinary_type_object_static_string_id32) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_static_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		zend_error(E_WARNING, "od_igbinary_unserialize_static_string: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	if (i >= od_static_strings_count) {
		zend_error(E_WARNING, "od_igbinary_unserialize_static_string: string index is out-of-bounds, i %d, strings_count %d", i, od_static_strings_count);
		return 1;
	}

	*s = od_static_strings[i];
	*len = strlen(od_static_strings[i]);

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_string */
/** Unserializes string. Unserializes both actual string or by string id. */
inline static int od_igbinary_unserialize_string(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC) {
	size_t i;

	if (t == od_igbinary_type_string_id8 || t == od_igbinary_type_object_id8) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_string_id16 || t == od_igbinary_type_object_id16) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_string_id32 || t == od_igbinary_type_object_id32) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			zend_error(E_WARNING, "od_igbinary_unserialize_string: end-of-data");
			return 1;
		}
		i = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		zend_error(E_WARNING, "od_igbinary_unserialize_string: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	if (i >= igsd->strings_count) {
		zend_error(E_WARNING, "od_igbinary_unserialize_string: string index is out-of-bounds, i %d, strings_count %d", i, igsd->strings_count);
		return 1;
	}

	// Lazy loading the string.
	if (!igsd->strings[i].data) {
		// unserializing functions are tied to igsd->buffer. Tune it here to make them happy.
		uint8_t *buffer_backup = igsd->buffer;
		igsd->buffer = igsd->original_buffer;

		uint32_t index_offset = igsd->strings_index_offset + i * sizeof(uint32_t);
		uint32_t string_offset = od_igbinary_unserialize32_at(igsd, index_offset TSRMLS_CC) + igsd->string_table_start;
		od_igbinary_unserialize_string_from_table(igsd, &igsd->strings[i].data, &igsd->strings[i].len, string_offset TSRMLS_CC);

		// Restore current buffer.
		igsd->buffer = buffer_backup;

		if (!igsd->strings[i].data) {
			return 1;
		}
	}

	*s = igsd->strings[i].data;
	*len = igsd->strings[i].len;

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_chararray */
/** Unserializes chararray of string. */
inline int od_igbinary_unserialize_chararray(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **s, uint32_t *len TSRMLS_DC) {
	uint32_t l;

	//XXX
	// in odus string ends with '\0'
	if (t == od_igbinary_type_string8 || t == od_igbinary_type_object8) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
			return 1;
		}
		l = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_string16 || t == od_igbinary_type_object16) {
		if (igsd->buffer_offset + 3 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
			return 1;
		}
		l = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_string32 || t == od_igbinary_type_object32) {
		if (igsd->buffer_offset + 5 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_unserialize_chararray: end-of-data");
			return 1;
		}
		l = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		od_error(E_ERROR, "od_igbinary_unserialize_chararray: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	*s = (char *) (igsd->buffer + igsd->buffer_offset);
	*len = l;

	//XXX
	// in odus string ends with '\0'
	igsd->buffer_offset += l+1;

	return 0;
}
/* }}} */

/* {{{ od_igbinary_unserialize_class_name */
/** Unserializes class name. */
inline int od_igbinary_unserialize_class_name(od_igbinary_unserialize_data *igsd, od_igbinary_type t, char **name, uint32_t *name_len TSRMLS_DC) {
	if (t == od_igbinary_type_object8 || t == od_igbinary_type_object16 || t == od_igbinary_type_object32) {
		return od_igbinary_unserialize_chararray(igsd, t, name, name_len TSRMLS_CC);
	} else if (t == od_igbinary_type_object_static_string_id8 || t == od_igbinary_type_object_static_string_id16 || t == od_igbinary_type_object_static_string_id32) {
		return od_igbinary_unserialize_static_string(igsd, t, name, name_len TSRMLS_CC);
	} else if (t == od_igbinary_type_object_id8 || t == od_igbinary_type_object_id16 || t == od_igbinary_type_object_id32) {
		return od_igbinary_unserialize_string(igsd, t, name, name_len TSRMLS_CC);
	}
	else {
		od_error(E_ERROR, "od_igbinary_unserialize_class_name: unknown object type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}
}
/* }}} */


/* {{{ od_igbinary_unserialize_string_from_table */
/** Unserializes chararray of string from the string table. */
inline int od_igbinary_unserialize_string_from_table(od_igbinary_unserialize_data *igsd, char **s, uint32_t *len, uint32_t pos TSRMLS_DC) {
	uint32_t l;
	uint32_t offset = pos;

	od_igbinary_type t = (od_igbinary_type) od_igbinary_unserialize8_at(igsd, offset TSRMLS_CC);

	offset += 1;
	if (t == od_igbinary_type_string8 || t == od_igbinary_type_object8) {
		// Ignore the check, because buffer_size may be small for an inside object. Just make sure
		// the buffer start and offset is correct on calling.

		// if (offset + 2 > igsd->buffer_size) {
		// 	od_error(E_ERROR, "od_igbinary_unserialize_string_from_table: end-of-data");
		// 	return 1;
		// }
		l = od_igbinary_unserialize8_at(igsd, offset TSRMLS_CC);
		offset += 1;
	} else if (t == od_igbinary_type_string16 || t == od_igbinary_type_object16) {
		// if (offset + 3 > igsd->buffer_size) {
		// 	od_error(E_ERROR, "od_igbinary_unserialize_string_from_table: end-of-data");
		// 	return 1;
		// }
		l = od_igbinary_unserialize16_at(igsd, offset TSRMLS_CC);
		offset += 2;
	} else if (t == od_igbinary_type_string32 || t == od_igbinary_type_object32) {
		// if (offset + 5 > igsd->buffer_size) {
		// 	od_error(E_ERROR, "od_igbinary_unserialize_string_from_table: end-of-data");
		// 	return 1;
		// }
		l = od_igbinary_unserialize32_at(igsd, offset TSRMLS_CC);
		offset += 4;
	} else {
		od_error(E_ERROR, "od_igbinary_unserialize_string_from_table: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	*s = (char *) (igsd->buffer + offset);
	*len = l;

	return 0;
}
/* }}} */

inline od_igbinary_type od_igbinary_get_type(od_igbinary_unserialize_data *igsd) {
	if (igsd->buffer_offset + 1 > igsd->buffer_size) {
		debug("wrong in %s:%d",__FUNCTION__,__LINE__);
		od_error(E_ERROR, "od_igbinary_get_type: end-of-data");
		return od_igbinary_type_null;
	}

	return (od_igbinary_type) od_igbinary_unserialize8(igsd TSRMLS_CC);
}

inline od_igbinary_type od_igbinary_get_type_at(od_igbinary_unserialize_data *igsd, uint32_t pos) {
	if (pos + 1 > igsd->buffer_size) {
		debug("wrong in %s:%d",__FUNCTION__,__LINE__);
		od_error(E_ERROR, "od_igbinary_get_type_at: end-of-data");
		return od_igbinary_type_null;
	}

	return (od_igbinary_type) od_igbinary_unserialize8_at(igsd, pos TSRMLS_CC);
}

inline uint32_t od_igbinary_get_member_num(od_igbinary_unserialize_data *igsd, od_igbinary_type t) {

	uint32_t n;

	if (t == od_igbinary_type_array8) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_member_num: end-of-data");
			return -1;
		}
		n = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_array16) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_member_num: end-of-data");
			return -1;
		}
		n = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_array32) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_member_num: end-of-data");
			return -1;
		}
		n = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		od_error(E_ERROR, "od_igbinary_get_member_num: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return -1;
	}

	// n cannot be larger than the number of minimum "objects" in the array
	if (n > igsd->buffer_size - igsd->buffer_offset) {
		od_error(E_ERROR, "%s: data size %zu smaller that requested array length %zu.", "od_igbinary_get_member_num", igsd->buffer_size - igsd->buffer_offset, n);
		return -1;
	}

	return n;
}

inline uint32_t od_igbinary_get_value_len(od_igbinary_unserialize_data *igsd) {

	if (igsd->compress_value_len) {
		uint8_t encode_len_probe = 0;

		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_value_len: end-of-data");
			return -1;
		}
		encode_len_probe = od_igbinary_unserialize8_at(igsd, igsd->buffer_offset);

		if ((encode_len_probe & 0xc0) == 0x40) {
			igsd->buffer_offset += 1;

			return encode_len_probe & 0x3f;
		} else if ((encode_len_probe & 0xc0) == 0x80) {
			if (igsd->buffer_offset + 2 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_get_value_len: end-of-data");
				return -1;
			}

			uint16_t value_len = od_igbinary_unserialize16(igsd);
			return value_len & 0x3fff;
		} else {
			if (igsd->buffer_offset + 4 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_get_value_len: end-of-data");
				return -1;
			}

			return (uint32_t)od_igbinary_unserialize32(igsd);
		}
	} else {
		if (igsd->buffer_offset + OD_IGBINARY_VALUE_LEN_SIZE > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_get_value_len: end-of-data");
			return -1;
		}

		return (uint32_t)od_igbinary_unserialize32(igsd);
	}
}

inline int od_igbinary_skip_value_len(od_igbinary_unserialize_data *igsd) {
	if (igsd->compress_value_len) {
		uint8_t encode_len_probe = 0;

		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_skip_value_len: end-of-data");
			return -1;
		}
		encode_len_probe = od_igbinary_unserialize8_at(igsd, igsd->buffer_offset);

		if ((encode_len_probe & 0xc0) == 0x40) {
			igsd->buffer_offset += 1;
			return 0;
		} else if ((encode_len_probe & 0xc0) == 0x80) {
			if (igsd->buffer_offset + 2 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_skip_value_len: end-of-data");
				return -1;
			}

			igsd->buffer_offset += 2;
			return 0;
		} else {
			if (igsd->buffer_offset + 4 > igsd->buffer_size) {
				od_error(E_ERROR, "od_igbinary_skip_value_len: end-of-data");
				return -1;
			}

			igsd->buffer_offset += 4;
			return 0;
		}
	} else {

		if (igsd->buffer_offset + OD_IGBINARY_VALUE_LEN_SIZE > igsd->buffer_size) {
			od_error(E_ERROR, "od_igbinary_skip_value_len: end-of-data");
			return 1;
		}

		igsd->buffer_offset += OD_IGBINARY_VALUE_LEN_SIZE;

		return 0;
	}
}

inline int od_igbinary_unserialize_skip_key(od_igbinary_unserialize_data *igsd) {
	char* key;
	uint32_t key_len;
	long key_index;

	return od_igbinary_unserialize_get_key(igsd,&key,&key_len,&key_index);
}

inline int od_igbinary_unserialize_get_key(od_igbinary_unserialize_data *igsd, char** key_p, uint32_t* key_len_p, long* key_index_p) {


	*key_p = NULL;

	od_igbinary_type key_type = od_igbinary_get_type(igsd TSRMLS_CC);

	switch (key_type) {
		case od_igbinary_type_string_id8:
		case od_igbinary_type_string_id16:
		case od_igbinary_type_string_id32:
			if (od_igbinary_unserialize_string(igsd, key_type, key_p, key_len_p TSRMLS_CC)) {
				return 1;
			}
			break;

		case od_igbinary_type_string8:
		case od_igbinary_type_string16:
		case od_igbinary_type_string32:
			if (od_igbinary_unserialize_chararray(igsd, key_type, key_p, key_len_p TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_static_string_id8:
		case od_igbinary_type_static_string_id16:
		case od_igbinary_type_static_string_id32:
			if (od_igbinary_unserialize_static_string(igsd, key_type, key_p, key_len_p TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_long8p:
		case od_igbinary_type_long8n:
		case od_igbinary_type_long16p:
		case od_igbinary_type_long16n:
		case od_igbinary_type_long32p:
		case od_igbinary_type_long32n:
		case od_igbinary_type_long64p:
		case od_igbinary_type_long64n:
			if (od_igbinary_unserialize_long(igsd, key_type, key_index_p TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_string_empty:
		case od_igbinary_type_null:
			*key_p = EMPTY_STRING;
			*key_len_p = 0;
			*key_index_p = 0;
			break;
		default:
			od_error(E_ERROR, "od_igbinary_unserialize_get_key: unknown key type '%02x', position %zu", key_type, igsd->buffer_offset);
			return 1;
	}

	return 0;
}

/* {{{ od_igbinary_unserialize_array */
/** Unserializes array. */
inline static int od_igbinary_unserialize_array(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z, int object TSRMLS_DC) {


	uint32_t i;

	zval *v = NULL;
	/*	zval *old_v; */

	char *key;
	uint32_t key_len = 0;
	long key_index = 0;

	od_igbinary_type key_type;

	HashTable *h;

	uint32_t n = od_igbinary_get_member_num(igsd,t);

	if(n<0) return 1;

	if (!object) {
		Z_TYPE_PP(z) = IS_ARRAY;
		ALLOC_HASHTABLE(Z_ARRVAL_PP(z));
		zend_hash_init(Z_ARRVAL_PP(z), n + 1, NULL, ZVAL_PTR_DTOR, 0);
	}

	if(od_igbinary_skip_value_len(igsd)) {
		zval_dtor(*z);
		ZVAL_NULL(*z);
		return 1;
	}

	/* empty array */
	if (n == 0) {
		return 0;
	}

	h = HASH_OF(*z);

	for (i = 0; i < n; i++) {
		if(od_igbinary_unserialize_get_key(igsd,&key,&key_len,&key_index)){
			zval_dtor(*z);
			ZVAL_NULL(*z);
			return 1;
		}

		ALLOC_INIT_ZVAL(v);
		if (od_igbinary_unserialize_zval(igsd, &v TSRMLS_CC)) {
			zval_dtor(*z);
			ZVAL_NULL(*z);
			zval_ptr_dtor(&v);
			return 1;
		}

		if (key) {
			zend_symtable_update(h, key, key_len + 1, &v, sizeof(v), NULL);
		} else {
			zend_hash_index_update(h, key_index, &v, sizeof(v), NULL);
		}
	}

	return 0;
}
/* }}} */
/* {{{ od_igbinary_unserialize_object_ser */
/** Unserializes object's property array of objects implementing Serializable -interface. */
inline static int od_igbinary_unserialize_object_ser(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z, zend_class_entry *ce TSRMLS_DC) {
	uint32_t n;

	if (ce->unserialize == NULL) {
		od_error(E_ERROR, "Class %s has no unserializer", ce->name);
		return 1;
	}

	if (t == od_igbinary_type_object_ser8) {
		if (igsd->buffer_offset + 1 > igsd->buffer_size) {
			debug("wrong in %s:%d",__FUNCTION__,__LINE__);od_error(E_ERROR, "od_igbinary_unserialize_object_ser: end-of-data");
			return 1;
		}
		n = od_igbinary_unserialize8(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_object_ser16) {
		if (igsd->buffer_offset + 2 > igsd->buffer_size) {
			debug("wrong in %s:%d",__FUNCTION__,__LINE__);od_error(E_ERROR, "od_igbinary_unserialize_object_ser: end-of-data");
			return 1;
		}
		n = od_igbinary_unserialize16(igsd TSRMLS_CC);
	} else if (t == od_igbinary_type_object_ser32) {
		if (igsd->buffer_offset + 4 > igsd->buffer_size) {
			debug("wrong in %s:%d",__FUNCTION__,__LINE__);od_error(E_ERROR, "od_igbinary_unserialize_object_ser: end-of-data");
			return 1;
		}
		n = od_igbinary_unserialize32(igsd TSRMLS_CC);
	} else {
		od_error(E_ERROR, "od_igbinary_unserialize_object_ser: unknown type '%02x', position %zu", t, igsd->buffer_offset);
		return 1;
	}

	if (igsd->buffer_offset + n > igsd->buffer_size) {
		debug("wrong in %s:%d",__FUNCTION__,__LINE__);od_error(E_ERROR, "od_igbinary_unserialize_object_ser: end-of-data");
		return 1;
	}

	if (ce->unserialize(z, ce, (const unsigned char*)(igsd->buffer + igsd->buffer_offset), n, NULL TSRMLS_CC) != SUCCESS) {
		return 1;
	} else if (EG(exception)) {
		return 1;
	}

	igsd->buffer_offset += n;

	return 0;
}
/* }}} */
/* {{{ od_igbinary_unserialize_object */
/** Unserialize object.
 * @see ext/standard/var_unserializer.c
 */
inline static int od_igbinary_unserialize_object(od_igbinary_unserialize_data *igsd, od_igbinary_type t, zval **z TSRMLS_DC) {


	zend_class_entry *ce;
	zend_class_entry **pce;

	zval *h = NULL;
	zval f;

	char *name = NULL;
	uint32_t name_len = 0;

	int r;

	bool incomplete_class = false;

	zval *user_func;
	zval *retval_ptr;
	zval **args[1];
	zval *arg_func_name;

	if (od_igbinary_unserialize_class_name(igsd, t, &name, &name_len TSRMLS_CC)) {
		return 1;
	}

	do {
		/* Try to find class directly */
		if (zend_lookup_class(name, name_len, &pce TSRMLS_CC) == SUCCESS) {
			ce = *pce;
			break;
		}

		//XXX -- xma
		//in zend_lookup_class will do the autoload things

		/* Check for unserialize callback */
		if ((PG(unserialize_callback_func) == NULL) || (PG(unserialize_callback_func)[0] == '\0')) {
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
			break;
		}

		/* Call unserialize callback */
		MAKE_STD_ZVAL(user_func);
		ZVAL_STRING(user_func, PG(unserialize_callback_func), 1);
		args[0] = &arg_func_name;
		MAKE_STD_ZVAL(arg_func_name);
		ZVAL_STRING(arg_func_name, name, 1);
		if (call_user_function_ex(CG(function_table), NULL, user_func, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) != SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "defined (%s) but not found", name);
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
			zval_ptr_dtor(&user_func);
			zval_ptr_dtor(&arg_func_name);
			break;
		}
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}

		/* The callback function may have defined the class */
		if (zend_lookup_class(name, name_len, &pce TSRMLS_CC) == SUCCESS) {
			ce = *pce;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Function %s() hasn't defined the class it was called for", name);
			incomplete_class = true;
			ce = PHP_IC_ENTRY;
		}

		zval_ptr_dtor(&user_func);
		zval_ptr_dtor(&arg_func_name);
	} while (0);

	/* previous user function call may have raised an exception */
	if (EG(exception)) {
		return 1;
	}

	object_init_ex(*z, ce);

	//odus doesn't allow object references
	/* reference */
	//if (igsd->references_count + 1 >= igsd->references_capacity) {
	//	while (igsd->references_count + 1 >= igsd->references_capacity) {
	//		igsd->references_capacity *= 2;
	//	}

	//	igsd->references = (void **) erealloc(igsd->references, sizeof(void *) * igsd->references_capacity);
	//	if (igsd->references == NULL)
	//		return 1;
	//}

	//igsd->references[igsd->references_count++] = (void *) *z;

	/* store incomplete class name */
	if (incomplete_class) {
		//XXX
		// odus doesn't allow incomplete class
		od_error(E_ERROR,"could not find define for class '%s'",name);
		php_store_class_name(*z, name, name_len);
	}

	t = (od_igbinary_type) od_igbinary_unserialize8(igsd TSRMLS_CC);

	switch (t) {
		case od_igbinary_type_array8:
		case od_igbinary_type_array16:
		case od_igbinary_type_array32:
			r = od_igbinary_unserialize_array(igsd, t, z, 1 TSRMLS_CC);
			break;
			/*
			 * odus doesn't allow class has its serialization method
		case od_igbinary_type_object_ser8:
		case od_igbinary_type_object_ser16:
		case od_igbinary_type_object_ser32:
			r = od_igbinary_unserialize_object_ser(igsd, t, z, ce TSRMLS_CC);
			break;
			*/
		default:
			od_error(E_ERROR, "od_igbinary_unserialize_object: unknown object inner type '%02x', position %zu", t, igsd->buffer_offset);
			return 1;
	}

	if (r) {
		return r;
	}

	if (Z_OBJCE_PP(z) != PHP_IC_ENTRY && zend_hash_exists(&Z_OBJCE_PP(z)->function_table, "__wakeup", sizeof("__wakeup"))) {
		INIT_PZVAL(&f);
		ZVAL_STRINGL(&f, "__wakeup", sizeof("__wakeup") - 1, 0);
		call_user_function_ex(CG(function_table), z, &f, &h, 0, 0, 1, NULL TSRMLS_CC);

		if (h) {
			zval_ptr_dtor(&h);
		}

		if (EG(exception)) {
			r = 1;
		}
	}

	return r;
}
/* }}} */

/* {{{ od_igbinary_unserialize_zval */
/** Unserialize zval. */
int od_igbinary_unserialize_zval(od_igbinary_unserialize_data *igsd, zval **z TSRMLS_DC) {
	od_igbinary_type t;


	long tmp_long;
	double tmp_double;
	char *tmp_chararray;
	uint32_t tmp_uint32_t;

	if (igsd->buffer_offset + 1 > igsd->buffer_size) {
		debug("wrong in %s:%d",__FUNCTION__,__LINE__);
		od_error(E_ERROR, "od_igbinary_unserialize_zval: end-of-data");
		return 1;
	}

	t = (od_igbinary_type) od_igbinary_unserialize8(igsd TSRMLS_CC);

	switch (t) {
		case od_igbinary_type_object8:
		case od_igbinary_type_object16:
		case od_igbinary_type_object32:
		case od_igbinary_type_object_id8:
		case od_igbinary_type_object_id16:
		case od_igbinary_type_object_id32:
		case od_igbinary_type_object_static_string_id8:
		case od_igbinary_type_object_static_string_id16:
		case od_igbinary_type_object_static_string_id32:
			if (od_igbinary_unserialize_object(igsd, t, z TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_array8:
		case od_igbinary_type_array16:
		case od_igbinary_type_array32:
			if (od_igbinary_unserialize_array(igsd, t, z, 0 TSRMLS_CC)) {
				return 1;
			}
			break;
		case od_igbinary_type_string8:
		case od_igbinary_type_string16:
		case od_igbinary_type_string32:
			if (od_igbinary_unserialize_chararray(igsd, t, &tmp_chararray, &tmp_uint32_t TSRMLS_CC)) {
				return 1;
			}
			ZVAL_STRINGL(*z, tmp_chararray, tmp_uint32_t, 1);
			break;
		case od_igbinary_type_string_empty:
			ZVAL_EMPTY_STRING(*z);
			break;
		case od_igbinary_type_string_id8:
		case od_igbinary_type_string_id16:
		case od_igbinary_type_string_id32:
			if (od_igbinary_unserialize_string(igsd, t, &tmp_chararray, &tmp_uint32_t TSRMLS_CC)) {
				return 1;
			}
			ZVAL_STRINGL(*z, tmp_chararray, tmp_uint32_t, 1);
			break;
		case od_igbinary_type_static_string_id8:
		case od_igbinary_type_static_string_id16:
		case od_igbinary_type_static_string_id32:
			if (od_igbinary_unserialize_static_string(igsd, t, &tmp_chararray, &tmp_uint32_t TSRMLS_CC)) {
				return 1;
			}
			ZVAL_STRINGL(*z, tmp_chararray, tmp_uint32_t, 1);
			break;
		case od_igbinary_type_long8p:
		case od_igbinary_type_long8n:
		case od_igbinary_type_long16p:
		case od_igbinary_type_long16n:
		case od_igbinary_type_long32p:
		case od_igbinary_type_long32n:
		case od_igbinary_type_long64p:
		case od_igbinary_type_long64n:
			if (od_igbinary_unserialize_long(igsd, t, &tmp_long TSRMLS_CC)) {
				return 1;
			}
			ZVAL_LONG(*z, tmp_long);
			break;
		case od_igbinary_type_null:
			ZVAL_NULL(*z);
			break;
		case od_igbinary_type_bool_false:
			ZVAL_BOOL(*z, 0);
			break;
		case od_igbinary_type_bool_true:
			ZVAL_BOOL(*z, 1);
			break;
		case od_igbinary_type_double:
			if (od_igbinary_unserialize_double(igsd, t, &tmp_double TSRMLS_CC)) {
				return 1;
			}
			ZVAL_DOUBLE(*z, tmp_double);
			break;
		default:
			od_error(E_ERROR, "od_igbinary_unserialize_zval: unknown type '%02x', position %zu", t, igsd->buffer_offset);
			return 1;
	}

	return 0;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 2
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
