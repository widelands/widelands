push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_militarysite_type {
   name = "amazons_observation_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Observation Tower"),
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 19,

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {50, 94}},
      unoccupied = {hotspot = {50, 94}},
   },

   aihints = {
      expansion = true,
   },

   max_soldiers = 5,
   heal_per_second = 150,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Amazon military site
      occupied = pgettext("amazons_building", "Your soldiers have occupied your observation tower."),
      -- TRANSLATORS: Message sent by an Amazon military site
      aggressor = pgettext("amazons_building", "Your observation tower discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Amazon military site
      attack = pgettext("amazons_building", "Your observation tower is under attack."),
      -- TRANSLATORS: Message sent by an Amazon military site
      defeated_enemy = pgettext("amazons_building", "The enemy defeated your soldiers at the observation tower."),
      -- TRANSLATORS: Message sent by an Amazon military site
      defeated_you = pgettext("amazons_building", "Your soldiers defeated the enemy at the observation tower.")
   },
}

pop_textdomain()
