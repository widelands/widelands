push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_fishbreeders_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Fish Breeder’s House"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 40, 50 },
      },
   },

   aihints = {
      needs_water = true,
      prohibited_till = 560,
      forced_after = 890
   },

   working_positions = {
      atlanteans_fishbreeder = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
         descname = _("breeding fish"),
         actions = {
            "callworker=breed",
            "sleep=duration:13s500ms"
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fish"),
      heading = _("Out of Fish"),
      message = pgettext("atlanteans_building", "The fish breeder working out of this fish breeder’s house can’t find any water where he can breed fish in his work area. Remember that you can only regenerate your fish resources if there ever were some fish in the vicinity in the first place."),
      productivity_threshold = 60
   },
   -- Translators: Productivity tooltip for Atlantean fish breeders when all water fields are already full of fish
   resource_not_needed_message = _("The fishing grounds are full");
}

pop_textdomain()
