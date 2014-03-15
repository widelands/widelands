
function automatic_forester()

local region_to_forest = map:get_field(0,0):region(23)
	while true do 
		for x,field in next,region_to_forest,f do
			if not _fully_flooded(map:get_field(field.x,field.y)) then 
				if map:get_field(field.x,field.y).immovable == 	nil then
					if (field.x + field.y) % 2 == 0 then
						map:place_immovable("tree1_t", map:get_field(field.x,field.y))
					else
						map:place_immovable("tree2_t", map:get_field(field.x,field.y))
					end
					sleep(750)
				end
			end
		end
		sleep(5*60*1000)
	end
end

function _fully_flooded(f)
   if f.terd == "wasser" and f.terr == "wasser" and
      f.tln.terr == "wasser" and f.tln.terd == "wasser" and
      f.ln.terr == "wasser" and f.trn.terd == "wasser" then
      return true
   end
   return false
end


function flooding()
	local flooded_fields = Set:new{}
	local region_to_change = joinTables(map:get_field(63,17):region(2),map:get_field(93,69):region(2))
	region_to_change = joinTables(region_to_change,map:get_field(106,112):region(2))
	region_to_change = joinTables(region_to_change,map:get_field(62,89):region(2))
	region_to_change = joinTables(region_to_change,map:get_field(97,140):region(2))
	region_to_change = joinTables(region_to_change,map:get_field(52,39):region(2))
	region_to_change = joinTables(region_to_change,map:get_field(0,0):region(20, 4))
	-- Fields are flooded
	for x,field in next,region_to_change,f do
		if field.immovable then field.immovable:remove() end
		flooded_fields:add(f_Field:new(field, field.terd, field.terr))
		local tr_to_change = Set:new{Triangle:new(map:get_field	(field.x,field.y),"d")}
		local tr = tr_to_change:pop_at(1)
		tr:set_ter("wasser")
		local tr_to_change = Set:new{Triangle:new(map:get_field	(field.x,field.y),"r")}
		local tr = tr_to_change:pop_at(1)
		tr:set_ter("wasser")
	end
	sleep(3*60*60*1000)
	-- Fields terrains are set back to their former shape
	while flooded_fields.size > 0 do
		local ff = flooded_fields:pop_at(1)
		local tr_to_setback = Set:new{Triangle:new(ff._f,"d")}
		local tr = tr_to_setback:pop_at(1)
		tr:set_ter(ff._td)
		local tr_to_setback = Set:new{Triangle:new(ff._f,"r")}
		local tr = tr_to_setback:pop_at(1)
		tr:set_ter(ff._tr)
	end
end


function volcano_eruptions()
	while true do
		sleep(80*60*1000)
		local fields_to_erupt = Set:new{}
		local central_field = map:get_field(78,77)
		local region_to_erupt = central_field:region(9,1)
		for a,field in next,region_to_erupt,f do
			if math.max(math.abs(field.x-central_field.x),math.abs(field.y-central_field.y)) < 6 or ((field.x -field.y) % 6 < 2) or ((field.x +field.y) % 	6 < 2) then
				if field.immovable then field.immovable:remove() end
				fields_to_erupt:add(f_Field:new(field, field.terd, field.terr))
				local tr_to_erupt = Set:new{Triangle:new(map:get_field	(field.x,field.y),"d")}
				local tr = tr_to_erupt:pop_at(1)
				tr:set_ter("lava")
				local tr_to_erupt = Set:new{Triangle:new(map:get_field	(field.x,field.y), "r")}
				local tr = tr_to_erupt:pop_at(1)
				tr:set_ter("lava")
			end
		end
		local central_field = map:get_field(71,40)
		local region_to_erupt = central_field:region(9,1)
		for a,field in next,region_to_erupt,f do
			if math.max(math.abs(field.x-central_field.x),math.abs(field.y-central_field.y)) < 6 or ((field.x -field.y) % 6 < 2) or ((field.x +field.y) % 	6 < 2) then
				if field.immovable then field.immovable:remove() end
				fields_to_erupt:add(f_Field:new(field, field.terd, field.terr))
				local tr_to_erupt = Set:new{Triangle:new(map:get_field	(field.x,field.y),"d")}
				local tr = tr_to_erupt:pop_at(1)
				tr:set_ter("lava")
				local tr_to_erupt = Set:new{Triangle:new(map:get_field	(field.x,field.y), "r")}
				local tr = tr_to_erupt:pop_at(1)
				tr:set_ter("lava")
			end
		end
		local central_field = map:get_field(86,123)
		local region_to_erupt = central_field:region(9,1)
		for a,field in next,region_to_erupt,f do
			if math.max(math.abs(field.x-central_field.x),math.abs(field.y-central_field.y)) < 6 or ((field.x -field.y) % 6 < 2) or ((field.x +field.y) % 	6 < 2) then
				if field.immovable then field.immovable:remove() end
				fields_to_erupt:add(f_Field:new(field, field.terd, field.terr))
				local tr_to_erupt = Set:new{Triangle:new(map:get_field	(field.x,field.y),"d")}
				local tr = tr_to_erupt:pop_at(1)
				tr:set_ter("lava")
				local tr_to_erupt = Set:new{Triangle:new(map:get_field	(field.x,field.y), "r")}
				local tr = tr_to_erupt:pop_at(1)
				tr:set_ter("lava")
			end
		end
		sleep(10*60*1000)
--		set back all eruptions		
		while fields_to_erupt.size > 0 do
			local ff = fields_to_erupt:pop_at(1)
			local tr_to_setback = Set:new{Triangle:new(ff._f,"d")}
			local tr = tr_to_setback:pop_at(1)
			tr:set_ter(ff._td)
			local tr_to_setback = Set:new{Triangle:new(ff._f,"r")}
			local tr = tr_to_setback:pop_at(1)
			tr:set_ter(ff._tr)
		end
	end
end


-- =================
-- A Field Triangle
-- =================
Triangle = {}
function Triangle:new(f, which)
   local rv = {
      _f = f,
      _d = which,
      __hash = ("%i_%i_%s"):format(f.x, f.y, which),
   }

   setmetatable(rv,self)
   self.__index = self

   return rv
end

function Triangle:get_ter()
   if self._d == "d" then
      return self._f.terd
   end
   return self._f.terr
end

function Triangle:set_ter(t)
   if self._d == "d" then
      self._f.terd = t
   else
      self._f.terr = t
   end
end

-- =================
-- A changed Field
-- =================
f_Field = {}
function f_Field:new(f, ter_d, ter_r)
   local rv = {
      _f = f,
      _td = ter_d,
      _tr = ter_r, 
      __hash = ("%i_%i_%s_%s"):format(f.x, f.y, ter_d, ter_r),
   }
   setmetatable(rv,self)
   self.__index = self

   return rv
end

function joinTables(t1, t2)
	for k,v in ipairs(t2) do
		table.insert(t1, v)
	end
	return t1
end


