use("map", "common_init")

-- ==========
-- Constants
-- ==========
if not wl.editor then
   game = wl.Game()
   -- make sure we do not crash because of saving.
   game.allow_saving = false
   egbase = game
else
   editor = wl.Editor()
   egbase = editor
end
map = egbase.map

player1 = egbase.players[1]
player2 = egbase.players[2]
player3 = egbase.players[3]

-- =================================
-- Tests for the core functionality
-- =================================
include "egbase"

include "math_random"
include "string_bformat"
include "path"
include "map"
include "cplayer"
include "cfield"
include "cplr_access"

include "immovables"

if not wl.editor then
   include "game"

   include "gplayer"
   include "gfield"
   include "gplr_access"

   include "objectives"
   include "messages"

   -- The functionality of UI scripting is identical but we only test it in game
   -- though as it is cumbersome to make those tests work also in the editor.
   include "ui"
else
   include "editor"
   include "eplr_access"

   include "efield"
end

-- ===========================
-- Test for auxiliary scripts
-- ===========================
include "table"
include "set"

-- ============
-- Test Runner
-- ============
lunit:run()
wl.ui.MapView():close()
