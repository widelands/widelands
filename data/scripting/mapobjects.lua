-- RST
-- mapobjects.lua
-- --------------
--
-- This script contains some convenience functions for loading the tribes
-- and world.
--

-- RST
-- .. function:: print_loading_message(preamble, func)
--
--    Prints a message containing the duration func() required to run.
--
--    :arg preamble: The name of the item that was being loaded
--    :type preamble: :class:`string`
--    :arg func: The function to time execution off.
--    :type func: :class:`function`
function print_loading_message(preamble, func)
   local start = ticks()
   func()
   print(("%s: %dms"):format(preamble, ticks() - start))
end
