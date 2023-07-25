push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_militarysite_type {
   name = "amazons_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Tower"),
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 16,

   enhancement = {
      name = "amazons_observation_tower",
      enhancement_cost = {
         ironwood = 1,
         rope = 1,
         balsa = 2
      },
      enhancement_return_on_dismantle = {
         ironwood = 1,
         balsa = 1
      }
   },

   buildcost = {
      rope = 2,
      granite = 2,
      log = 2,
      ironwood = 2
   },
   return_on_dismantle = {
      rope = 1,
      granite = 1,
      log = 1,
      ironwood = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {50, 81}},
      unoccupied = {hotspot = {50, 81}},
   },

   aihints = {
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 500
   },

   max_soldiers = 4,
   heal_per_second = 110,
   conquers = 8,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Amazon military site
      occupied = pgettext("amazons_building", "Your soldiers have occupied your tower."),
      -- TRANSLATORS: Message sent by an Amazon military site
      aggressor = pgettext("amazons_building", "Your tower discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Amazon military site
      attack = pgettext("amazons_building", "Your tower is under attack."),
      -- TRANSLATORS: Message sent by an Amazon military site
      defeated_enemy = pgettext("amazons_building", "The enemy defeated your soldiers at the tower."),
      -- TRANSLATORS: Message sent by an Amazon military site
      defeated_you = pgettext("amazons_building", "Your soldiers defeated the enemy at the tower.")
   },
}

pop_textdomain()
