push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_militarysite_type {
   name = "amazons_warriors_dwelling",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Warriors Dwelling"),
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 11,

   buildcost = {
      ironwood = 2,
      granite = 3,
   },
   return_on_dismantle = {
      ironwood = 1,
      granite = 2,

   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {45, 59}},
      unoccupied = {hotspot = {45, 59}},
   },

   aihints = {
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 500
   },

   max_soldiers = 5,
   heal_per_second = 150,
   conquers = 8,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your warriors delling.",
      aggressor = _"Your warriors dwelling discovered an aggressor.",
      attack = _"Your warriors dwelling is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the warriors dwelling.",
      defeated_you = _"Your soldiers defeated the enemy at the warriors dwelling."
   },
}

pop_textdomain()
