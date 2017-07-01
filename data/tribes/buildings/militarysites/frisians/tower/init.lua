dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "frisians_building",
   name = "frisians_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   vision_range = 21,

   buildcost = {
      brick = 2,
      granite = 8,
      log = 6,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 1,
      granite = 4,
      log = 3,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 107, 118 },
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1200
   },

   max_soldiers = 7,
   heal_per_second = 300,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your warriors have occupied your tower.",
      aggressor = _"Your tower discovered an aggressor.",
      attack = _"Your tower is under attack.",
      defeated_enemy = _"The enemy defeated your warriors at the tower.",
      defeated_you = _"Your warriors defeated the enemy at the tower."
   },
}
