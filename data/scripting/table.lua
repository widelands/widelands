-- RST
-- table.lua
-- -------------
--
-- This script adds some functions that are useful to work with Lua tables as
-- Sets or Arrays
--
-- To make these function(s) available include this file at the beginning
-- of a script via:
--
-- .. code-block:: lua
--
--    include "scritping/table.lua"
--

-- RST
-- .. function:: array_combine(arrays)
--
--    Takes all arguments which must be Lua Arrays (that is tables with integer
--    keys starting with index 1) and combines them into one single array.
--    This is must useful to combine arrays of Fields.
--
--    :arg arrays: any number of arrays
--    :returns: a new array with all values of all arguments
function array_combine(...)
   local t = {}
   for _,arg in ipairs{...} do
      for _,v in ipairs(arg) do
         t[#t+1] = v
      end
   end
   return t
end
