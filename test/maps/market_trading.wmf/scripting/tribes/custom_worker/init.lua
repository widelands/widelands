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
         "playsound=sound/woodcutting/woodcutting priority:100%",
         "animate=hacking duration:15s",
         "playsound=sound/woodcutting/tree_falling priority:50%",
         "callobject=fall",
         "animate=idle duration:2s",
         "createware=custom_ware",
         "script=barbarians_custom_worker_program_counter_increment",
         "return"
      },
      remove_pinned_note = {
         "findobject=type:special name:pinned_note radius:5",
         "walk=object",
         "animate=hacking duration:15s",
         "removeobject",
         "animate=idle duration:2s",
         "return"
      }
   },
}

function barbarians_custom_worker_program_counter_increment(worker)
   assert_equal("barbarians_custom_worker", worker.descr.name)
   barbarians_custom_worker_program_counter = (barbarians_custom_worker_program_counter or 0) + 1
end
