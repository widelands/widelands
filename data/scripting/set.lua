-- RST
-- set.lua
-- -------------
--
-- This adds a new data type Set.

-- RST
-- Set
-- ---
--
-- .. class:: Set(iteratable)
--
--    A set is a collection of items. Each item must compare uniquely
--    in the set, otherwise it is discarded; that is a Set never contains
--    the same item more than once.
--
--    It works with all types that are either unique (strings) or provide a
--    __hash property. :class:`wl.map.Field` and :class:`wl.map.MapObject`
--    implement such a property

-- RST
--    .. attribute:: size
--
--       (RO) Due to a bug in Lua 5.1, one cannot use the '#' operator to get
--       the number of items in a set. Use this property instead.
--
Set = {}

function Set:new(l)
   local set = {
      size = 0,
   }
   setmetatable(set, self)
   self.__index = self

   if l then
      for _, v in ipairs(l) do
         set:add(v)
      end
   end

   return set
end

-- RST
--    .. method:: add(item)
--
--       Add this item to the set
function Set:add(item)
   local hash = item.__hash or item
   if not self[hash] then
      self.size = self.size + 1
      self[hash] = item
   end
end

-- RST
--    .. method:: discard(item)
--
--       Remove this item from the set. Does nothing if the item is not in the
--       set.
function Set:discard(item)
   local hash = item.__hash or item
   if self[hash] then
      self.size = self.size - 1
      self[hash] = nil
   end
end

-- RST
--    .. method:: contains(item)
--
--       Returns :const:`true` if the item is contained in this set,
--       :const:`false` otherwise
function Set:contains(item)
   local hash = item.__hash or item
   return self[hash] ~= nil
end

-- RST
--    .. method:: pop_at(n)
--
--       Returns the n-th item of this set and removes it. Note that the only
--       way to get to this item is by iterating, so this function scales
--       linearly with n.
function Set:pop_at(n)
   assert(n <= self.size)
   assert(n >= 1)
   local ghash, gitem
   for hash,item in pairs(self) do
      if hash ~= "size" then
         if n == 1 then
            ghash, gitem = hash, item
            break
         end
         n = n - 1
      end
   end
   self:discard(gitem)
   return gitem
end

-- RST
--    .. method:: items
--
--       Iterator function that allows easy iterating of all items.
--
--       .. code-block:: lua
--
--          s = Set:new{"a","b","c"}
--          for i in s:items() do print(i) end
function Set:items()
   local co = coroutine.create(function ()
      for hash, v in pairs(self) do
         if hash ~= "size" then
            coroutine.yield(v)
         end
      end
   end)
   return function ()   -- iterator
      local code, res = coroutine.resume(co)
      return res
   end
end

-- RST
--    .. method:: union(other_set)
--
--       Returns a new set that is the union of both sets. This is
--       also overloaded to the '+' operator
function Set:union(b)
   local rv = Set:new()
   for hash,item in pairs(self) do
      if hash ~= "size" then rv:add(item) end
   end
   for hash,item in pairs(b) do
      if hash ~= "size" then rv:add(item) end
   end
   return rv
end
Set.__add = Set.union

-- RST
--    .. method:: subtract(other_set)
--
--       Returns a new set that contains all values of this set that
--       are not in other_set. This is also overloaded to the '-' operator.
function Set:substract(b)
   local rv = Set:new()
   for hash, value in pairs(self) do
      if hash ~= "size" then rv:add(value) end
   end
   for hash, value in pairs(b) do
      if hash ~= "size" then rv:discard(value) end
   end
   return rv
end
Set.__sub = Set.substract

-- RST
--    .. method:: intersection(other_set)
--
--       Returns a new set that contains all values of this set that are also
--       in other_set. This is also overloaded to the '*' operator.
function Set:intersection(b)
   local rv = Set:new{}
   for hash,value in pairs(self) do
      if b[hash] and hash ~= "size" then
         rv:add(value)
      end
   end
   return rv
end
Set.__mul = Set.intersection

function Set:__eq(b)
   if b.size == self.size and (self-b).size == 0 then return true end
   return false
end

function Set:__tostring()
   local l = {}
   for hash,item in pairs(self) do
      if hash ~= "size" then
         l[#l + 1] = tostring(item)
      end
   end
   table.sort(l)
   return '{' .. table.concat(l, ", ") .. '}'
end
