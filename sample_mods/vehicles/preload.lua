
function serach_vehicles(item, active)
  local vehicles = game.get_vehicles()
  for _,v in ipairs(vehicles) do
    game.add_msg(v.name .. ": (" .. tostring(v.posx) .. "," .. tostring(v.posy) .. ")")
  end
end

game.register_iuse("IUSE_SEARCH_VEHICLES", serach_vehicles)
