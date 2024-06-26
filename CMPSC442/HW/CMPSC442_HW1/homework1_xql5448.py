############################################################
# CMPSC 442: Uninformed Search
############################################################

student_name = "Xuhong Lin"    # 0.6

############################################################
# Imports
############################################################
# Include your imports here, if any are used.
import math
import random
import copy
from collections import deque

############################################################
# Section 1: N-Queens
############################################################
# Gloabl Micro and Variable Declear

def num_placements_all(n): # 3/3
    '''
    >>> num_placements_all(1)
    1
    >>> num_placements_all(2)
    6
    >>> num_placements_all(3)
    84
    '''
    # variable declear
    totalGrid = n**2
    return int(math.factorial(totalGrid)/(math.factorial(totalGrid-n) * math.factorial(n)))


def num_placements_one_per_row(n): #2/2
    '''
    >>> num_placements_one_per_row(1)
    1
    >>> num_placements_one_per_row(2)
    4
    >>> num_placements_one_per_row(3)
    27
'''
    return n**n


def n_queens_valid(board): #5/5
    '''
    >>> n_queens_valid([0,0])
    False
    >>> n_queens_valid([0,2])
    True
    >>> n_queens_valid([0,1])
    False
    >>> n_queens_valid([0,3,1])
    True
    >>> n_queens_valid([2, 1, 3, 0])
    False
    >>> n_queens_valid([0, 3, 3])
    False
    >>> n_queens_valid([1, 4, 10, 9])
    False
    >>> n_queens_valid([0, 1, 2, 3, 4])
    False
    '''
    for i in range(0, len(board)-1):
        for j in range(i+1,len(board)):
            if board[i] == board[j]: return False
            elif i + board[i] == j + board[j]: return False
            elif i - board[i] == j - board[j]: return False
    return True
                
    
      
def n_queens_solutions(n): #15/15
    '''
    >>> solutions = n_queens_solutions(4)
    >>> next(solutions)
    [1, 3, 0, 2]
    >>> next(solutions)
    [2, 0, 3, 1]
    >>> list(n_queens_solutions(6))
    [[1, 3, 5, 0, 2, 4], [2, 5, 1, 4, 0, 3], [3, 0, 4, 1, 5, 2], [4, 2, 0, 5, 3, 1]]
    >>> len(list(n_queens_solutions(8)))
    92
    '''
    # Declear variable
    tempBoard = []
    # check for valid n input
    if n <= 0: return []
    
    return n_queens_backtracing(tempBoard, n)


def n_queens_backtracing(Board, n):  
    # termination step
    if (len(Board) == n):
        #print(Board)
        yield Board[:]
        
    # backtracing section 
    for colIndex in range(0,n):
        if (checkValid(Board, colIndex)):
            Board.append(colIndex)
            yield from n_queens_backtracing(Board, n)
            Board.pop()
            

def checkValid(Board, colIndex):
    for rowIndex in range(0, len(Board)):
        if Board[rowIndex] == colIndex: return False
        if len(Board)+colIndex == rowIndex+Board[rowIndex]: return False
        if len(Board)-colIndex == rowIndex-Board[rowIndex]: return False
        
    return True
            

############################################################
# Section 2: Lights Out
############################################################

class LightsOutPuzzle(object):
    '''
    >>> b = [[True, False], [False, True]]
    >>> p = LightsOutPuzzle(b)
    >>> p.get_board()
    [[True, False], [False, True]]
    >>> b = [[True, True], [True, True]]
    >>> p = LightsOutPuzzle(b)
    >>> p.get_board()
    [[True, True], [True, True]]
    >>> p = create_puzzle(2, 3)
    >>> p.get_board()
    [[False, False, False], [False, False, False]]
    >>> p = create_puzzle(3, 3)
    >>> p.perform_move(1, 1)
    >>> p.get_board()
    [[False, True, False], [True, True, True], [False, True, False]]
    >>> p = create_puzzle(3, 3)
    >>> p.perform_move(0, 0)
    >>> p.get_board()
    [[True, True, False], [True, False, False], [False, False, False]]
    >>> p.perform_move(2,2)
    >>> p.get_board()
    [[True, True, False], [True, False, True], [False, True, True]]
    >>> p.perform_move(1,1)
    >>> p.get_board()
    [[True, False, False], [False, True, False], [False, False, True]]
    >>> p = create_puzzle(1,1)
    >>> p.get_board()
    [[False]]
    >>> p.perform_move(0,0)
    >>> p.get_board()
    [[True]]
    >>> p = LightsOutPuzzle([[True, False], [False, True]])
    >>> p.is_solved()
    False
    >>> p = LightsOutPuzzle([[False, False], [False, False]])
    >>> p.is_solved()
    True
    >>> p = create_puzzle(3, 3)
    >>> p2 = p.copy()
    >>> p.get_board() == p2.get_board()
    True
    >>> p.perform_move(1,1)
    >>> p.get_board() == p2.get_board()
    False
    >>> p = create_puzzle(2, 2)
    >>> for move, new_p in p.successors():
    ...     print(move, new_p.get_board())
    (0, 0) [[True, True], [True, False]]
    (0, 1) [[True, True], [False, True]]
    (1, 0) [[True, False], [True, True]]
    (1, 1) [[False, True], [True, True]]
    >>> for i in range(2, 6):
    ...     p = create_puzzle(i, i + 1)
    ...     print(len(list(p.successors())))
    6
    12
    20
    30
    >>> p = create_puzzle(2, 3)
    >>> for row in range(2):
    ...     for col in range(3):
    ...         p.perform_move(row, col)
    >>> p.find_solution()   
    [(0, 0), (0, 2)]
    >>> b = [[False, False, False],
    ... [False, False, False]]
    >>> b[0][0] = True
    >>> p = LightsOutPuzzle(b)
    >>> p.find_solution() is None
    True
    >>> p = create_puzzle(8,6)
    >>> p.perform_move(2,5)
    >>> p.perform_move(4,2)
    >>> p.perform_move(7,1)
    >>> p.find_solution()
    [(2, 5), (4, 2), (7, 1)]
    '''

    def __init__(self, board): # 5/5
        self.board = board
        self.rownum = len(board)
        self.colnum = len(board[0])

    def get_board(self): # 5/5
        return self.board

    def perform_move(self, row, col): #5/5
        # Check invalid input ??
        # toggle given point
        self.board[row][col] = bool(True - self.board[row][col])
        # Check four directions
        if (row != 0): self.board[row-1][col] = bool(True - self.board[row-1][col]) # Check above
        if (col != 0): self.board[row][col-1] = bool(True - self.board[row][col-1]) # Check left
        if (row != self.rownum - 1): self.board[row+1][col] = bool(True - self.board[row+1][col]) # Check below
        if (col != self.colnum - 1): self.board[row][col+1] = bool(True - self.board[row][col+1]) # Check right
    
    def scramble(self): # 0/0
        for i in range(0, self.rownum):
            for j in range(0, self.colnum):
                if random.random() < 0.5:
                    self.perform_move(i, j)

    def is_solved(self): # 2/2
        for i in range(0, self.rownum):
            for j in range(0, self.colnum):
                if self.board[i][j] == True:
                    return False
        return True

    def copy(self): # 3/3
        # make a deep copy 
        return copy.deepcopy(self)

    def successors(self): #5/5
        for i in range(0, self.rownum):
            for j in range(0, self.colnum):
                temp = self.copy()
                temp.perform_move(i,j)
                yield ((i,j), temp)

    def find_solution(self): # 13.8/15
        dict = {} # key = tuple of current node or board, value = [move, parent board]
        result = []
        
        # define queue and init queue and dict
        visited_set = set()
        queue = deque()
        queue.append(self)
        dict[self.nestList2Tuple()] = [None, None]
        
        # set root node as visited 
        visited_set.add(self.nestList2Tuple())
        
        while queue:
            visitedBoard = queue.popleft()
            
            for move, nextBoard in visitedBoard.successors():
                nextBoardTuple = nextBoard.nestList2Tuple()
                
            #    print("parent Node is = ", visitedBoard.board)
            #    print("move = ", move)
            #    print("nextBoard =", nextBoard.board)
                
                # if visited, then ignore
                if nextBoardTuple in visited_set:
                #    print("enter continue setion")
                    continue
                # if not, mark the node and store info
                visited_set.add(nextBoardTuple)
                dict[nextBoard] = [move, visitedBoard]
                
                if nextBoard.is_solved():
                    # get all moves
                    currentBoard = nextBoard
                    while (currentBoard != self):
                        result = [dict[currentBoard][0]] + result
                        currentBoard = dict[currentBoard][1]
                    return result
                queue.append(nextBoard)
                
        #    for i in queue:
        #        print(i.board)
        #    print("end of one successor")
        return None
        
    def nestList2Tuple(self):
        result = ()
        for i in self.board:
            result = result + tuple(i)      
        return result
        
def create_puzzle(rows, cols): #5/5
    if rows <=0 or cols <= 0:
        return None
    newBoard = [[False for j in range(0, cols)] for i in range(0,rows)]
    newBoardClass = LightsOutPuzzle(newBoard)        
    return newBoardClass
    

############################################################
# Section 3: Linear Disk Movement
############################################################

def solve_identical_disks(length, n): # 14/15
    '''
    >>> solve_identical_disks(4, 2)
    [(0, 2), (1, 3)]
    >>> solve_identical_disks(5, 2) 
    [(0, 2), (1, 3), (2, 4)]
    >>> solve_identical_disks(4, 3)
    [(1, 3), (0, 1)]
    >>> solve_identical_disks(5, 3)
    [(1, 3), (0, 1), (2, 4), (1, 2)]
    '''
    if length == n:
        return None
    board = I_createNewBoard(length, n)
    return solve_disk(length, n, board)
  

def solve_distinct_disks(length, n): # 14/15
    '''
    >>> solve_distinct_disks(4, 2)
    [(0, 2), (2, 3), (1, 2)]
    >>> solve_distinct_disks(5, 2)
    [(0, 2), (1, 3), (2, 4)]
    >>> solve_distinct_disks(4, 3)
    [(1, 3), (0, 1), (2, 0), (3, 2), (1, 3), (0, 1)]
    >>> solve_distinct_disks(5, 3)
    [(1, 3), (2, 1), (0, 2), (2, 4), (1, 2)]
    '''
    if length == n:
        return None
    board = D_createNewBoard(length, n)
    return solve_disk(length, n, board)


def I_createNewBoard(length, n):
    board = []
    for i in range(0,length):
        if i < n:
            board.append(1)
        else:
            board.append(0)
    return board

def D_createNewBoard(length, n):
    cnt = 1
    board = []
    for i in range(0, length):
        if i < n:
            board.append(cnt)
            cnt = cnt + 1
        else:
            board.append(0)
    return board
            
def move_func(index, step, maxLength, board):
    board[index + step] = board[index]
    board[index] = 0
    
def successor(board):
    length = len(board)
    # four possiable moves
    for i in range(length):
        if board[i] !=0:
            if i + 1 < length and board[i + 1] == 0:
                tempBoard = copy.deepcopy(board)
                move_func(i, 1, length, tempBoard)
                yield((i,i+1), tempBoard)
                
            if i - 1 >= 0 and board[i - 1] == 0:
                tempBoard = copy.deepcopy(board)
                move_func(i, -1, length, tempBoard)
                yield((i,i-1), tempBoard)
                
            if i + 2 < length and board[i + 2] == 0 and board[i + 1] != 0:
                tempBoard = copy.deepcopy(board)
                move_func(i, 2, length, tempBoard)
                yield((i,i+2), tempBoard)
                
            if i - 2 >= 0 and board[i - 2] == 0 and board[i - 1] != 0:
                tempBoard = copy.deepcopy(board)
                move_func(i, -2, length, tempBoard)
                yield((i,i-2), tempBoard) 

def LD_is_solved(testBoard, targetBoard):
    if testBoard == targetBoard:
        return True
    return False

def solve_disk(length, n, board):
    targetBoard = board[::-1]
    boardTuple = tuple(board) # list is mutable, cannnot be used as a key
    
    dict = {} # key = tuple of current node or board, value = [move, parent board]
    result = []
    
    # define queue
    visited_set = set()
    queue = deque([board])
    
    # set root node as visited
    visited_set.add(boardTuple)
    
    #init dictionary
    dict[boardTuple] = [(0,0), None]
    
    while queue:
        visitedBoard = queue.popleft()
            
        for move, nextBoard in successor(visitedBoard):
            nextBoardTuple = tuple(nextBoard)
            
            # if visited then ignore
            if nextBoardTuple in visited_set:
                continue
            # mark neighbors as visited and store move and parent node to dict
            visited_set.add(nextBoardTuple)
            dict[nextBoardTuple] = [move, visitedBoard]
            
            # check if solved, if solved, return the moves, if not, append this one to queue
            if LD_is_solved(nextBoard, targetBoard):
                # get all the moves from dictionary
                currentBoard = nextBoard
                while (currentBoard != board):
                    result = [dict[tuple(currentBoard)][0]] + result
                    currentBoard = dict[tuple(currentBoard)][1]
                return result
           
            queue.append(nextBoard)      
    return None


if __name__ == "__main__":
    import doctest
    doctest.testmod()