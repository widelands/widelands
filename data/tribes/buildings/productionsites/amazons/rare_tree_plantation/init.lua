push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "amazons_rare_tree_plantation",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Rare Tree Plantation"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 6,
      granite = 3,
      rope = 3,
   },
   return_on_dismantle = {
      log = 2,
      granite = 2,
      rope = 1,
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {49, 50},
         fps = 4,
         frames = 4,
         columns = 2,
         rows = 2
      }
   },
   animations = {unoccupied = {hotspot = {48, 50}}},

   aihints = {
      supports_production_of = { "balsa", "rubber", },
      requires_supporters = true,
      space_consumer = true,
   },

   working_positions = {
      amazons_jungle_master = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_balsa",
            "call=plant_rubber",
            "call=plant_ironwood",
         },
      },
      plant_balsa = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"planting balsatree",
         actions = {
            -- time total: xx
            "return=skipped unless economy needs balsa",
            "callworker=plant_balsa",
            "sleep=duration:13s"
         },
      },
      plant_rubber = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"planting rubbertree",
         actions = {
            -- time total: xx
            "return=skipped unless economy needs rubber",
            "callworker=plant_rubber",
            "sleep=duration:13s"
         },
      },
      plant_ironwood = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"planting ironwoodtree",
         actions = {
            -- time total: xx
            "return=skipped unless economy needs ironwood",
            "callworker=plant_ironwood",
            "sleep=duration:13s"
         },
      },
      dummy = {
         -- dirty hack to make the relation to cutter inverse due to the cutter being an upgraded building
         -- TODO(hessenfarmer): make this dissapear in building help
         descname = "dummy",
         actions = {
            "produce=ironwood",
         },
      },
   }
}

pop_textdomain()
