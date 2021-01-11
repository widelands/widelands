push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "atlanteans_tower_high",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "High Tower"),
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 21,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 78 },
      }
   },

   aihints = {},

   max_soldiers = 5,
   heal_per_second = 170,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your high tower.",
      aggressor = _"Your high tower discovered an aggressor.",
      attack = _"Your high tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the high tower.",
      defeated_you = _"Your soldiers defeated the enemy at the high tower."
   },
}

pop_textdomain()
