-- RST
-- set.lua
-- -------------
--
-- This adds a new data type Set. It works with all types
-- that are either unique (strings) or provide a __hash property. 
-- :class:`wl.map.Field` implements such a property

-- Prototype
Set = {}

-- SirVer, TODO: document all of these
-- SirVer, TODO: add a persistence test

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

function Set:add(item)
   local hash = item.__hash or item
   if not self[hash] then
      self.size = self.size + 1
      self[hash] = item
   end
end

function Set:discard(item)
   local hash = item.__hash or item
   if self[hash] then
      self.size = self.size - 1
      self[hash] = nil
   end
end

function Set:contains(item)
   local hash = item.__hash or item
   return self[hash] ~= nil
end

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



