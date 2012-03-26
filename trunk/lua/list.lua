
function CurrentGameList()
	local nu = require("PlanetNu")

	local list = nu.GetList()

	print(string.format("%-20s  %10s  %20s","Name","Turn","Short Description"))
	print(string.format("-----------------------------------------"))
	for i,v in ipairs(list) do
		if i > print_count then break end
		print(string.format("%-20s %10d %20d",v.name,v.turn,v.shortdescription)
	end
end

CurrentGameList()
