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
--    include "scripting/table.lua"
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

-- Helper function for ordered_pairs(t)
-- Generate a sorted index from a table
-- Taken from http://lua-users.org/wiki/SortedIteration
function __gen_ordered_index(t)
    local ordered_index = {}
    for key in pairs(t) do
        table.insert( ordered_index, key )
    end
    table.sort( ordered_index )
    return ordered_index
end

-- Helper function for ordered_pairs(t)
-- Equivalent of the next function, but returns the keys in the alphabetic
-- order. We use a temporary ordered key table that is stored in the
-- table being iterated.
-- Taken from http://lua-users.org/wiki/SortedIteration
function ordered_next(t, state)

    local key = nil
    if state == nil then
        -- the first time, generate the index
        t.__ordered_index = __gen_ordered_index( t )
        key = t.__ordered_index[1]
    else
        -- fetch the next value
        for i = 1,#t.__ordered_index do
            if t.__ordered_index[i] == state then
                key = t.__ordered_index[i+1]
            end
        end
    end

    if key then
        return key, t[key]
    end

    -- no more value to return, cleanup
    t.__ordered_index = nil
    return
end

-- RST
-- .. function:: ordered_pairs(t)
--
--    Equivalent of the :func:`pairs()` function on tables. 
--    Allows to iterate in deterministic order.
--    Taken from http://lua-users.org/wiki/SortedIteration
--
--    :arg table t: The table to iterate over
--    :returns: a key,value iterator
function ordered_pairs(t)
    return ordered_next, t, nil
end

