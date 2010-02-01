function include(s) 
   use("map", s)
end

include "lunit"
lunit.import "assertions"

include "test_field"
include "test_immovables"
include "test_player"

rv = lunit:run()
if rv == 0 then -- No errors in the testsuite. Exit the game
   wl.debug.exit()
else
   wl.debug.set_see_all(1) -- Reveal everything, easier for debugging
end

