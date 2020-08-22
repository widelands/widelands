dirname = "tribes/buildings/productionsites/empire/well/"

push_textdomain("scenario_emp04.wmf")

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_well1",
   descname = pgettext("empire_building", "Well"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1,
      marble = 1
   },
   return_on_dismantle = {
      log = 1,
      marble = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 43, 43 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 43, 43 },
      },
   },

   aihints = {

   },

   working_positions = {
      empire_carrier = 1
   },

   programs = {
      main = {
         descname = "working",
         actions = {
            "sleep=30000",
            "animate=working duration:30s",
            "mine=water 5 100 5 2",
            "produce=water"
         }
      },
   },
   out_of_resource_notification = {
      title = "No Water",
      heading = "Out of Water",
      message = "The carrier working at this well can’t find any water in his well.",
      productivity_threshold = 15
   },
}
pop_textdomain()
