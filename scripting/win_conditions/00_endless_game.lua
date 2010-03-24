-- =======================================================================
--                         An endless game without rules
-- =======================================================================

set_textdomain("win_conditions")
return {
   name = _ "Endless Game",
   description = _"An endless game without rules",
   -- A do nothing function
   check_func = coroutine.create(
   function() 
   end),
}
