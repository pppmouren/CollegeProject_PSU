############################################################
# CMPSC 442: Informed Search
############################################################

student_name = "Xuhong Lin"

############################################################
# Imports
############################################################

# Include your imports here, if any are used.
import copy
import random
import math
from queue import PriorityQueue
from copy import deepcopy
############################################################
# Section 1: Tile Puzzle
############################################################

def create_tile_puzzle(rows, cols):
    # init newBoard
    newBoard = [[0 for m in range(cols)] for n in range(rows)]
    count = 1
    
    if rows <= 0 or cols < 0:
        return None
    for i in range(rows):
        for j in range(cols):
            if (i == rows - 1 and j == cols - 1):
                newBoard[i][j] = 0
            else:
                newBoard[i][j] = count
                count = count + 1
    return TilePuzzle(newBoard)    

class TilePuzzle(object):
    '''
    >>> p = TilePuzzle([[1, 2], [3, 0]])
    >>> p.get_board()
    [[1, 2], [3, 0]]
    >>> p = TilePuzzle([[0, 1], [3, 2]])
    >>> p.get_board()
    [[0, 1], [3, 2]]
    >>> p = create_tile_puzzle(3, 3)
    >>> p.get_board()
    [[1, 2, 3], [4, 5, 6], [7, 8, 0]]
    >>> p = create_tile_puzzle(2, 4)
    >>> p.get_board()
    [[1, 2, 3, 4], [5, 6, 7, 0]]
    >>> p = create_tile_puzzle(3, 3)
    >>> p.perform_move("up")
    True
    >>> p.get_board()
    [[1, 2, 3], [4, 5, 0], [7, 8, 6]]
    >>> p = create_tile_puzzle(3, 3)
    >>> p.perform_move("down")
    False
    >>> p.get_board()
    [[1, 2, 3], [4, 5, 6], [7, 8, 0]]
    >>> p = TilePuzzle([[1, 2], [3, 0]])
    >>> p.is_solved()
    True
    >>> p = TilePuzzle([[0, 1], [3, 2]])
    >>> p.is_solved()
    False
    >>> p = create_tile_puzzle(3, 3)
    >>> p2 = p.copy()
    >>> p.get_board() == p2.get_board()
    True
    >>> p = create_tile_puzzle(3, 3)
    >>> p2 = p.copy()
    >>> p.perform_move("left")
    True
    >>> p.get_board() == p2.get_board()
    False
    >>> p = create_tile_puzzle(3, 3)
    >>> for move, new_p in p.successors():
    ... 	print(move, new_p.get_board())
    up [[1, 2, 3], [4, 5, 0], [7, 8, 6]]
    left [[1, 2, 3], [4, 5, 6], [7, 0, 8]]
    >>> b = [[1,2,3], [4,0,5], [6,7,8]]
    >>> p = TilePuzzle(b)
    >>> for move, new_p in p.successors():
    ... 	print(move, new_p.get_board())
    up [[1, 0, 3], [4, 2, 5], [6, 7, 8]]
    down [[1, 2, 3], [4, 7, 5], [6, 0, 8]]
    left [[1, 2, 3], [0, 4, 5], [6, 7, 8]]
    right [[1, 2, 3], [4, 5, 0], [6, 7, 8]]
    >>> b = [[4,1,2], [0,5,3], [7,8,6]]
    >>> p = TilePuzzle(b)
    >>> solutions = p.find_solutions_iddfs()
    >>> next(solutions)
    ['up', 'right', 'right', 'down', 'down']
    >>> b = [[1,2,3], [4,0,8], [7,6,5]]
    >>> p = TilePuzzle(b)
    >>> list(p.find_solutions_iddfs())
    [['down', 'right', 'up', 'left', 'down', 'right'], ['right', 'down', 'left', 'up', 'right', 'down']]
    >>> b = [[4,1,2], [0,5,3], [7,8,6]]
    >>> p = TilePuzzle(b)
    >>> p.find_solution_a_star()
    ['up', 'right', 'right', 'down', 'down']
    >>> b = [[1,2,3], [4,0,5], [6,7,8]]
    >>> p = TilePuzzle(b)
    >>> p.find_solution_a_star()
    ['right', 'down', 'left', 'left', 'up', 'right', 'down', 'right', 'up', 'left', 'left', 'down', 'right', 'right']
    '''
    
    # Required
    def __init__(self, board):
        self.board = board
        self.numRow = len(board)
        self.numCol = len(board[0])
    
    # Get the empty position index    
    def get_zero(self):
        board = self.board
        for i in range(0, self.numRow):
            for j in range(0, self.numCol):
                if (board[i][j] == 0):
                    return (i,j)
        return None

    def get_board(self):
        return self.board

    def perform_move(self, direction):
        zeroPosi = self.get_zero()
      
        if (direction == "up"):
            if(zeroPosi[0] == 0):
                return False
            self.board[zeroPosi[0]][zeroPosi[1]] = self.board[zeroPosi[0] - 1][zeroPosi[1]]
            self.board[zeroPosi[0] - 1][zeroPosi[1]] = 0
        elif (direction == "down"):
            if(zeroPosi[0] == self.numRow - 1):
                return False
            self.board[zeroPosi[0]][zeroPosi[1]] = self.board[zeroPosi[0] + 1][zeroPosi[1]]
            self.board[zeroPosi[0] + 1][zeroPosi[1]] = 0
        elif (direction == "left"):
            if(zeroPosi[1] == 0):
                return False
            self.board[zeroPosi[0]][zeroPosi[1]] = self.board[zeroPosi[0]][zeroPosi[1] - 1]
            self.board[zeroPosi[0]][zeroPosi[1] - 1] = 0
        elif (direction == "right"):
            if(zeroPosi[1] == self.numCol - 1):
                return False
            self.board[zeroPosi[0]][zeroPosi[1]] = self.board[zeroPosi[0]][zeroPosi[1] + 1]
            self.board[zeroPosi[0]][zeroPosi[1] + 1] = 0
        return True
            

    def scramble(self, num_moves):
        directions = ["up", "down", "left", "right"]
        for i in range(num_moves):
            randomDirect = random.choice(directions)
            self.perform_move(randomDirect)

    def is_solved(self):
        resultBoard = create_tile_puzzle(self.numRow, self.numCol).board
        if (self.board == resultBoard):
            return True
        return False
        

    def copy(self):
        return copy.deepcopy(self)

    def successors(self):
        possibleMove = ["up", "down", "left", "right"]
        # only four possible moves
        for move in possibleMove:
            newTilePuzzle = self.copy()
            # if true, means valid successor, then yield
            if (newTilePuzzle.perform_move(move)):
                yield move, newTilePuzzle

    # Required
    def recursive_iddfs_helper(self, limit, movePath):
        visitedBoard = [self.board]
        if self.is_solved():
            yield movePath
        elif (limit == 0):
            return None
        else:
            for move, nextTilePuzzle in self.successors():
                #ignore the visited node
                if (nextTilePuzzle.board not in visitedBoard):
                    visitedBoard.append(nextTilePuzzle.board)
                    results = nextTilePuzzle.recursive_iddfs_helper(limit - 1, movePath + [move])
                    # successor will either generate None or goal status move
                    # if None, then 
                    if results != None:
                        for move in results:
                            #print("move", move)
                            yield move
              
            
    def find_solutions_iddfs(self):
        limit = 0
        movePath = []
        while(1):
            results = list(self.recursive_iddfs_helper(limit, movePath))
            #print(results)
            if(len(results)) > 0:
                for i in results:
                    #print("i", i)
                    yield i
                break
            limit = limit + 1



    # Required
    def md_calculator(self, dict):
        rows = self.numRow
        cols = self.numCol
        board = self.board
        
        # After make a dictionary, we need to calculate the manhattan distance
        # Loop each Tile in the board and calculate the manhattan distance as:
        # md += abs(Tile Current Row Index - dict[Tile][0]) + abs(Tile Current Column Index - dict[Tile][1])
        md = 0
       
        for i in range(rows):
            for j in range(cols):
                currTile = board[i][j]
                # if in the correct position, md = 0
                if ((i,j) == dict[currTile]):
                    continue
                md = md + abs(i - dict[currTile][0]) + abs(j - dict[currTile][1])
        return md
      
      
    # 1. I will use Manhattan Distance as the huristic function to evaluate each status
    # 2. the current number of moved steps will be the g() greedy function applied as how many distance 
    #    has been moved
    # 3. priority queue will be used in the order of total evaluation value = number of current move step 
    #    + manhattan distance of this status   
    def find_solution_a_star(self):
        # use dictionary to make a standard board that each key stores the tile number and its value 
        # stores their correct position tuple. e.g: {1:(0,0), 2:(0,0), ......, {0: (rows-1, cols-1)}}
        rows = self.numRow
        cols = self.numCol
        dict = {}
        currTileNum = 1
        
        for i in range(rows):
            for j in range(cols):
                if(i == rows - 1 and j == cols - 1):
                    dict[0] = (i,j)
                else:
                    dict[currTileNum] = (i,j)
                currTileNum = currTileNum + 1
        
        
        q = PriorityQueue()
        movePath = []
        visitedBoard = set()
        #init priorityqueue
        q.put((self.md_calculator(dict), movePath, self)) # root node does not take any move, so g(n) = 0, f(n) = h(n)
        
        while(not q.empty()):
            # unlike hw 1 A* use priority queue to store node based on f(n)
            # we only put the node as visited when we decide to expand this node
            # in hw 1, we add all node to visitedBoard when we look over them to avoid 
            # duplicated cases, but in here, we explore node based on the priority value
            # even we looked before, it could be the prior node to visied again later
            (f, currMove, currTilePuzzle) = q.get()
            if(currTilePuzzle.nestList2Tuple() in visitedBoard):
                continue
            else:
                visitedBoard.add(currTilePuzzle.nestList2Tuple())
                
            if (currTilePuzzle.is_solved()):
                return currMove
            
            for move, nextTilePuzzle in currTilePuzzle.successors():
                if (nextTilePuzzle.nestList2Tuple() not in visitedBoard):
                    nextMove = currMove + [move]
                    q.put((len(currMove) + nextTilePuzzle.md_calculator(dict), nextMove, nextTilePuzzle))
        return None
        
    def nestList2Tuple(self):
        result = ()
        for i in self.board:
            result = result + tuple(i)      
        return result
        

############################################################
# Section 2: Grid Navigation
############################################################
# Straight Line Euclidean distance heuristic
def sled_calculator(currPos, goal):
    currRow, currCol = currPos[0], currPos[1]
    goalRow, goalCol = goal[0], goal[1]
    return math.sqrt((goalRow - currRow) ** 2 + (goalCol - currCol) ** 2)

def successor(currPos, scene):
    currRow, currCol = currPos[0], currPos[1]
    rowNum, colNum = len(scene), len(scene[0])
    
    for row in range(currRow - 1, currRow + 2):
        for col in range(currCol - 1, currCol + 2):
            # if the possible move is inside the scene and it is vaild to move 
            # and it is not the original position, then yield this possible move
            if (0 <= row <= rowNum - 1 and 0<= col <= colNum - 1 and scene[row][col] == False):
                if (row != currRow or col != currCol):
                    yield row,col
                    
def find_path(start, goal, scene):
    # unlike hw 1 A* use priority queue to store node based on f(n)
    # we only put the node as visited when we decide to expand this node
    # in hw 1, we add all node to visitedBoard when we look over them to avoid 
    # duplicated cases, but in here, we explore node based on the priority value
    # even we looked before, it could be the prior node to visied again later
    '''
    >>> scene = [[False, False, False], [False, True, False], [False, False, False]]
    >>> find_path((0, 0), (2, 1), scene)
    [(0, 0), (1, 0), (2, 1)]
    >>> scene = [[False, True, False], [False, True, False], [False, True, False]]
    >>> find_path((0, 0), (0, 2), scene)
    '''
    # check valid scene first
    if (scene[start[0]][start[1]] == True or scene[goal[0]][goal[1]] == True):
        return None
    # define 
    costFromStart = 0
    movePath = [start]
    q = PriorityQueue()
    visitedPos = set()
    #init
    q.put((sled_calculator(start, goal), costFromStart, movePath, start))
    
    while not q.empty():
        f, currCostFromStart, currMove, currPos = q.get()
        if currPos in visitedPos:
            continue
        else:
            visitedPos.add(currPos)
        
        if (currPos == goal):
            return currMove
        
        for nextPos in successor(currPos, scene):
            if nextPos not in visitedPos:
                nextMove = currMove + [nextPos]
                nextCostFromStart = currCostFromStart + sled_calculator(currPos, nextPos)
                totalCost = nextCostFromStart + sled_calculator(nextPos, goal)
                q.put((totalCost, nextCostFromStart, nextMove, nextPos))
    return None

############################################################
# Section 3: Linear Disk Movement, Revisited
############################################################
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
    
def LD_successor(board):
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

def huristic_func(currBoard, targetBoard):
    # use the distance of each disk from its final resting position as the huristic function
    h = 0 
    for i in currBoard:
        if i != 0:
            h = h + abs(currBoard.index(i) - targetBoard.index(i))
    return h

def solve_distinct_disks(length, n):
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
    if(length <= n or n < 0 or length <= 0):
        return None
    startBoard = D_createNewBoard(length, n)
    targetBoard = startBoard[::-1]
    
    q = PriorityQueue()
    visitedBoard = set()
    movePath = []
    
    q.put((huristic_func(startBoard, targetBoard), movePath, startBoard))
    
    while not q.empty():
        f, currMove, currBoard = q.get()
        if tuple(currBoard) in visitedBoard:
            continue
        else:
            visitedBoard.add(tuple(currBoard))
        if LD_is_solved(currBoard, targetBoard):
            return currMove    
        
        for move, nextBoard in LD_successor(currBoard):
            #print(".......................................")
            #print("current board = ", currBoard)
            #print("current f =", f)
            #print(move, nextBoard)
            if (tuple(nextBoard) not in visitedBoard):
                nextMove = currMove + [move]
                q.put((len(nextMove) + huristic_func(nextBoard, targetBoard), nextMove, nextBoard))
            #print("len of move = ", len(nextMove), "next f = ", len(nextBoard) + huristic_func(nextBoard, targetBoard))
            #print(".................................")

    return None

############################################################
# Section 4: Dominoes Game
############################################################

def create_dominoes_game(rows, cols):
    board = [[False for j in range(cols)] for i in range(rows)]
    return DominoesGame(board)
class DominoesGame(object):
    '''
    >>> b = [[False, False], [False, False]]
    >>> g = DominoesGame(b)
    >>> g.get_board()
    [[False, False], [False, False]]
    >>> b = [[True, False], [True, False]]
    >>> g = DominoesGame(b)
    >>> g.get_board()
    [[True, False], [True, False]]
    >>> g = create_dominoes_game(2, 2)
    >>> g.get_board()
    [[False, False], [False, False]]
    >>> g = create_dominoes_game(2, 3)
    >>> g.get_board()
    [[False, False, False], [False, False, False]]
    >>> b = [[False, False], [False, False]]
    >>> g = DominoesGame(b)
    >>> g.get_board()
    [[False, False], [False, False]]
    >>> g.reset()
    >>> g.get_board()
    [[False, False], [False, False]]
    >>> b = [[True, False], [True, False]]
    >>> g = DominoesGame(b)
    >>> g.get_board()
    [[True, False], [True, False]]
    >>> g.reset()
    >>> g.get_board()
    [[False, False], [False, False]]
    >>> b = [[False, False], [False, False]]
    >>> g = DominoesGame(b)
    >>> g.is_legal_move(0, 0, True)
    True
    >>> g.is_legal_move(0, 0, False)
    True
    >>> b = [[True, False], [True, False]]
    >>> g = DominoesGame(b)
    >>> g.is_legal_move(0, 0, False)
    False
    >>> g.is_legal_move(0, 1, True)
    True
    >>> g.is_legal_move(1, 1, True)
    False
    >>> g = create_dominoes_game(3, 3)
    >>> list(g.legal_moves(True))
    [(0, 0), (0, 1), (0, 2), (1, 0), (1, 1), (1, 2)]
    >>> list(g.legal_moves(False))
    [(0, 0), (0, 1), (1, 0), (1, 1), (2, 0), (2, 1)]
    >>> b = [[True, False], [True, False]]
    >>> g = DominoesGame(b)
    >>> list(g.legal_moves(True))
    [(0, 1)]
    >>> list(g.legal_moves(False))
    []
    >>> g = create_dominoes_game(3, 3)
    >>> g.perform_move(0, 1, True)
    >>> g.get_board()
    [[False, True, False], [False, True, False], [False, False, False]]
    >>> g = create_dominoes_game(3, 3)
    >>> g.perform_move(1, 0, False)
    >>> g.get_board()
    [[False, False, False], [True, True, False], [False, False, False]]
    >>> b = [[False, False], [False, False]]
    >>> g = DominoesGame(b)
    >>> g.game_over(True)
    False
    >>> g.game_over(False)
    False
    >>> b = [[True, False], [True, False]]
    >>> g = DominoesGame(b)
    >>> g.game_over(True)
    False
    >>> g.game_over(False)
    True
    >>> g = create_dominoes_game(4, 4)
    >>> g2 = g.copy()
    >>> g.get_board() == g2.get_board()
    True
    >>> g = create_dominoes_game(4, 4)
    >>> g2 = g.copy()
    >>> g.perform_move(0, 0, True)
    >>> g.get_board() == g2.get_board()
    False
    >>> b = [[False, False], [False, False]]
    >>> g = DominoesGame(b)
    >>> for m, new_g in g.successors(True):
    ...     print(m, new_g.get_board())
    (0, 0) [[True, False], [True, False]]
    (0, 1) [[False, True], [False, True]]
    >>> b = [[True, False], [True, False]]
    >>> g = DominoesGame(b)
    >>> for m, new_g in g.successors(True):
    ...     print(m, new_g.get_board())
    (0, 1) [[True, True], [True, True]]
    >>> b = [[False] * 3 for i in range(3)]
    >>> g = DominoesGame(b)
    >>> g.get_best_move(True, 1)
    ((0, 1), 2, 6)
    >>> g.get_best_move(True, 2)
    ((0, 1), 3, 10)
    >>> b = [[False] * 3 for i in range(3)]
    >>> g = DominoesGame(b)
    >>> g.perform_move(0, 1, True)
    >>> g.get_best_move(False, 1)
    ((2, 0), -3, 2)
    >>> g.get_best_move(False, 2)
    ((2, 0), -2, 5)
    '''
    # Required
    def __init__(self, board):
        self.board = board
        self.numRow = len(board)
        self.numCol = len(board[0])
        

    def get_board(self):
        return self.board

    def reset(self):
        self.board = [[False for j in range(self.numCol)] for i in range(self.numRow)]

    def is_legal_move(self, row, col, vertical):
        # check if out of bound and current location is valid or not
        if (row < 0 or row >= self.numRow or col < 0 or col >= self.numCol or self.board[row][col] == True):
            return False
        
        # check the vertical or horizontal
        if (vertical == True):
            if (row + 1 >= self.numRow or self.board[row + 1][col] == True):
                return False
        else:
            if (col + 1 >= self.numCol or self.board[row][col + 1] == True):
                return False
        return True

    def legal_moves(self, vertical):
        # loop through each position to check the validation
        for i in range(self.numRow):
            for j in range(self.numCol):
                if self.is_legal_move(i, j, vertical):
                    yield i, j

    def perform_move(self, row, col, vertical):
        # check if the input is valid or not
        if self.is_legal_move(row, col, vertical) == False:
            return None
        
        # perform the move based on vertical 
        self.board[row][col] = True
        if vertical == True:
            self.board[row + 1 ][col] = True
        else:
            self.board[row][col + 1] = True

    def game_over(self, vertical):
        if len(list(self.legal_moves(vertical))) == 0:
            return True
        return False

    def copy(self):
        return copy.deepcopy(self)

    def successors(self, vertical):
        for move in self.legal_moves(vertical):
            tempBoard = self.copy()
            tempBoard.perform_move(move[0], move[1], vertical)
            yield move, tempBoard

    def get_random_move(self, vertical):
        return random.choice(list(self.legal_moves(vertical)))



    # Required
    def evaluated_utility(self, currPlayerMoves, oppoPlayerMoves):
        return len(currPlayerMoves) - len(oppoPlayerMoves)
    
    def max_value(self, currMove, alpha, beta, limit, vertical):
        currPlayerMoves = list(self.successors(vertical))
        oppoPlayerMoves = list(self.successors(not vertical))
        utility = self.evaluated_utility(currPlayerMoves, oppoPlayerMoves)
        # Terminal state
        if limit == 0 or self.game_over(vertical):
            return currMove, utility, 1
        
        # set value
        maxMove, maxV, maxLeafNode = currMove, -math.inf, 0
        for move, minBoard in currPlayerMoves:
            # v <-- max(v, min_value(result(s,a), alpha, beta))
            minMove, minV, minLeafNode = minBoard.min_value(move, alpha, beta, limit - 1, not vertical)
            maxLeafNode = maxLeafNode + minLeafNode
            if (minV > maxV):
                maxV = minV
                maxMove = move
            
            # if v >= beta return v
            if maxV >= beta:
                return maxMove, maxV, maxLeafNode
            # alpha <- max(alpha, v)
            alpha = max(alpha, maxV)
        return maxMove, maxV, maxLeafNode
        
        
        
    def min_value(self, currMove, alpha, beta, limit, vertical):
        # note that in the max_value, the parameter passed in is already "not vertical"
        currPlayerMoves = list(self.successors(vertical))
        oppoPlayerMoves = list(self.successors(not vertical))
        # note: we calculate the utility base on the player that want to make a move
        # here the player who want to make a move is on the oppoPlayer side
        utility = self.evaluated_utility(oppoPlayerMoves, currPlayerMoves)
        # Terminal State
        if limit == 0 or self.game_over(vertical):
            return currMove, utility, 1
        
        # Set Value
        minMove, minV, minLeafNode = currMove, math.inf, 0
        for move, maxBoard in currPlayerMoves:
             # v <-- min(v, max_value(result(s,a), alpha, beta))
            maxMove, maxV, maxLeafNode = maxBoard.max_value(move, alpha, beta, limit - 1, not vertical)
            minLeafNode = minLeafNode + maxLeafNode
            if maxV < minV:
                minV = maxV
                minMove = move
            # if v <= alpha then return v
            if minV <= alpha:
                return minMove, minV, minLeafNode
            # beta <- min(beta, v)
            beta = min(beta, minV)
        return minMove, minV, minLeafNode
              
    def get_best_move(self, vertical, limit):
        alpha, beta = -math.inf, math.inf
        return self.max_value((), alpha, beta, limit, vertical)
        

if __name__ == "__main__":
    import doctest
    doctest.testmod()