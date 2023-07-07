local dirname = "campaigns/emp04.wmf/" .. path.dirname (__file__)

push_textdomain("scenario_emp04.wmf")

wl.Descriptions():new_productionsite_type {
   name = "empire_brewery1",
   descname = pgettext("empire_building", "Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "empire_brewery",
      enhancement_cost = {
         planks = 1,
         granite = 1
      },
      enhancement_return_on_dismantle = {}
   },

   buildcost = {
      log = 1,
      planks = 2,
      granite = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 42, 66 },
      },
      working = {
         basename = "idle", -- TODO(hessenfarmer): No animation yet.
         hotspot = { 42, 66 },
      },
   },

   aihints = {
   },

   working_positions = {
      empire_brewer = 1
   },

   inputs = {
      { name = "water", amount = 3 },
      { name = "wheat", amount = 7 }
   },

   programs = {
      main = {
         descname = pgettext("empire_building", "brewing beer"),
         actions = {
            "sleep=duration:30s",
            "return=skipped unless economy needs beer",
            "consume=water:3 wheat",
            "playsound=sound/empire/beerbubble priority:40% allow_multiple",
            "animate=working duration:30s",
            "produce=beer"
         }
      },
   },
}
pop_textdomain()
