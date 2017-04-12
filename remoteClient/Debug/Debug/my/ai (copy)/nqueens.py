"""N-Queens problem"""

from search import *
import sys

class NQueensProblem(Problem) :
	"""Subclass of search.Problem"""
	
	def __init__(self, n) :
		"""input n: size of table """
		super(NQueensProblem, self).__init__(tuple([0 for j in range(n)]), None)
		self.n = n

	def actions(self, state) :
		if 0 in state :
			col = state.index(0)
		else :
			col = len(state)-1
		return [ (col, i+1) for i in range(self.n) ]
		
	def result(self, state, action) :
		i = action[0]
		j = action[1]
		
		# Copy state
		newStateList = list(state)
			
		newStateList[i] = j
		return tuple(newStateList)
		
	def goal_test(self, state) :
		if 0 in state :
			return False
		else :
			for i in range(self.n) :
				for j in range(i+1, self.n) :
					if ( state[i]==state[j] or ( abs(j-i) == abs(state[i]-state[j]) ) ) :
						return False
			return True
		
	# Useless functions
#	def path_cost(self, c, state1, action, state2) :
#		return c+1
#		
#    def value(self, state) :
#    	return 1
    	
    	
def h1(n) :
	state = n.state
	if 0 in state :
		i = state.index(0)
	else :
		i = len(state)-1
	if (i == 0 ) :
		return state[0]
	else :
		return abs(state[i]-state[i-1])

def h2(n) :
	state = n.state
	if 0 in state :
		i = state.index(0)
	else :
		i = len(state)-1
	if (i == 0 ) :
		return state[0]
	else :
		sum = 0
		for j in range(1, i) :
			sum += abs(state[i]-state[i-1]) 
		return sum*1.0 / i
		
				
p = NQueensProblem(int(sys.argv[1]))
#print p.initial, type(p.initial)
#for a in p.actions(p.initial) :
#	print p.result(p.initial, a)
print "N:", sys.argv[1], "- Heuristic:", sys.argv[2]
if sys.argv[2] == "1" :
	solution = astar_search(p, lambda node : h1(node))
else :
	solution = astar_search(p, lambda node : h2(node))
	
l = list(solution.state)
for r in l :
	sys.stdout.write("+")
	sys.stdout.write("-"*(len(l)*2-1))
	sys.stdout.write("+\n")
	for c in range(len(l)) :
		if ( c != r-1 ) :
			sys.stdout.write('|o') 
		else :
			sys.stdout.write('|x');
	sys.stdout.write('|\n')

