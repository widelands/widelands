-- =======================================================================
--                         Defeat all Win condition                         
-- =======================================================================

use("aux", "coroutine")

set_textdomain("win_conditions")

return {
   name = _ "Autocrat",
   check_func = function() 
      while true do
         sleep(500)
         print "hi there!"
      end
   end,
}
