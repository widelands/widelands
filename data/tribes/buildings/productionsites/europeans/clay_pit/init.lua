push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_clay_pit",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Clay Pit and Fish Breeder’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      planks = 2,
      reed = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 34, 42 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 34, 42 },
      },
   },

   aihints = {
      needs_water = true,
      supports_production_of = { "fish" }
   },

   working_positions = {
      europeans_fishbreeder = 2
   },

   inputs = {
      { name = "water", amount = 4 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=making_clay",
            "callworker=check_sea",
            "call=fish_breeding_in_sea",
            "callworker=check_pond",
            "call=fish_breeding_in_pond",
         }
      },
      making_clay = {
         -- TRANSLATORS: Completed/Skipped/Did not start making clay because ...
         descname = _"making clay",
         actions = {
            "return=skipped unless economy needs clay or economy needs fish",
            "return=failed unless site has water",
            "callworker=dig",
            "consume=water",
            "sleep=duration:20s",
            "animate=working duration:15s",
            "sleep=duration:1s",
            "produce=clay"
         },
      },
      fish_breeding_in_pond = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
         descname = _"breeding fish",
         actions = {
            "return=skipped unless economy needs fish",
            "return=skipped when economy needs clay and site has water",
            "consume=water",
            "callworker=breed_in_pond",
            "sleep=duration:12s500ms"
         }
      },
      fish_breeding_in_sea = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
         descname = _"breeding fish",
         actions = {
            "return=skipped when economy needs clay and site has water",
            "callworker=breed_in_sea",
            "sleep=duration:12s500ms"
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fish",
      heading = _"Out of Fish",
      message = pgettext("europeans_building", "The fish breeder working out of this fish breeder’s house can’t find any fish in his work area. Remember that you can only regenerate your fish resources if there are some fish left to breed."),
      productivity_threshold = 60
   },
   -- Translators: Productivity tooltip for Europeans fish breeders when all water fields are already full of fish
   resource_not_needed_message = _"The fishing grounds are full";
}

pop_textdomain()
