--!  #textdomain wesnoth-Roboke

local H = wesnoth.require "lua/helper.lua"

function wesnoth.wml_actions.save_shroud_map(cfg)
  local s = wesnoth.sides[1]
  local v = cfg.variable or H.wml_error "save_shroud_map missing required variable= attribute."
  wesnoth.set_variable(v, string.format("%s", s.__cfg.shroud_data))
end

function wesnoth.wml_actions.save_fog_map(cfg)
  local s = wesnoth.sides[1]
  local v = cfg.variable or H.wml_error "save_fog_map missing required variable= attribute."
  wesnoth.set_variable(v, string.format("%s", s.__cfg.fog_data))
end

function string:split(sep)
        local sep, fields = sep or ":", {}
        local pattern = string.format("([^%s]+)", sep)
        self:gsub(pattern, function(c) fields[#fields+1] = c end)
        return fields
end

function wesnoth.wml_actions.load_shroud_map(cfg)
  local v = cfg.variable or H.wml_error "load_shroud_map missing required variable= attribute."
  local sr = wesnoth.get_variable(v)
	if sr == nil then
		return
	end
	if sr == "" then
		return
	end
  local st = sr:split("|")

  local w, h, b = wesnoth.get_map_size()
  for xx = 1 - b, w + b do
    local sx = st[xx+1]
		if sx == nil then
			wesnoth.fire("place_shroud", { side = 1, x = xx})
		else
			for yy = 1 - b, h + b do
				local r = sx:sub(yy + b, yy + b)
				if r == "1" then
					wesnoth.fire("remove_shroud", { side = 1, x = xx, y = yy })
				else
					wesnoth.fire("place_shroud", { side = 1, x = xx, y = yy })
				end
			end
		end
  end
end

function wesnoth.wml_actions.load_fog_map(cfg)
  local v = cfg.variable or H.wml_error "load_fog_map missing required variable= attribute."
  local sr = wesnoth.get_variable(v)
	if sr == nil then
		return
	end
	if sr == "" then
		return
	end
  local st = sr:split("|")

  local w, h, b = wesnoth.get_map_size()
  for xx = 1 - b, w + b do
    local sx = st[xx+1]
		if not sx == nil then
			for yy = 1 - b, h + b do
				local r = sx:sub(yy + b, yy + b)
				if r == "1" then
					wesnoth.fire("lift_fog", { x = xx, y = yy })
				end
			end
		end
  end
end

-- see http://wiki.wesnoth.org/LuaWML:Tiles
-- * id: string
-- * name, description: translatable strings
-- * castle, keep, village: booleans
-- * healing: integer
function wesnoth.wml_actions.get_terrain_info(cfg)
  local v = cfg.variable or H.wml_error "get_terrain_info missing required variable= attribute."
-- TODO check if cfg.x, cfg.y are set and inside bounds.
--local w,h,b = wesnoth.get_map_size()
  local tr = wesnoth.get_terrain_info(wesnoth.get_terrain(cfg.x, cfg.y))
  wesnoth.set_variable(v, tr)
end

-- http://wiki.wesnoth.org/LuaWML:Misc
-- *  version: string (read only)
-- * base_income: integer (read/write)
-- * village_income: integer (read/write)
-- * poison_amount: integer (read/write)
-- * rest_heal_amount: integer (read/write)
-- * recall_cost: integer (read/write)
-- * kill_experience: integer (read/write)
-- * debug: boolean (read only)
function wesnoth.wml_actions.get_game_config(cfg)
  local v = cfg.variable or H.wml_error "get_game_config missing required variable= attribute."
  wesnoth.set_variable(v,  wesnoth.game_config)
end
