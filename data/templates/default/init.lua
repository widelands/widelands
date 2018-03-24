-- This script defines a GUI style for Widelands. At the moment, we only
-- support the default template.
-- So far, only background textures and colors can be defined, and they all have
-- the format { image = filename, color = {r, g, b } }.

dirname = path.dirname(__file__)

-- Some common elements for reuse
local fs_button = dirname .. "fsmenu/button.png"
local wui_button = dirname .. "wui/button.png"

local fs_blue =  {0, 31, 40}
local fs_green =  {10, 50, 0}
local fs_brown =  {45, 34, 18}

local wui_light =  {85, 63, 35}
local wui_green =  {3, 15, 0}
local wui_brown =  {32, 19, 8}

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
   },

   -- Font sizes and colors
   fonts = {
      sizes = {
         title = 22,    -- Big titles
         normal = 14,   -- Default UI color
         message = 12,  -- Message font size
         slider = 11,   -- Slider font size
         minimum = 6,   -- When autoresizing text to fit, don't go below this size
      },
      colors = {
         foreground = {255, 255, 0}, -- Main UI color
         disabled = {127, 127, 127}, -- Disabled interactive UI elements
         warning = {255, 22, 22},    -- For highlighting warnings
         tooltip = {255, 255, 0},    -- Tooltips
         progresswindow_text = { 128, 128, 255 },    -- FS Progress bar text
         progresswindow_background = { 64, 64, 0 },  -- FS Progress bar background
         progress_bright = {255, 250, 170},          -- Progress bar text
         progress_construction = {163, 144, 19}, -- Construction/Dismantle site progress
         productivity_low = {187, 0, 0},         -- Low building productivity
         productivity_medium = {255, 225, 30},   -- Medium building productivity
         productivity_high = {0, 187, 0},        -- High building productivity
         chat_message = {51, 255, 51},       -- Chat message text color
         chat_me = { 238, 238, 238 },        -- Color for /me messages
         chat_spectator = { 153, 153, 153 }, -- Chat spectator player color
         chat_log = { 221, 221, 221 },       -- Chat log messages color
         plot_axis_line = { 0, 0, 0 },       -- Statistics plot
         plot_zero_line = { 255, 255, 255 }, -- Statistics plot
         plot_xtick = { 255, 0, 0 },         -- Statistics plot
         plot_yscale_label = { 60, 125, 0 }, -- Statistics plot
         plot_min_value = { 125, 0, 0 },     -- Statistics plot
         heading_color_wui = { 209, 209, 209 },      -- Map and game details etc
         heading_color_fsmenu = { 255, 255, 0 },     -- Map and game details etc
         contents_color_wui = { 255, 255, 0 },       -- Map and game details etc
         contents_color_fsmenu = { 209, 209, 209 },  -- Map and game details etc
         game_setup_headings = { 0, 255, 0 },     -- Internet lobby and launch game
         game_setup_mapname = { 255, 255, 127 },  -- Internet lobby and launch game
         game_tip = { 33, 33, 27 },  -- Tips in progress screen
         intro = { 192, 192, 128 },  -- Intro screen
      }
   }
}
