# HW3
# Due Date: 10/15/2021, 11:59PM
# REMINDER: The work in this assignment must be your own original work and must be completed alone.


class Node:
    def __init__(self, value):
        self.value = value
        self.next = None
    
    def __str__(self):
        return "Node({})".format(self.value)

    __repr__ = __str__
                          

#=============================================== Part I ==============================================

class Stack:
    '''
        >>> x=Stack()
        >>> x.pop()
        >>> x.push(2)
        >>> x.push(4)
        >>> x.push(6)
        >>> x
        Top:Node(6)
        Stack:
        6
        4
        2
        >>> x.pop()
        6
        >>> x
        Top:Node(4)
        Stack:
        4
        2
        >>> len(x)
        2
        >>> x.peek()
        4
    '''
    def __init__(self):
        self.top = None
    
    def __str__(self):
        temp=self.top
        out=[]
        while temp:
            out.append(str(temp.value))
            temp=temp.next
        out='\n'.join(out)
        return ('Top:{}\nStack:\n{}'.format(self.top,out))

    __repr__=__str__


    def isEmpty(self):
        # YOUR CODE STARTS HERE
        return self.top == None

    def __len__(self):
        # YOUR CODE STARTS HERE
        if self.isEmpty():
            return 0
        else:
            current = self.top
            l = 1
            while current.next is not None:
                current = current.next
                l += 1
            return l
            

    def push(self,value):
        # YOUR CODE STARTS HERE
        n = Node(value)
        if self.isEmpty():
            self.top = n
        else:
            n.next = self.top
            self.top = n

     
    def pop(self):
        # YOUR CODE STARTS HERE
        if not self.isEmpty():
            k = self.top.next
            t = self.top.value
            self.top.next = None
            self.top = k
            return t

    def peek(self):
        # YOUR CODE STARTS HERE
        if not self.isEmpty():
            return self.top.value
            



#=============================================== Part II ==============================================

class Calculator:
    def __init__(self):
        self.__expr = None


    @property
    def getExpr(self):
        return self.__expr

    def setExpr(self, new_expr):
        if isinstance(new_expr, str):
            self.__expr=new_expr
        else:
            print('setExpr error: Invalid expression')
            return None

    def _isNumber(self, txt):
        '''
            >>> x=Calculator()
            >>> x._isNumber(' 2.560 ')
            True
            >>> x._isNumber('7 56')
            False
            >>> x._isNumber('2.56p')
            False
        '''
        # YOUR CODE STARTS HERE
        try:
            float(txt)
            return True
        except ValueError:
            return False




    def _getPostfix(self, txt):
        '''
            Required: _getPostfix must create and use a Stack for expression processing
            >>> x=Calculator()
            >>> x._getPostfix('2 ^ 4')
            '2.0 4.0 ^'
            >>> x._getPostfix('2')
            '2.0'
            >>> x._getPostfix('2.1 * 5 + 3 ^ 2 + 1 + 4.45')
            '2.1 5.0 * 3.0 2.0 ^ + 1.0 + 4.45 +'
            >>> x._getPostfix('2 * 5.34 + 3 ^ 2 + 1 + 4')
            '2.0 5.34 * 3.0 2.0 ^ + 1.0 + 4.0 +'
            >>> x._getPostfix('2.1 * 5 + 3 ^ 2 + 1 + 4')
            '2.1 5.0 * 3.0 2.0 ^ + 1.0 + 4.0 +'
            >>> x._getPostfix('( 2.5 )')
            '2.5'
            >>> x._getPostfix ('( ( 2 ) )')
            '2.0'
            >>> x._getPostfix ('2 * ( ( 5 + -3 ) ^ 2 + ( 1 + 4 ) )')
            '2.0 5.0 -3.0 + 2.0 ^ 1.0 4.0 + + *'
            >>> x._getPostfix ('( 2 * ( ( 5 + 3 ) ^ 2 + ( 1 + 4 ) ) )')
            '2.0 5.0 3.0 + 2.0 ^ 1.0 4.0 + + *'
            >>> x._getPostfix ('( ( 2 * ( ( 5 + 3 ) ^ 2 + ( 1 + 4 ) ) ) )')
            '2.0 5.0 3.0 + 2.0 ^ 1.0 4.0 + + *'
            >>> x._getPostfix('2 * ( -5 + 3 ) ^ 2 + ( 1 + 4 )')
            '2.0 -5.0 3.0 + 2.0 ^ * 1.0 4.0 + +'
            >>> x._getPostfix('7 ^ 2 ^ 3')
            '7.0 2.0 3.0 ^ ^'
            >>> x._getPostfix('4 + 3 - 2')
            '4.0 3.0 + 2.0 -'
            
            
            # In invalid expressions, you might print an error message, adjust doctest accordingly
            # If you are veryfing the expression in calculate before passing to postfix, this cases are not necessary

            >>> x._getPostfix('2 * 5 + 3 ^ + -2 + 1 + 4')
            'invalid expresstion'
            >>> x._getPostfix('2 * 5 + 3 ^ - 2 + 1 + 4')
            'invalid expresstion'
            >>> x._getPostfix('2    5')
            'invalid expresstion'
            >>> x._getPostfix('25 +')
            'invalid expresstion'
            >>> x._getPostfix(' 2 * ( 5 + 3 ) ^ 2 + ( 1 + 4 ')
            'invalid expresstion'
            >>> x._getPostfix(' 2 * ( 5 + 3 ) ^ 2 + ) 1 + 4 (')
            'invalid expresstion'
            >>> x._getPostfix('2 * 5% + 3 ^ + -2 + 1 + 4')
            'invalid expresstion'
        '''

        # YOUR CODE STARTS HERE
        if self.isvalid(txt) == False:
            return 'invalid expresstion'
        postfixStack = Stack()  # method must use postfixStack to compute the postfix expression
        priority = {'+':1, '-':1, '*':2, '/':2, '^':3}
        operators = ['+', '-', '*', '/', '^']
        expr = txt.split()
        output = []
        for i in expr:
            if i == '(':
                postfixStack.push(i)
            elif i == ')':
                while postfixStack.top.value != '(':
                    output.append(postfixStack.pop())
                postfixStack.pop()
            elif i not in operators:
                output.append(str(float(i)))
            elif i in operators and i != '^':
                while not postfixStack.isEmpty() and postfixStack.top.value != '(' and priority[i] <= priority[postfixStack.top.value]:
                    output.append(postfixStack.pop())
                postfixStack.push(i)
            elif i in operators and i == '^':
                while not postfixStack.isEmpty() and postfixStack.top.value != '(' and priority[i] < priority[postfixStack.top.value]:
                    output.append(postfixStack.pop())
                postfixStack.push(i)
        while not postfixStack.isEmpty():
            output.append(postfixStack.pop())
        return ' '.join(output)
        
            
    def isvalid(self, txt):
        operators = ['+', '-', '*', '/', '^']
        temp = txt.split()
        if temp[0] != '(' and self._isNumber(temp[0]) == False:
            return False
        elif temp[-1] in operators or temp[-1] == '(':
            return False
        else:
            for i in temp:
                if i not in operators and self._isNumber(i) == False and i != '(' and i != ')':
                    return False
            for i in range(len(temp)-1):
                if temp[i] in operators:
                    if temp[i+1] in operators or temp[i+1] == ')':
                        return False
                elif self._isNumber(temp[i]) == True:
                    if self._isNumber(temp[i+1]) == True:
                        return False
                elif temp[i] == '(' and i != 0:
                    if self._isNumber(temp[i-1]) == True or temp[i-1] == ')':
                        return False
                elif temp[i] == ')':
                    if self._isNumber(temp[i+1]) == True or temp[i+1] == '(':
                        return False
            num = 0
            for i in temp:
                if i == '(':
                    num += 1
                elif  i == ')':
                    num -= 1
                if num < 0:
                    return False
            if num != 0:
                return False
        return True
            
                
                    

                    
    
                    
        
            
        


    @property
    def calculate(self):
        '''
            calculate must call _getPostfix
            calculate must create and use a Stack to compute the final result as shown in the video lecture
            
            >>> x=Calculator()
            >>> x.setExpr('4 + 3 - 2')
            >>> x.calculate
            5.0
            >>> x.setExpr('-2 + 3.5')
            >>> x.calculate
            1.5
            >>> x.setExpr('4 + 3.65 - 2 / 2')
            >>> x.calculate
            6.65
            >>> x.setExpr('23 / 12 - 223 + 5.25 * 4 * 3423')
            >>> x.calculate
            71661.91666666667
            >>> x.setExpr(' 2 - 3 * 4')
            >>> x.calculate
            -10.0
            >>> x.setExpr('7 ^ 2 ^ 3')
            >>> x.calculate
            5764801.0
            >>> x.setExpr(' 3 * ( ( ( 10 - 2 * 3 ) ) )')
            >>> x.calculate
            12.0
            >>> x.setExpr('8 / 4 * ( 3 - 2.45 * ( 4 - 2 ^ 3 ) ) + 3')
            >>> x.calculate
            28.6
            >>> x.setExpr('2 * ( 4 + 2 * ( 5 - 3 ^ 2 ) + 1 ) + 4')
            >>> x.calculate
            -2.0
            >>> x.setExpr(' 2.5 + 3 * ( 2 + ( 3.0 ) * ( 5 ^ 2 - 2 * 3 ^ ( 2 ) ) * ( 4 ) ) * ( 2 / 8 + 2 * ( 3 - 1 / 3 ) ) - 2 / 3 ^ 2')
            >>> x.calculate
            1442.7777777777778
            

            # In invalid expressions, you might print an error message, but code must return None, adjust doctest accordingly
            >>> x.setExpr(" 4 + + 3 + 2")
            >>> x.calculate
            'invalid expresstion'
            >>> x.setExpr("4  3 + 2")
            >>> x.calculate
            'invalid expresstion'
            >>> x.setExpr('( 2 ) * 10 - 3 * ( 2 - 3 * 2 ) )')
            >>> x.calculate
            'invalid expresstion'
            >>> x.setExpr('( 2 ) * 10 - 3 * / ( 2 - 3 * 2 )')
            >>> x.calculate
            'invalid expresstion'
            >>> x.setExpr(' ) 2 ( * 10 - 3 * ( 2 - 3 * 2 ) ')
            >>> x.calculate
            'invalid expresstion'
            >>> x.setExpr('( 3.5 ) ( 15 )')
            >>> x.calculate
            'invalid expresstion'
            >>> x.setExpr('3 ( 5 ) - 15 + 85 ( 12 )')
            >>> x.calculate
            'invalid expresstion'
            >>> x.setExpr("( -2 / 6 ) + ( 5 ( ( 9.4 ) ) )")
            >>> x.calculate
            'invalid expresstion'
        '''

        if not isinstance(self.__expr,str) or len(self.__expr)<=0:
            print("Argument error in calculate")
            return None

        calcStack = Stack()   # method must use calcStack to compute the  expression

        # YOUR CODE STARTS HERE
        t = self._getPostfix(self.__expr)
        if t == 'invalid expresstion':
            return 'invalid expresstion'
        expr = t.split()
        operators = ['+', '-', '*', '/', '^']
        for i in expr:
            if i not in operators:
                calcStack.push(i)
            else:
                a = calcStack.pop()
                b = calcStack.pop()
                if i == '+':
                    c = float(b) + float(a)
                elif i == '-':
                    c = float(b) - float(a)
                elif i == '*':
                    c = float(b) * float(a)
                elif i == '/':
                    c = float(b) / float(a)
                elif i == '^':
                    c = float(b) ** float(a)
                calcStack.push(c)
        return float(calcStack.pop())


    
  
          
    
#=============================================== Part III ==============================================

class AdvancedCalculator:
    '''
        >>> C = AdvancedCalculator()
        >>> C.states == {}
        True
        >>> C.setExpression('a = 5;b = 7 + a;a = 7;c = a + b;c = a * 0;return c')
        >>> C.calculateExpressions() == {'a = 5': {'a': 5.0}, 'b = 7 + a': {'a': 5.0, 'b': 12.0}, 'a = 7': {'a': 7.0, 'b': 12.0}, 'c = a + b': {'a': 7.0, 'b': 12.0, 'c': 19.0}, 'c = a * 0': {'a': 7.0, 'b': 12.0, 'c': 0.0}, '_return_': 0.0}
        True
        >>> C.states == {'a': 7.0, 'b': 12.0, 'c': 0.0}
        True
        >>> C.setExpression('x1 = 5;x2 = 7 * ( x1 - 1 );x1 = x2 - x1;return x2 + x1 ^ 3')
        >>> C.states == {}
        True
        >>> C.calculateExpressions() == {'x1 = 5': {'x1': 5.0}, 'x2 = 7 * ( x1 - 1 )': {'x1': 5.0, 'x2': 28.0}, 'x1 = x2 - x1': {'x1': 23.0, 'x2': 28.0}, '_return_': 12195.0}
        True
        >>> print(C.calculateExpressions())
        {'x1 = 5': {'x1': 5.0}, 'x2 = 7 * ( x1 - 1 )': {'x1': 5.0, 'x2': 28.0}, 'x1 = x2 - x1': {'x1': 23.0, 'x2': 28.0}, '_return_': 12195.0}
        >>> C.states == {'x1': 23.0, 'x2': 28.0}
        True
        >>> C.setExpression('x1 = 5 * 5 + 97;x2 = 7 * ( x1 / 2 );x1 = x2 * 7 / x1;return x1 * ( x2 - 5 )')
        >>> C.calculateExpressions() == {'x1 = 5 * 5 + 97': {'x1': 122.0}, 'x2 = 7 * ( x1 / 2 )': {'x1': 122.0, 'x2': 427.0}, 'x1 = x2 * 7 / x1': {'x1': 24.5, 'x2': 427.0}, '_return_': 10339.0}
        True
        >>> C.states == {'x1': 24.5, 'x2': 427.0}
        True
        >>> C.setExpression('A = 1;B = A + 9;C = A + B;A = 20;D = A + B + C;return D - A')
        >>> C.calculateExpressions() == {'A = 1': {'A': 1.0}, 'B = A + 9': {'A': 1.0, 'B': 10.0}, 'C = A + B': {'A': 1.0, 'B': 10.0, 'C': 11.0}, 'A = 20': {'A': 20.0, 'B': 10.0, 'C': 11.0}, 'D = A + B + C': {'A': 20.0, 'B': 10.0, 'C': 11.0, 'D': 41.0}, '_return_': 21.0}
        True
        >>> C.states == {'A': 20.0, 'B': 10.0, 'C': 11.0, 'D': 41.0}
        True
        >>> C.setExpression('A = 1;B = A + 9;2C = A + B;A = 20;D = A + B + C;return D + A')
        >>> C.calculateExpressions() is None
        True
        >>> C.states == {}
        True
    '''
    def __init__(self):
        self.expressions = ''
        self.states = {}

    def setExpression(self, expression):
        self.expressions = expression
        self.states = {}

    def _isVariable(self, word):
        '''
            >>> C = AdvancedCalculator()
            >>> C._isVariable('volume')
            True
            >>> C._isVariable('4volume')
            False
            >>> C._isVariable('volume2')
            True
            >>> C._isVariable('vol%2')
            False
        '''
        # YOUR CODE STARTS HERE
        a = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890'
        if len(word) == 0:
            return False
        else:
            if ord(word[0]) >= ord('0') and ord(word[0]) <= ord('9'):
                return False
            else:
                for i in word:
                    if i not in a:
                        return False
                return True
            
                    
                    
                   
       

    
   
    def _replaceVariables(self, expr):
        '''
            >>> C = AdvancedCalculator()
            >>> C.states = {'x1': 23.0, 'x2': 28.0}
            >>> C._replaceVariables('1')
            '1'
            >>> C._replaceVariables('105 + x')
            >>> C._replaceVariables('7 * ( x1 - 1 )')
            '7 * ( 23.0 - 1 )'
            >>> C._replaceVariables('x2 - x1')
            '28.0 - 23.0'
        '''
        # YOUR CODE STARTS HERE
        expr = expr.split()
        for i in range(len(expr)):
            if expr[i] in self.states:
                expr[i] = str(self.states[expr[i]])
        C = Calculator()
        a = '+-*/^()'
        for i in expr:
            if i not in a and C._isNumber(i) == False:
                return None
        return ' '.join(expr)
                
        


    
    def calculateExpressions(self):
        self.states = {}
        calcObj = Calculator()# method must use calcObj to compute each expression
        # YOUR CODE STARTS HERE
        data ={}
        k = self.expressions
        k = k.split('return ')
        k = ''.join(k)
        k = k.split(';')
        for i in range(len(k)):
            
            if i != len(k)-1:
                temp = k[i].split()
                first = temp[0]
                temp = temp[2:]
                temp = ' '.join(temp)
                temp = self._replaceVariables(temp)
                if temp != None:
                    calcObj.setExpr(temp)
                    self.states[first] = calcObj.calculate
                    data[k[i]] = self.states.copy()
                else:
                    self.states = {}
                    return None
            else:
                temp = k[i]
                temp = self._replaceVariables(temp)
                if temp != None:
                    calcObj.setExpr(temp)
                    data['_return_'] = calcObj.calculate
                else:
                    self.states = {}
                    return None
        return data
            
                    

if __name__ == "__main__":
    import doctest
    doctest.testmod()
    
        
            
