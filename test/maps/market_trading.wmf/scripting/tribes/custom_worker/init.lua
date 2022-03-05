dirname = "test/maps/market_trading.wmf/" .. path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_custom_worker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Custom Worker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      felling_ax = 1
   },

   animations = {
      idle = {
         hotspot = { 5, 23 }
      },
      hacking = {
         hotspot = { 19, 17 }
      },
      walk = {
         hotspot = { 10, 22 },
         directional = true
      },
      walkload = {
         hotspot = { 10, 21 },
         directional = true
      },
   },

   programs = {
      harvest = {
         "findobject=attrib:tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking duration:15s",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=idle duration:2s",
         "createware=custom_ware",
         "return"
      }
   },
}
