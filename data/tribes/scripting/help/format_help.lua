-- RST
-- format_help.lua
-- -------------------------------------
--
-- Functions used in the ingame help windows for formatting the text and pictures.

include "scripting/richtext.lua"

--  =======================================================
--  *************** Basic helper functions ****************
--  =======================================================

-- RST
-- .. function:: image_line(image, count[, text = nil])
--
--    Aligns the image to a row on the right side with text on the left.
--
--    :arg image: the picture to be aligned to a row.
--    :arg count: length of the picture row.
--    :arg text: if given the text aligned on the left side, formatted via richtext.lua functions.
--    :returns: the text on the left and a picture row on the right.
--
function image_line(image, count, text)
   if not text then
      text = ""
   end
   local images = ""
   for i=1,count do
      images = images .. img(image)
   end

   return
      div("width=100%",
         div("width=50%", p(vspace(6) .. text .. space(6))) ..
         div("width=*", p("align=right", vspace(6) .. images .. vspace(12)))
      )
end

-- RST
-- .. function:: plot_size_line(size, size_only)
--
--    Creates a line describing space required on the map.
--    Consists of a header colored text, followed by normal text and an image.
--
--    :arg size: size key. Expected values are "mine", "port", "small, "medium", "big", "none".
--    :arg size_only: size_only key. Optional bool value if size is a space requirement as well.
--    :returns: header followed by normal text and image if a space is required, or empty string.
--
function plot_size_line(size, size_only)
   local text = ""
   local image = ""
   if (size_only == true) then
      if (size == "small") then
         -- TRANSLATORS: Size of a map immovablee
         text = _("Small")
      elseif (size == "medium") then
         -- TRANSLATORS: Size of a map immovable
         text = _("Medium")
      elseif (size == "big") then
         -- TRANSLATORS: Size of a map immovable
         text = _("Big")
      end
      -- TRANSLATORS: Size of a map immovable
      if text ~= "" then
         text = p(font("size=13 color=D1D1D1", text))
      end
      return text
   else
      if (size == "mine") then
         -- TRANSLATORS: Space on the map required for building a building there
         text = _("Mine plot")
         image = "images/wui/overlays/mine.png"
      elseif (size == "port") then
         -- TRANSLATORS: Space on the map required for building a building there
         text = _("Port plot")
         image = "images/wui/overlays/port.png"
      elseif (size == "small") then
         -- TRANSLATORS: Space on the map required for building a building there
         text = _("Small plot")
         image = "images/wui/overlays/small.png"
      elseif (size == "medium") then
         -- TRANSLATORS: Space on the map required for building a building there
         text = _("Medium plot")
         image = "images/wui/overlays/medium.png"
      elseif (size == "big") then
         -- TRANSLATORS: Space on the map required for building a building there
         text = _("Big plot")
         image = "images/wui/overlays/big.png"
      else
         return ""
      end
   -- TRANSLATORS: Space on the map required for building a building there
      text = p(join_sentences(font("size=13 color=D1D1D1", _("Space required:")), text))
      return div("width=100%", div("float=right padding_l=6", p(img(image)))) .. text
   end
end


--  =======================================================
--  ********** Helper functions for dependencies **********
--  =======================================================

-- RST
-- .. function:: dependencies(items[, text = nil])
--
--    Creates a dependencies line of any length.
--
--    :arg items: ware, worker and/or building descriptions in the correct order
--                from left to right as table (set in {}).
--    :arg text: comment of the image.
--    :returns: a row of pictures connected by arrows.
--
function dependencies(items, text)
   if not text then
      text = ""
   end
   local images = img(items[1].icon_name)
   for k,v in ipairs({table.unpack(items,2)}) do
      images = images .. img("images/richtext/arrow-right.png") ..  img(v.icon_name)
   end
   return
      div("width=100%", p(vspace(6) .. images .. space(6) .. text .. vspace(12)))
end


-- RST
-- .. function:: help_ware_amount_line(ware_description, amount)
--
--    Displays an amount of wares with name and images
--
--    :arg ware_description: The :class:`wl.map.WareDescription` for the ware type to be displayed
--    :arg amount: The amount to show as a number
--    :returns: image_line for the ware type and amount
--
function help_ware_amount_line(ware_description, amount)
   amount = tonumber(amount)
   local image = ware_description.icon_name
   local result = ""
   local imgperline = 6
   local temp_amount = amount

   while (temp_amount > imgperline) do
      result = result .. image_line(image, imgperline)
      temp_amount = temp_amount - imgperline
   end
   -- TRANSLATORS: %1$d is a number, %2$s the name of a ware, e.g. 12x Stone
   result = image_line(image, temp_amount, p(_("%1$dx %2$s"):bformat(amount, linkify_encyclopedia_object(ware_description)))) .. result
   return result
end

-- RST
-- .. function:: help_worker_experience(worker_description, becomes_description)
--
--    Displays needed experience levels for workers
--
--    :arg worker_description: The :class:`wl.map.WorkerDescription` for the lower-level worker
--    :arg becomes_description: The :class:`wl.map.WorkerDescription` for the higher-level worker
--    :returns: text describing the needed experience
--
function help_worker_experience(worker_description, becomes_description)
   local result = ""
   -- TRANSLATORS: EP = Experience Points
   local exp_string = _("%s to %s (%s EP)"):format(
         linkify_encyclopedia_object(worker_description),
         linkify_encyclopedia_object(becomes_description),
         worker_description.needed_experience
      )

   worker_description = becomes_description
   becomes_description = worker_description.becomes
   if(becomes_description) then
     -- TRANSLATORS: EP = Experience Points
      exp_string = exp_string .. "<br>" .. _("%s to %s (%s EP)"):format(
            linkify_encyclopedia_object(worker_description),
            linkify_encyclopedia_object(becomes_description),
            worker_description.needed_experience
         )
   end
   result = result .. p("align=right", exp_string)
   return result
end

-- RST
-- .. function:: help_tool_string(tribe, toolname, no_of_workers)
--
--    Displays tools with an intro text and images
--
--    :arg tribe: The :class:`wl.map.TribeDescription` for the tribe that uses the tools
--    :arg toolnames: e.g. {"shovel", "basket"}.
--    :arg no_of_workers: the number of workers using the tools; for plural formatting.
--    :returns: image_line for the tools
--
function help_tool_string(tribe, toolnames, no_of_workers)
   local result = ""
   local game  = wl.Game();
   for i, toolname in ipairs(toolnames) do
      if (tribe:has_ware(toolname)) then
         local ware_description = game:get_ware_description(toolname)
         result = result .. image_line(ware_description.icon_name, 1, p(linkify_encyclopedia_object(ware_description)))
      elseif (tribe:has_worker(toolname)) then
         local worker_description = game:get_worker_description(toolname)
         result = result .. image_line(worker_description.icon_name, 1, p(linkify_encyclopedia_object(worker_description)))
      end
   end
   return result
end


-- RST
-- .. function:: help_consumed_wares_workers(tribe, building, program_name)
--
--    Returns information for which wares and workers in which amounts are consumed by a production program.
--
--    :arg tribe: The :class:`wl.map.TribeDescription` for the tribe that consumes the ware
--    :arg building: The :class:`wl.map.BuildingDescription` for the building that runs the program
--    :arg program_name: The name of the production program that the info is collected for
--
--    :returns: A "Ware(s) consumed:" section with image_lines
--
function help_consumed_wares_workers(tribe, building, program_name)
   local result = ""
   local consumed_items_string = ""
   local consumed_items_counter = 0
   local consumed_wares_workers = building:consumed_wares_workers(program_name)
   local consumes_wares = false
   local consumes_workers = false
   for countlist, itemlist in pairs(consumed_wares_workers) do
      local consumed_itemnames = {}
      local consumed_images = {}
      local consumed_amount = {}
      local count = 1
      for consumed_item, amount in pairs(itemlist) do
         local description
         if tribe:has_ware(consumed_item) then
            description = wl.Game():get_ware_description(consumed_item)
            consumes_wares = true
         else
            description = wl.Game():get_worker_description(consumed_item)
            consumes_workers = true
         end
         consumed_itemnames[count] = _("%1$dx %2$s"):bformat(amount, linkify_encyclopedia_object(description))
         consumed_images[count] = description.icon_name
         consumed_amount[count] = amount
         count = count + 1
         consumed_items_counter = consumed_items_counter + amount
      end
      local text = localize_list(consumed_itemnames, "or")
      if (countlist > 1) then
         text = _("%s and"):bformat(text)
      end
      local images = ""
      local image_counter = 1
      while (image_counter <= consumed_amount[1]) do
         images = images .. img(consumed_images[1])
         image_counter = image_counter + 1
      end
      for k, v in ipairs({table.unpack(consumed_images,2)}) do
         image_counter = 1
         while (image_counter <= consumed_amount[k + 1]) do
            images = images .. img(v)
            image_counter = image_counter + 1
         end
      end
      consumed_items_string =
         div("width=100%",
            div("width=50%", p(vspace(6) .. text .. space(6))) ..
            div("width=*", p("align=right", vspace(6) .. images .. vspace(12)))
         )
         .. consumed_items_string
   end
   if (consumed_items_counter > 0) then
      local consumed_header = ""
      if (consumes_workers) then
         if (consumes_wares) then
            -- TRANSLATORS: Tribal Encyclopedia: Heading for wares and workers consumed by a productionsite
            consumed_header = _("Wares and workers consumed:")
         else
            -- TRANSLATORS: Tribal Encyclopedia: Heading for workers consumed by a productionsite
            consumed_header = _("Workers consumed:")
         end
      else
         -- TRANSLATORS: Tribal Encyclopedia: Heading for wares consumed by a productionsite
         consumed_header = _("Wares consumed:")
      end
      result = result .. h3(consumed_header) .. consumed_items_string
   end
   return result
end
