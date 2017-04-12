"""Aline tiles problem"""

from search import *
import sys

class AlienTiles(Problem) :

	def __inti__(self, n) :
	i = n
	j = n
	random.seed()
	for c1 in i :
		for c2 in j :
			grid[i][j] = random.getstate mod 4
	for c1 in i :
		print "\n"
		for c2 in j :
			print grid[i][j]


p = AlienTiles(int(sys.argv[1]))

