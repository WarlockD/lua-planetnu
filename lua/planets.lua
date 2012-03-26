
ore = { "neutronium", "molybdenum", "duranium", "tritanium" }


function GetPlanetResources(turn, playerid, ore_type, print_count)
	
	function resource_sort_func(a,b)
		local as, bs
		as = a[ore_type]  + a["ground" .. ore_type]-- + a.groundtritanium
		bs = b[ore_type]  + b["ground" .. ore_type] -- + b.groundtritanium
		return as > bs
	end

	local nu = require "PlanetNu"

	local turn = nu.GetTurn(turn,playerid)
	local players = turn.players
	local planets = turn.planets
	local known_resources = {}
	
	-- first pass, get planets we know about 
	for i,v in ipairs(planets) do 
		if v[ore_type] ~= -1 then
			table.insert(known_resources,v)
		end
	end
	--sortby = "tritanium"
	--print_count = 10
	table.sort(known_resources, resource_sort_func)

	local ore_label  = ore_type:gsub("^%l", string.upper)
	print(string.format("%-20s  %10s  %7s","Planets",ore_label,"Mines"))
	print(string.format("-----------------------------------------"))
	for i,v in ipairs(known_resources) do
		if i > print_count then break end
		print(string.format("%-20s %5d/%5d %7d",v.name,v[ore_type],v["ground" .. ore_type],v.mines))
	end
	print("\n\n")
	return known_resources
end

GetPlanetResources(815,3,"duranium", 20)