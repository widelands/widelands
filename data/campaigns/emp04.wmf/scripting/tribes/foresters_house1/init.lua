dirname = "campaigns/emp04.wmf/" .. path.dirname (__file__)

push_textdomain("scenario_emp04.wmf")

wl.Descriptions():new_productionsite_type {
   name = "empire_foresters_house1",
   descname = pgettext("empire_building", "Forester’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },

   return_on_dismantle = {
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 54, 56 },
      },
   },

   aihints = {
   },

   working_positions = {
      empire_forester = 1
   },

   programs = {
      main = {
         descname = pgettext("empire_building", "planting trees"),
         actions = {
            "sleep=duration:1m6s",
            "callworker=plant"
         }
      },
   },
}
pop_textdomain()
