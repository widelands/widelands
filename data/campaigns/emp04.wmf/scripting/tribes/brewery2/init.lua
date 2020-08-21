dirname = "tribes/buildings/productionsites/empire/brewery/"

push_textdomain("scenario_emp04.wmf")

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_brewery2",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Brewery"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      planks = 1,
      granite = 1
   },
   return_on_dismantle_on_enhanced = {
   },

   animations = {
      idle = {
         hotspot = { 39, 62 },
      },
      working = {
         basename = "idle",
         hotspot = { 39, 62 },
      },
   },

   aihints = {
      prohibited_till = 790,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_brewer = 1
   },

   inputs = {
      { name = "water", amount = 7 },
      { name = "wheat", amount = 7 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _"brewing beer",
         actions = {
            "sleep=duration:30s",
            "return=skipped unless economy needs beer",
            "consume=water wheat",
            "playsound=sound/empire/beerbubble priority:40% allow_multiple",
            "animate=working duration:30s",
            "produce=beer"
         }
      },
   },
}
pop_textdomain()
