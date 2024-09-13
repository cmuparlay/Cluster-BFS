import os

website="https://pasgal-bs.cs.ucr.edu/bin/"

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
# GRAPH_DIR = f"{CURRENT_DIR}/../data/graphs"
GRAPH_DIR = f"/data/graphs/bin"

graphs = [
"Epinions1",
"Slashdot",
"DBLP",
"com-youtube",
"skitter",
"in_2004",
"soc-LiveJournal1",
"hollywood_2009",
"socfb-uci-uni",
"socfb-konect",
"com-orkut",
"indochina",
"eu-2015-host",
"uk-2002",
"arabic",
"twitter",
"friendster",
"sd_arc"
]

graph_map={
"Epinions1": "EP",
"Slashdot":"SLDT",
"DBLP":"DBLP",
"com-youtube":"YT",
"skitter":"SK",
"in_2004":"IN04",
"soc-LiveJournal1":"LJ",
"hollywood_2009":"HW",
"socfb-uci-uni":"FBUU",
"socfb-konect":"FBKN",
"com-orkut":"OK",
"indochina":"INDO",
"eu-2015-host":"EU",
"uk-2002":"UK",
"arabic":"AR",
"twitter":"TW",
"friendster":"FT",
"sd_arc":"SD"
}