-- RST
-- table.lua
-- -------------
--
-- This script adds some functions that are useful to work with Lua tables as
-- Sets or Arrays

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
   
-- RST
-- .. function:: array_unique(a)
--
--    Removes duplicated values from an array, so that for example
--
--    .. code-block:: lua
--
--       array_unique{"one", "two", "one", "three"}
--
--    will return 
--
--    .. code-block:: lua
--
--       {"one", "two", "three"}
--
--    :arg a: One array object containing duplicated values
--    :type a: :class:`array`
--
--    :returns: array with double values removed
function array_unique(a)
   local set = {}
   rv = {}
   for idx,value in ipairs(a) do
      if not set[value] then 
         table.insert(rv, value)
         set[value] = idx
      end
   end
   return rv
end
