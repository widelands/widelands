dirname = path.dirname(__file__)

local fs_button = dirname .. "fsmenu/button.png"
local wui_button = dirname .. "wui/button.png"

local fs_blue =  {0, 24, 40}
local fs_green =  {10, 50, 0}
local fs_brown =  {45, 26, 18}

local wui_light =  {85, 63, 35}
local wui_green =  {3, 15, 0}
local wui_brown =  {32, 20, 10}

return {
   buttons = {
      fsmenu = {
         menu = { image = fs_button, color = fs_blue },
         primary = { image = fs_button, color = fs_green },
         secondary = { image = fs_button, color = fs_brown },
      },
      wui = {
         menu = { image = wui_button, color = wui_light },
         primary = { image = wui_button, color = wui_green },
         secondary = { image = wui_button, color = wui_brown },
         -- Building buttons on fieldaction and building stats need to be transparent
         building_stats = { image = "", color = {0, 0, 0} },
      }
   },
   sliders = {
      fsmenu = {
         menu = { image = fs_button, color = fs_blue },
      },
      wui = {
         light = { image = wui_button, color = wui_brown },
         dark = { image = wui_button, color = wui_green },
      }
   },
   tabpanels = {
      fsmenu = {
         menu = { image = "", color = {5, 5, 5} },
      },
      wui = {
         light = { image = "", color = {0, 0, 0} },
         dark = { image = wui_button, color = wui_brown },
      }
   },
   editboxes = {
      fsmenu = {
         menu = { image = fs_button, color = fs_green },
      },
      wui = {
         menu = { image = wui_button, color = wui_brown },
      }
   },
   dropdowns = {
      fsmenu = {
         menu = { image = fs_button, color = fs_brown },
      },
      wui = {
         menu = { image = wui_button, color = wui_brown },
      }
   },
   scrollbars = {
      fsmenu = {
         menu = { image = fs_button, color = fs_blue },
      },
      wui = {
         menu = { image = wui_button, color = wui_brown },
      }
   }
}
