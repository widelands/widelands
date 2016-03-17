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
                     icon = "images/wui/stats/menu_tab_wares.png",
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
                     name = "ware",
                     title = _"Ware",
                     icon = "images/wui/stats/menu_tab_wares.png",
                     script = "tribes/scripting/help/ware_help.lua",
                     script_parameters = {"log"}
                  }
               }
            },
            {
               name = "workers",
               title = _"Workers",
               icon = "images/wui/buildings/menu_tab_workers.png",
               entries = {
                  {
                     name = "worker",
                     title = _"Worker",
                     icon = "images/wui/stats/menu_tab_workers.png",
                     script = "tribes/scripting/help/worker_help.lua",
                     script_parameters =  {"barbarians", "carrier"}
                  }
               }
            },
            {
               name = "buildings",
               title = _"Buildings",
               icon = "images/wui/stats/genstats_nrbuildings.png",
               entries = {
                  {
                     name = "building",
                     title = _"Building",
                     icon = "images/wui/stats/genstats_nrbuildings.png",
                     script = "tribes/scripting/help/building_help.lua",
                     script_parameters = {"barbarians", "ax_workshop"}
                  }
               }
            },
         }
      }
   end
}
