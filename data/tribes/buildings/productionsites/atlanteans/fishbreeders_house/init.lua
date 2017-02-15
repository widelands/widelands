dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_fishbreeders_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Fish Breeder’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },
   return_on_dismantle = {
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 34, 42 },
      },
   },

   aihints = {
      needs_water = true,
      renews_map_resource = "fish",
      prohibited_till = 700,
      forced_after = 900
   },

   working_positions = {
      atlanteans_fishbreeder = 1
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
         descname = _"breeding fish",
         actions = {
            "sleep=24000",
            "worker=breed"
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fish",
      heading = _"Out of Fish",
      message = pgettext("atlanteans_building", "The fish breeder working out of this fish breeder’s house can’t find any fish in his work area. Remember that you can only regenerate your fish resources if there are some fish left to breed."),
      productivity_threshold = 60
   },
   -- Translators: Productivity tooltip for Atlantean fish breeders when all water fields are already full of fish
   resource_not_needed_message = _"The fishing grounds are full";
}
