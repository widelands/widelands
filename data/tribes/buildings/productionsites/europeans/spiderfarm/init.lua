push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_spiderfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Spider Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 4,
      reed = 4,
      spidercloth = 2,
      brick = 2,
      grout = 2
   },
   return_on_dismantle = {
      log = 4,
      spidercloth = 2,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 87, 75 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 87, 75 },
      }
   },

   aihints = {
      prohibited_till = 1800
   },

   working_positions = {
      europeans_breeder_normal = 1
   },

   inputs = {
      { name = "water", amount = 6 },
      { name = "corn", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_spidersilk_basic",
            "call=produce_spidersilk"
         }
      },
      produce_spidersilk_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"produce spider silk",
         actions = {
            "consume=corn water",
            "sleep=duration:60s",
            "animate=working duration:60s",
            "produce=spider_silk"
         }
      },
      produce_spidersilk = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"produce spider silk",
         actions = {
            "return=skipped unless economy needs spider_silk",
            "return=skipped when economy needs water",
            "return=skipped when economy needs corn",
            "consume=corn:2 water:2",
            "sleep=duration:60s",
            "animate=working duration:60s",
            "produce=spider_silk:2"
         }
      },
   },
}

pop_textdomain()
