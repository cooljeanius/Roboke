CFLAGS=-Wall -O2

default: maps

wmap: wmap.c

clean:
	rm -f wmap

maps: wmap
	./wmap -1 > ../maps/island_cave.map
	./wmap -2 > ../maps/ship.map
	./wmap -3 > ../maps/ship_sea.map
	./wmap -4 > ../maps/riddle.map
	./wmap -5 > ../maps/Testing.map
	./wmap 0 > ../maps/east_shore.map
	./wmap 1 > ../maps/amazon_city_nww.map
	./wmap 2 > ../maps/north_shore.map
	./wmap 4 > ../maps/south_east.map
	./wmap 5 > ../maps/south_west.map
	./wmap 7 > ../maps/island_hill.map
	./wmap 8 > ../maps/north_east.map
	./wmap 9 > ../maps/north_west.map
	./wmap 10 > ../maps/southern_cliffs.map
	./wmap 11 > ../maps/southern_cave.map
	./wmap 13 > ../maps/north_cave.map
	./wmap 12 > ../maps/island_nw.map
	./wmap 14 > ../maps/southern_secrets.map
	./wmap 15 > ../maps/river_passage.map
	./wmap 16 > ../maps/south_east_cave.map
	./wmap 17 > ../maps/prison_camp.map
	#./wmap 18 > ../maps/northern_island.map
	./wmap 19 > ../maps/crater_island.map
	./wmap 20 > ../maps/lair.map
	./wmap 21 > ../maps/south_west_cave.map
	./wmap 22 > ../maps/oceb_hill.map
	./wmap 23 > ../maps/deceipt_cave.map
	./wmap 24 > ../maps/deceipt_hill.map
	./wmap 25 > ../maps/oceb_cave.map
	./wmap 26 > ../maps/amazon_cave_nww.map

.PHONY: clean maps default
