What is GP2Lap
--------------

GP2Lap is a real-time extension for GP2.
Its main functions include logging information for online competitions, like processor occupancy, and showing on-screen information such as the current standings, fastest lap times, and a track map.

GP2Lap should work with all (language) versions of GP2.
If you have trouble getting it to work with your version then please let us know.

Under Windows XP or newer, GP2 can be run inside [DOSBox](http://www.dosbox.com/).
A whole copy of GP2 including a DOSBox configuration can be found on [OldGames.sk](http://www.oldgames.sk/en/game/grand-prix-2/download/5004/).

Building GP2Lap
---------------

GP2Lap can be cross compiled to DOS/4G with [Open Watcom](http://www.openwatcom.org/).

The included Makefile requires Windows XP/Vista/7/8, with the Watcom/BINNT directory in the search PATH.
It can be run with nmake.exe and it will use wcc386.exe, wasm.exe and wlink.exe to build gp2lap.exe.
To run gp2lap.exe in DOSBox, dos4gw.exe needs to be present too. It can be copied from the Watcom/BINW diretory.
