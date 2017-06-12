-- This script defines a GUI style for Widelands. At the moment, we only
-- support the default template.
-- So far, only background textures and colors can be defined, and they all have
-- the format { image = filename, color = {r, g, b } }.

dirname = path.dirname(__file__)

-- Some common elements for reuse
local fs_button = dirname .. "fsmenu/button.png"
local wui_button = dirname .. "wui/button.png"

local fs_blue =  {0, 24, 40}
local fs_green =  {10, 50, 0}
local fs_brown =  {45, 26, 18}

local wui_light =  {85, 63, 35}
local wui_green =  {3, 15, 0}
local wui_brown =  {32, 20, 10}

-- These are the style definitions to be returned.
-- Note: you have to keep all the keys intact, or Widelands will not be happy.
return {
   -- Button backgrounds
   buttons = {
      -- Buttons used in Fullscreen menus
      fsmenu = {
         -- Main menu ("Single Player", "Watch Replay", ...)
         menu = { image = fs_button, color = fs_blue },
         -- Primary user selection ("OK", ...)
         primary = { image = fs_button, color = fs_green },
         -- Secondary user selection ("Cancel", "Delete", selection buttons, ...)
         secondary = { image = fs_button, color = fs_brown },
      },
      -- Buttons used in-game and in the editor
      wui = {
         -- Main menu ("Exit Game"), Building Windows, selection buttons, ...
         menu = { image = wui_button, color = wui_light },
         -- Primary user selection ("OK", attack, ...)
         primary = { image = wui_button, color = wui_green },
         -- Secondary user selection ("Cancel", "Delete", ...)
         secondary = { image = wui_button, color = wui_brown },
         -- Building buttons on fieldaction and building statistics need to be
         -- transparent in order to match the background of the tab panel.
         building_stats = { image = "", color = {0, 0, 0} },
      }
   },
   -- Slider cursors (Sound control, attack, statistics, ...)
   sliders = {
      fsmenu = {
         menu = { image = fs_button, color = fs_blue },
      },
      wui = {
         -- Sound Options, Statistics
         light = { image = wui_button, color = wui_brown },
         -- Fieldaction (attack)
         dark = { image = wui_button, color = wui_green },
      }
   },
   -- Background for tab panels
   tabpanels = {
      fsmenu = {
         -- Options, About, ... this comes with a hard-coded border too
         menu = { image = "", color = {5, 5, 5} },
      },
      wui = {
         -- Most in-game and in-editor tabs. Building windows, Editor tools,
         -- Encyclopedia, ...
         light = { image = "", color = {0, 0, 0} },
         -- Building buttons in Fieldaction and Building Statistics need a dark
         -- background, otherwise the icons will be hard to see.
         dark = { image = wui_button, color = wui_brown },
      }
   },
   -- Used both for one-line and multiline edit boxes
   editboxes = {
      fsmenu = {
         menu = { image = fs_button, color = fs_green },
      },
      wui = {
         menu = { image = wui_button, color = wui_brown },
      }
   },
   -- Background for dropdown menus
   dropdowns = {
      fsmenu = {
         menu = { image = fs_button, color = fs_brown },
      },
      wui = {
         menu = { image = wui_button, color = wui_brown },
      }
   },
   -- Scrollbar buttons, table headers etc.
   scrollbars = {
      fsmenu = {
         menu = { image = fs_button, color = fs_blue },
      },
      wui = {
         menu = { image = wui_button, color = wui_brown },
      }
   }
}
