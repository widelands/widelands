dirname = "tribes/buildings/productionsites/empire/well/"

push_textdomain("scenario_emp04.wmf")

descriptions:new_productionsite_type {
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 43, 43 },
      },
      working = {
         basename = "idle", -- TODO(hessenfarmer): No animation yet.
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
         descname = pgettext("empire_building", "working"),
         actions = {
            "sleep=duration:30s",
            "animate=working duration:30s",
            "mine=resource_water radius:5 yield:100% when_empty:5%",
            "produce=water"
         }
      },
   },
   out_of_resource_notification = {
      title = pgettext("empire_building", "No Water"),
      heading = pgettext("empire_building", "Out of Water"),
      message = pgettext("empire_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 15
   },
}
pop_textdomain()
