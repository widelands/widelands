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
      granite = 2
   },
   return_on_dismantle = {
      log = 2,
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
      supports_production_of = { "fish" }
   },

   working_positions = {
      europeans_claydigger = 1
      europeans_fishbreeder = 1,
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
            "call=fish_breeding_in_pond",
            "call=fish_breeding_in_sea",
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
            "callworker=breed_in_pond",
            "sleep=duration:12s500ms"
         }
      },
      fish_breeding_in_sea = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
         descname = _"breeding fish",
         actions = {
            "return=skipped unless economy needs fish",
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
