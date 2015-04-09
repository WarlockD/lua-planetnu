### Starting Up ###
The very basic is to copy the PlanetNu.dll into the same directory as the lua.exe is in.

Once there you just require it like any other library, for example.
`nu = require("PlanetNu")`
You will see a file, if it dosn't exisit, be created called "dkjson.lua"  This is temorary till I get around to code an internal loader for it.

The PlanetNuLua table is in this structure
nu.json = dkjson parser
nu.lpeg = lpeg string handler
nu.zlib = some zlib functions

### Functions ###
`nu.GetList()`

Gets the current game list from PlanetNu and puts it in a table

`nu.GetList({ username, scope, type, status })`

Filters games dependsing on the settings on the sent table.  For example:
`nu.GetList({ username="joshua", status=3 })`

Will return a list of all games created by joshua that have finished.  You can see more codes ate the site http://planets.nu/api-documentation/list-games

`nu.GetTurn(gameid,playerid [,turn])`

Gets a finished game or turn in that game.  Once it gets the file, it will save the turn on your hard disk as a gzip file.
If you try to load the turn again, it will try to find the file first and read that.

Eventually this will mimic all the other api.

### Extras ###
I have two lua files you can test out with.
> `planets.lua`
> If you run this, it will output the top 20 Duranium resources on player id 3 in game 815.

> `list.lua`
> It will load up the current game list, and sort them by the amount of turns played.