push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   name = "amazons_patrol_post",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Patrol Post"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      granite = 1,
      log = 1,
      rope = 1
   },
   return_on_dismantle = {
      granite = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 46}},
      unoccupied = {hotspot = {39, 46}}
   },

   aihints = {
      expansion = true,
      fighting = true,
   },

   max_soldiers = 2,
   heal_per_second = 100,
   conquers = 6,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your patrol post.",
      aggressor = _"Your patrol post discovered an aggressor.",
      attack = _"Your patrol post is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the patrol post.",
      defeated_you = _"Your soldiers defeated the enemy at the patrol post."
   },
}
pop_textdomain()
