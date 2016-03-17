-- NOCOM include "tribes/scripting/help/format_help.lua"
-- NOCOM document

return {
   func = function(tribename)
      set_textdomain("tribes_encyclopedia")
      return {
         title = _"Tribal Encyclopedia",
         tabs = {
            {
               name = "general",
               title = _"General",
               entries = {
                  {
                     name = "intro",
                     title = _"Intro",
                     -- NOCOM icon = "images/wui/buildings/menu_tab_wares.png",
                     script = "txts/README.lua"
                  },
                  {
                     name = "abacadabra",
                     title = _"Abacadabra",
                     icon = "images/wui/buildings/menu_tab_wares.png",
                     script = "txts/README.lua"
                  }
               }
            },
            {
               name = "wares",
               title = _"Wares",
               icon = "images/wui/buildings/menu_tab_wares.png",
               entries = {
                  {
                     name = "log",
                     title = _"Log",
                     icon = "images/wui/buildings/menu_tab_wares.png",
                     script = "tribes/scripting/help/ware_help.lua",
                     script_parameters = {[1] = "barbarians", [2] = "log"}
                  }
               }
            },
            {
               name = "workers",
               title = _"Workers",
               icon = "images/wui/buildings/menu_tab_workers.png",
               entries = {
                  {
                     name = "barbarians_carrier",
                     title = _"Carrier",
                     icon = "images/wui/buildings/menu_tab_workers.png",
                     script = "tribes/scripting/help/worker_help.lua",
                     script_parameters = {[1] = "barbarians", [2] = "barbarians_carrier"}
                  }
               }
            },
            {
               name = "buildings",
               title = _"Buildings",
               icon = "images/wui/stats/genstats_nrbuildings.png",
               entries = {
                  {
                     name = "barbarians_ax_workshop",
                     title = _"Ax Workshop",
                     icon = "images/wui/stats/genstats_nrbuildings.png",
                     script = "tribes/scripting/help/building_help.lua",
                     script_parameters = {[1] = "barbarians", [2] = "barbarians_ax_workshop"}
                  }
               }
            },
         }
      }
   end
}
