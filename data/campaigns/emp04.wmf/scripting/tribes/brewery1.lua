dirname = "tribes/buildings/productionsites/empire/brewery/"

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_brewery1",
   descname = "Brewery",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   return_on_dismantle = {
      planks = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 39, 62 },
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
   outputs = {
      "beer"
   },

   programs = {
      work = {
         descname = "brewing beer",
         actions = {
            "sleep=30000",
            "return=skipped unless economy needs beer",
            "consume=water:3 wheat",
            "play_sound=sound/empire beerbubble 180",
            "animate=working 30000",
            "produce=beer"
         }
      },
   },
}
