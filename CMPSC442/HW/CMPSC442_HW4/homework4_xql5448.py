############################################################
# CMPSC 442: Logic
############################################################

student_name = "Xuhong Lin"

############################################################
# Imports
############################################################

# Include your imports here, if any are used.
import copy


############################################################
# Section 1: Propositional Logic
############################################################
class Expr(object):
    def __hash__(self):
        return hash((type(self).__name__, self.hashable))

class Atom(Expr):
    '''
    >>> Atom("a") == Atom("a")
    True 
    >>> Atom("a") == Atom("b")
    False 
    >>> Atom("a").atom_names()
    set(['a'])
    >>> Atom("a").to_cnf()
    Atom(a)
    '''
    def __init__(self, name):
        self.name = name
        self.hashable = name
        
    def __hash__(self):
        return hash(self.hashable)
    
    def __eq__(self, other):
        # isinstance function to check other expression is the same class type
        return isinstance(other, Atom) and self.name == other.name
    
    def __repr__(self):
        return f"Atom({self.name})"
    
    def atom_names(self):
        return set([self.name])
    
    def evaluate(self, assignment):
        return assignment[self.name]
    
    def to_cnf(self):
        return self

class Not(Expr):
    '''
    >>> a = Atom("a")
    >>> Not(Or(Not(a)))
    Not(Or(Not(Atom(a))))
    >>> Not(Atom("a")).atom_names()
    set(['a'])
    >>> Not(And(Atom("a"), Atom("b"))).to_cnf()
    Or(Not(Atom(a)), Not(Atom(b)))
    >>> Not(Or(Atom("a"), Atom("b"))).to_cnf()
    And(Not(Atom(a)), Not(Atom(b)))  
    >>> a, b, c, d = map(Atom, "abcd")  
    >>> Not(Or(And(a, b), And(c, d))).to_cnf()
    And(Or(Not(Atom(a)), Not(Atom(b))), Or(Not(Atom(c)), Not(Atom(d))))
    >>> Not(Implies(a, b)).to_cnf()
    And(Atom(a), Not(Atom(b)))
    >>> Not(Iff(a, b)).to_cnf()
    And(Or(Atom(a), Atom(b)), Or(Atom(a), Not(Atom(a))), Or(Not(Atom(a)), Not(Atom(b))), Or(Atom(b), Not(Atom(b))))
    '''
    def __init__(self, arg):
        self.arg = arg
        self.hashable = arg
        
    def __hash__(self):
        return hash(self.hashable)
    
    def __eq__(self, other):
        return isinstance(other, Not) and self.arg == other.arg
        
    def __repr__(self):
        return f"Not({self.arg})"
    
    def atom_names(self):
        return self.arg.atom_names()
    
    def evaluate(self, assignment):
        return not self.arg.evaluate(assignment)
    
    def to_cnf(self):
        if isinstance(self.arg, Atom):
            return self
        elif isinstance(self.arg, Not):
            return self.arg.arg.to_cnf()
        elif isinstance(self.arg, And):
            # De Morgen ~(a ^ b) = ~a v ~b
            return Or(*(Not(conjunct.to_cnf()).to_cnf() for conjunct in self.arg.conjuncts)).to_cnf()
        elif isinstance(self.arg, Or):
            # De Morgen ~(a V b) = ~a ^ ~b
            return And(*(Not(disjunct.to_cnf()).to_cnf() for disjunct in self.arg.disjuncts)).to_cnf()
        else:
            return Not(self.arg.to_cnf()).to_cnf()
        
        
class And(Expr):
    '''
    >>> And(Atom("a"), Not(Atom("b"))) == And(Not(Atom("b")), Atom("a"))
    True
    >>> a, b, c = map(Atom, "abc")
    >>> And(a, Or(Not(b), c))
    And(Atom(a), Or(Not(Atom(b)), Atom(c)))
    >>> a, b, c = map(Atom, "abc")
    >>> expr = And(a, Implies(b, Iff(a, c)))
    >>> expr.atom_names()
    set(['a', 'c', 'b'])
    >>> expr = And(Or(a, b, c), And(a, Or(b, c)))
    >>> expr.to_cnf()
    And(Or(Atom(a), Atom(b), Atom(c)), Atom(a), Or(Atom(b), Atom(c)))
    '''
    def __init__(self, *conjuncts):
        self.conjuncts = frozenset(conjuncts)
        self.hashable = self.conjuncts
        
    def __hash__(self):
        return hash(self.hashable)
    
    def __eq__(self, other):
        return isinstance(other, And) and self.conjuncts == other.conjuncts
        
    def __repr__(self):
        return f"And({', '.join(map(repr, self.conjuncts))})"
    
    def atom_names(self):
        # we need to loop through all arguements in the conjuncts
        args = set()
        for arg in self.conjuncts:
            args |= arg.atom_names()
        return args
    
    def evaluate(self, assignment):
        # check for all conjuncts and if any is false then false, otherwise ture
        for arg in self.conjuncts:
            if arg.evaluate(assignment) == False:
                return False
        return True
    
    def to_cnf(self):
        # Distrivbet And over or/Atom
        # '*' is theunpacked operator, it unpacks the elemetns of the iterable
        # into individual arguments 
        clauses = []
        conjuncts_in_list = [conj.to_cnf() for conj in self.conjuncts] #make sure all clauses in CNF
        for conjunct in conjuncts_in_list:
            if isinstance(conjunct, And):
                clauses.extend(conjunct.conjuncts)
            else:
                clauses.append(conjunct)
        return And(*(clauses))

class Or(Expr):
    '''
    >>> a, b, c = map(Atom, "abc")
    >>> Or(a, And(Not(b), c), c)
    Or(Atom(a), And(Not(Atom(b)), Atom(c)), Atom(c))
    >>> Or(Atom("a"), Atom("b")).to_cnf()
    Or(Atom(b), Atom(a)
    >>> a, b, c, d, e, f = map(Atom, "abcdef")
    >>> Or(And(a, b), And(c, d)).to_cnf()
    And(Or(Atom(d), Atom(a)), Or(Atom(a), Atom(c)), Or(Atom(b), Atom(c)), Or(Atom(b), Atom(d)))
    >>> Or(And(a, b), And(c,d), And(e,f)).to_cnf()
    And(Or(Atom(a), Atom(c), Atom(e)), Or(Atom(a), Atom(c), Atom(f)), Or(Atom(a), Atom(d), Atom(e)), Or(Atom(a), Atom(d), Atom(f)), Or(Atom(b), Atom(c), Atom(e)), Or(Atom(b), Atom(c), Atom(f)), Or(Atom(b), Atom(d), Atom(e)), Or(Atom(b), Atom(d), Atom(f)))
    '''
    def __init__(self, *disjuncts):
        self.disjuncts = frozenset(disjuncts)
        self.hashable = self.disjuncts
        
    def __hash__(self):
        return hash(self.hashable)
    
    def __eq__(self, other):
        return isinstance(other, Or) and self.disjuncts == other.disjuncts
        
    def __repr__(self):
        return f"Or({', '.join(map(repr, self.disjuncts))})"
    
    def atom_names(self):
        # we need to loop through all arguements in the conjuncts
        args = set()
        for arg in self.disjuncts:
            args |= arg.atom_names()
        return args
    
    def evaluate(self, assignment):
        # check for all disjuncts and if any is True then True, otherwise False
        for arg in self.disjuncts:
            if arg.evaluate(assignment) == True:
                return True
        return False
    
    def to_cnf(self):
        clauses = []
        disjuncts_in_list = [disj.to_cnf() for disj in self.disjuncts] # make all clauses in cnf
        for disjunct in disjuncts_in_list:
            if isinstance(disjunct, Atom):
                clauses.append(disjunct)
            elif isinstance(disjunct, Or):
                clauses.extend(disjunct.disjuncts)
            elif isinstance(disjunct, And):
                # Distribute Or over And
                clauses = []
                distributes = []
                conjuncts_in_list = [conj.to_cnf() for conj in disjunct.conjuncts] # make sure clauses in And are in CNF
                # disjunct is an And, attribute all elements in And over all other Or elements
                for conjunct in conjuncts_in_list:
                    # pop out the And clause
                    new_disjuncts = [d for d in disjuncts_in_list if d != disjunct]
                    # Add one of its element back
                    new_disjuncts.append(conjunct)
                    distributes.append(Or(*(new_disjuncts)))
                clauses.extend(distributes)
                break
            else:
                clauses.append(disjunct)
                
            #print(clauses)
            
        return And(*(clause for clause in clauses)).to_cnf() if any(isinstance(disj, And) for disj in self.disjuncts) else Or(*(clause.to_cnf() for clause in clauses))
                    
                
class Implies(Expr):
    '''
    >>> a, b, c, d, e, f= map(Atom, "abcdef")
    >>> Implies(a, Iff(b, c))
    Implies(Atom(a), Iff(Atom(b), Atom(c)))
    >>> Implies(a, b).to_cnf()
    Or(Not(Atom(a), Atom(b)))
    >>> Implies(And(a, Or(b, c)), And(d, e)).to_cnf()
    And(Or(Not(Atom(a)), Not(Atom(b)), Atom(d)), Or(Not(Atom(a)), Not(Atom(b)), Atom(e)), Or(Not(Atom(a)), Not(Atom(c)), Atom(d)), Or(Not(Atom(a)), Not(Atom(c)), Atom(e)))
    >>> Implies(Or(And(a, b), And(c, d)), Or(e, f)).to_cnf()
    And(Or(Not(Atom(a)), Not(Atom(b)), Atom(e), Atom(f)), Or(Not(Atom(c)), Not(Atom(d)), Atom(e), Atom(f)))
    '''
    def __init__(self, left, right):
        self.left = left
        self.right = right
        self.hashable = (left, right)
        
    def __hash__(self):
        return hash(self.hashable)
    
    def __eq__(self, other):
        return isinstance(other, Implies) and (self.left == other.left and self.right == other.right)
    
    def __repr__(self):
        return f"Implies({self.left}, {self.right})"
        
    def atom_names(self):
        return self.left.atom_names() | self.right.atom_names()
    
    def evaluate(self, assignment):
        #True Table for Implies
        # a    b    out
        # T    T     T
        # T    F     F
        # F    T     T
        # F    F     T
        if(self.left.evaluate(assignment) == True and self.right.evaluate(assignment) == False):
            return False
        else:
            return True
    
    def to_cnf(self):
        # Convert A -> B to !A v B
         return Or(Not(self.left).to_cnf(), self.right.to_cnf()).to_cnf()

class Iff(Expr):
    '''
    >>> a, b, c = map(Atom, "abc")
    >>> Implies(And(Not(a), b, c), Not(Or(a, b, c)))
    Implies(And(Not(Atom(a)), Atom(b), Atom(c)), Not(Or(Atom(a), Atom(b), Atom(c))))
    >>> a, b, c = map(Atom, "abc")
    >>> Iff(a, Or(b, c)).to_cnf()
    And(Or(Atom(b), Atom(c), Not(Atom(a))), Or(Not(Atom(b)), Atom(a)), Or(Not(Atom(c)), Atom(a)))
    >>> Iff(a, b).to_cnf()
    And(Or(Not(Atom(a)), Atom(b)), Or(Not(Atom(b)), Atom(a)))
    '''
    def __init__(self, left, right):
        self.left = left
        self.right = right
        self.hashable = (left, right)
        
    def __hash__(self):
        return hash(self.hashable)
    
    def __eq__(self, other):
        return isinstance(other, Iff) and (self.left == other.left and self.right == other.right) or (self.left == other.right and self.right == other.left)
    
    def __repr__(self):
        return f"Iff({self.left}, {self.right})"
        
    def atom_names(self):
        return self.left.atom_names() | self.right.atom_names()
    
    def evaluate(self, assignment):
        return self.left.evaluate(assignment) == self.right.evaluate(assignment)
    
    def to_cnf(self):
        # Convert A <=> B to (A -> B) ^ (B -> A)
        return And(Implies(self.left, self.right).to_cnf(), Implies(self.right, self.left).to_cnf()).to_cnf()

def satisfying_helper(atom_names, expr, assignment):
    # Terminal State, if set is empty
    if len(atom_names) == 0:
        if expr.evaluate(assignment) == True:
            yield assignment.copy() #yield a copy, reference of assignment will be empty set eventually
    else:      
        # Recursive
        # pop a random element from set
        poped_name = atom_names.pop()
        # since each logic could have True or False
        for v in [True, False]:
            assignment[poped_name] = v
            yield from satisfying_helper(atom_names, expr, assignment)
            # backtracking
            assignment.pop(poped_name)
        atom_names.add(poped_name) # Add poped atom_name back for following backtracing
        
def satisfying_assignments(expr):
    '''
    >>> e = Implies(Atom("a"), Atom("b"))
    >>> a = satisfying_assignments(e)
    >>> next(a)
    {'a': False, 'b': False}
    >>> next(a)
    {'a': False, 'b': True}
    >>> next(a)
    {'a': True, 'b': True}
    >>> e = Iff(Iff(Atom("a"), Atom("b")), Atom("c"))
    >>> list(satisfying_assignments(e))
    [{'a': False, 'c': False, 'b': True},
    {'a': False, 'c': True, 'b': False},
    {'a': True, 'c': False, 'b': False},
    {'a': True, 'c': True, 'b': True}]
    '''
    atom_names = expr.atom_names()
    return satisfying_helper(atom_names, expr, {})

class KnowledgeBase(object):
    '''
    >>> a, b, c = map(Atom, "abc")
    >>> kb = KnowledgeBase()
    >>> kb.tell(a)
    >>> kb.tell(Implies(a, b))
    >>> kb.get_facts()
    set([Or(Atom(b), Not(Atom(a))),
    Atom(a)])
    >>> [kb.ask(x) for x in (a, b, c)]
    [True, True, False]
    >>> a, b, c = map(Atom, "abc")
    >>> kb = KnowledgeBase()
    >>> kb.tell(Iff(a, Or(b, c)))
    >>> kb.tell(Not(a))
    >>> [kb.ask(x) for x in (a, Not(a))]
    [False, True]
    >>> [kb.ask(x) for x in (b, Not(b))]
    [False, True]
    >>> [kb.ask(x) for x in (c, Not(c))]
    [False, True]
    
    '''
    def __init__(self):
        self.facts = set()
    
    def get_facts(self):
        return self.facts
    
    def tell(self, expr):
        expr_cnf = expr.to_cnf()
        # In resolution algorithm, each fact is considered independently
        # as each conjunct individually might be better vasulization
        if isinstance(expr_cnf, And):
            self.facts |= expr_cnf.conjuncts
        else:
            self.facts.add(expr.to_cnf())
    
    def ask(self, expr):
        
        #print("expr = ", expr)
        
        expr_cnf = expr.to_cnf()
        negated_expr = Not(expr_cnf).to_cnf()
        # The set of clauses in the CNF representation of KB ^ ~a
        clauses = copy.deepcopy(self.facts)
        clauses.add(negated_expr)
        paired_clauses = set()
        # loop do
        while True:
            clauses_in_list = list(clauses)
            
            #print("clauses_in_list", clauses_in_list)
            
            # new <-- {}
            new_fact = set()
            # for each pair of clauses Ci, Cj in clauses do
            for i in range(len(clauses_in_list)):
                for j in range(i+1, len(clauses_in_list)):
                    if (clauses_in_list[i], clauses_in_list[j]) in paired_clauses or (clauses_in_list[j], clauses_in_list[i]) in paired_clauses:
                        continue
                    else:
                        #print("clause i =", clauses_in_list[i], "clause j = ", clauses_in_list[j])
                        
                        # resolvents <-- PL-RESOLVE(Ci, Cj)   
                        resolvents = self.PL_Resolve(clauses_in_list[i], clauses_in_list[j])
                        
                        #print("resolvents = ", resolvents)
                        
                        #if resolvents contains the empty clause then return true
                        if frozenset() in resolvents:
                            return True
                        # new <-- new U resolvents
                        new_fact |= resolvents
                        paired_clauses.add((clauses_in_list[i], clauses_in_list[j]))
                        #print("new fact = ", new_fact)
                    
            # if new belongs to clauses then return false
            if new_fact.issubset(clauses):
                return False
            # clauses <-- clauses U new
            clauses |= new_fact


    def PL_Resolve(self, Ci, Cj):
        resolvents = set()
        Ci_fact_list = self.Get_PL_List(Ci)
        Cj_fact_list = self.Get_PL_List(Cj)
        
        #print("Ci_fact_list = ", Ci_fact_list, "Cj_fact_list = ", Cj_fact_list)
        
        for fact_i in Ci_fact_list:
            for fact_j in Cj_fact_list:
                if (fact_i == Not(fact_j) or Not(fact_i) == fact_j):
                    # resolve two facts
                    new_Ci_fact_list = copy.deepcopy(Ci_fact_list)
                    new_Cj_fact_list = copy.deepcopy(Cj_fact_list)
                    new_Ci_fact_list.remove(fact_i)
                    new_Cj_fact_list.remove(fact_j)
                    #new_fact_list = list(set(new_Ci_fact_list) | set(new_Cj_fact_list))
                    new_fact_list = new_Ci_fact_list + new_Cj_fact_list
                    
                    #print("new_Ci_fact_list = ", new_Ci_fact_list, "new_Cj_fact_list = ", new_Cj_fact_list)
                    #print("new_fact_list = ", new_fact_list)
                    
                    # check the length of the new fact list
                    if len(new_fact_list) == 0:
                        # empty clause happened
                        resolvents.add(frozenset())
                    elif len(new_fact_list) == 1:
                        # Atom or Not(Atom)
                        resolvents.add(new_fact_list[0])
                    elif len(new_fact_list) >= 2:
                        # Or(fact1, fact2, ...)
                        resolvents.add(Or(*(new_fact_list)))
                    
        return resolvents
                        
    def Get_PL_List(self, clauses):
    # since all clauses are in CNF
    # we can make sure that there are only three types of clauses
    # 1. Or 2.Atom 3.Not of Atom
        list = []
        if isinstance(clauses, Atom):
            list.append(clauses)
        elif isinstance(clauses, Not):
            list.append(clauses)
        elif isinstance(clauses, Or):
            for disj in clauses.disjuncts:
                list.append(disj)
        return list
    
############################################################
# Section 2: Logic Puzzles
############################################################

# Puzzle 1  score: 5/5

# Populate the knowledge base using statements of the form kb1.tell(...)
kb1 = KnowledgeBase()
kb1.tell(Implies(Atom("mythical"), Not(Atom("mortal"))))
kb1.tell(Implies(Not(Atom("mythical")), And(Atom("mortal"), Atom("mammal"))))
kb1.tell(Implies(Or(Not(Atom("mortal")), Atom("mammal")), Atom("horned")))
kb1.tell(Implies(Atom("horned"), Atom("magical")))


# Write an Expr for each query that should be asked of the knowledge base
mythical_query = Atom("mythical")
magical_query = Atom("magical")
horned_query = Atom("horned")

#is_mythical = kb1.ask(mythical_query)
#is_magical = kb1.ask(magical_query)
#is_horned = kb1.ask(horned_query)
#print(is_mythical, is_magical, is_horned)

# Record your answers as True or False; if you wish to use the above queries,
# they should not be run when this file is loaded
is_mythical = False
is_magical = True
is_horned = True
# Puzzle 2  score: 5/5

# Write an Expr of the form And(...) encoding the constraints
party_constraints = And(Implies(Or(Atom("m"), Atom("a")), Atom("j")), Implies(Not(Atom("m")), Atom("a")), Implies(Atom("a"), Not(Atom("j"))))

# Compute a list of the valid attendance scenarios using a call to
# satisfying_assignments(expr)
valid_scenarios = list(satisfying_assignments(party_constraints))

# Write your answer to the question in the assignment
puzzle_2_question = """
{'j': True, 'm': True, 'a': False}
John and Mary will come, but Ann does not come
"""

# Puzzle 3
# Populate the knowledge base using statements of the form kb3.tell(...)
kb3 = KnowledgeBase()
kb3.tell(And(Atom("e1"), Atom("p2")))
kb3.tell(And(Not(Atom("s1")), Atom("s2")))
kb3.tell(Iff(Atom("p1"), Not(Atom("e1"))))
kb3.tell(Iff(Atom("p2"), Not(Atom("e2"))))

'''
s1_query = Atom("s1")
s2_query = Atom("s2")
s1_s2_query = And(Atom("s1"), Not(Atom("s2")))
is_s1 = kb3.ask(s1_query)
is_s2 = kb3.ask(s2_query)
is_s1_s2 = kb3.ask(s1_s2_query)
p1 = kb3.ask(Atom("p1"))
e1 = kb3.ask(Atom("e1"))
p2 = kb3.ask(Atom("p2"))
e2 = kb3.ask(Atom("e2"))
print(is_s1, is_s2, is_s1_s2, p1, e1, p2, e2)
'''

# Write your answer to the question in the assignment; the queries you make
# should not be run when this file is loaded
puzzle_3_question = """
sign 2 is true, Room 1 is empty, and room 2 has prize
"""

# Puzzle 4

# Populate the knowledge base using statements of the form kb4.tell(...)
kb4 = KnowledgeBase()
kb4.tell(Implies(Atom("ia"), And(Atom("kb"), Not(Atom("kc")))))
kb4.tell(Implies(Atom("ib"), Not(Atom("kb"))))
kb4.tell(Implies(Atom("ic"), And(And(Atom("ka"), Atom("kb")), Or(And(Atom("ia"), Not(Atom("ib"))), And(Not(Atom("ia")), Atom("ib"))))))
#kb4.tell(Or(And(Not(Atom("ia")), Atom("ib"), Atom("ic")), And(Atom("ia"), Not(Atom("ib")), Atom("ic")), And(Atom("ia"), Atom("ib"), Not(Atom("ic")))))
kb4.tell(And(Atom("ia"), Not(Atom("ib")), Atom("ic")))

# Uncomment the line corresponding to the guilty suspect
# guilty_suspect = "Adams"
guilty_suspect = "Brown"
# guilty_suspect = "Clark"
# which_guilty = kb4.ask(Atom(guilty_suspect))

# Describe the queries you made to ascertain your findings
puzzle_4_question = """
Brown is guilty
"""






if __name__ == "__main__":
    import doctest
    doctest.testmod()