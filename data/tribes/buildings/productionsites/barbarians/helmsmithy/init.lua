push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_helmsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Helm Smithy"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      blackwood = 1,
      granite = 2,
      grout = 2,
      reed = 3
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      grout = 1,
      reed = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 65, 83 },
      },
      unoccupied = {
         hotspot = { 65, 83 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 65, 83 }
      },
      working = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 65, 83 }
      },
   },

   aihints = {
      prohibited_till = 1400
   },

   working_positions = {
      barbarians_helmsmith = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_helmet",
            "call=produce_helmet_mask",
            "call=produce_helmet_warhelm",
         }
      },
      produce_helmet = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _("forging a helmet"),
         actions = {
            -- time total: 67 + 3.6
            "return=skipped unless economy needs helmet",
            "consume=coal iron",
            "sleep=duration:32s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=helmet"
         }
      },
      produce_helmet_mask = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a mask because ...
         descname = _("forging a mask"),
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs helmet_mask",
            "consume=coal iron:2",
            "sleep=duration:32s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:45s",
            "produce=helmet_mask"
         }
      },
      produce_helmet_warhelm = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a warhelm because ...
         descname = _("forging a warhelm"),
         actions = {
            -- time total: 87 + 3.6
            "return=skipped unless economy needs helmet_warhelm",
            "consume=coal gold iron:2",
            "sleep=duration:32s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:55s",
            "produce=helmet_warhelm"
         }
      },
   },
}

pop_textdomain()
