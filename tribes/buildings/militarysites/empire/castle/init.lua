dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "empire_building",
   name = "empire_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Castle"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement_cost = {
      planks = 5,
      marble_column = 4,
      marble = 4,
      granite = 2
   },
   return_on_dismantle_on_enhanced = {
      planks = 2,
      marble_column = 2,
      marble = 3,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 94, 106 }
      }
   },

   aihints = {},

   max_soldiers = 12,
   heal_per_second = 220,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your castle.",
      aggressor = _"Your castle discovered an aggressor.",
      attack = _"Your castle is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the castle.",
      defeated_you = _"Your soldiers defeated the enemy at the castle."
   },
}
