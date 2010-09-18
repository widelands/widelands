use("map", "common_init")

include "test_math_random"
include "test_map"
include "test_cplayer"

if not wl.editor then
   include "test_game"
   include "test_objectives"
   include "test_messages"

   include "test_gplayer"

   -- TODO: most of these tests should also run in the editor
   include "test_immovables"
end

include "test_field"
-- TODO: ui need some love. There is too much if editor stuff inside it
include "test_ui"

include "test_table"


rv = lunit:run()
if rv == 0 then -- No errors in the testsuite. Exit.
   wl.debug.exit()
elseif not wl.editor then
   wl.game.Player(1).see_all = true -- Reveal everything, easier for debugging
end
