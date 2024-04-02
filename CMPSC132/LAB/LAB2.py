# LAB2
# Due Date: 09/17/2021, 11:59PM
# REMINDER: The work in this assignment must be your own original work and must be completed alone.

import random

class Vendor:

    def __init__(self, name):
        '''
            In this class, self refers to Vendor objects
            
            name: str
            vendor_id: random int in the range (999, 999999)
        '''
        self.name = name
        self.vendor_id = random.randint(999, 999999)
    
    def install(self):
        '''
            Creates and initializes (instantiate) an instance of VendingMachine 
        '''
        return VendingMachine()
    
    def restock(self, machine, item, amount):
        '''
            machine: VendingMachine
            item: int
            amount : int/float

            Call _restock for the given VendingMachine object
        '''
        return machine._restock(item, amount)
        


class VendingMachine:
    '''
        In this class, self refers to VendingMachine objects

        >>> john_vendor = Vendor('John Doe')
        >>> west_machine = john_vendor.install()
        >>> west_machine.getStock
        {156: [1.5, 3], 254: [2.0, 3], 384: [2.5, 3], 879: [3.0, 3]}
        >>> john_vendor.restock(west_machine, 215, 9)
        'Invalid item'
        >>> west_machine.isStocked
        True
        >>> john_vendor.restock(west_machine,156, 1)
        'Current item stock: 4'
        >>> west_machine.getStock
        {156: [1.5, 4], 254: [2.0, 3], 384: [2.5, 3], 879: [3.0, 3]}
        >>> west_machine.purchase(156)
        'Please deposit $1.5'
        >>> west_machine.purchase(156,2)
        'Please deposit $3.0'
        >>> west_machine.purchase(156,23)
        'Current 156 stock: 4, try again'
        >>> west_machine.deposit(3)
        'Balance: $3'
        >>> west_machine.purchase(156,3)
        'Please deposit $1.5'
        >>> west_machine.purchase(156)
        'Item dispensed, take your $1.5 back'
        >>> west_machine.getStock
        {156: [1.5, 3], 254: [2.0, 3], 384: [2.5, 3], 879: [3.0, 3]}
        >>> west_machine.deposit(300)
        'Balance: $300'
        >>> west_machine.purchase(876)
        'Invalid item'
        >>> west_machine.purchase(384,3)
        'Item dispensed, take your $292.5 back'
        >>> west_machine.purchase(156,10)
        'Current 156 stock: 3, try again'
        >>> west_machine.purchase(156,3)
        'Please deposit $4.5'
        >>> west_machine.deposit(4.5)
        'Balance: $4.5'
        >>> west_machine.purchase(156,3)
        'Item dispensed'
        >>> west_machine.getStock
        {156: [1.5, 0], 254: [2.0, 3], 384: [2.5, 0], 879: [3.0, 3]}
        >>> west_machine.purchase(156)
        'Item out of stock'
        >>> west_machine.deposit(6)
        'Balance: $6'
        >>> west_machine.purchase(254,3)
        'Item dispensed'
        >>> west_machine.deposit(9)
        'Balance: $9'
        >>> west_machine.purchase(879,3)
        'Item dispensed'
        >>> west_machine.isStocked
        False
        >>> west_machine.deposit(5)
        'Machine out of stock. Take your $5 back'
        >>> west_machine.purchase(156,2)
        'Machine out of stock'
        >>> west_machine.purchase(665,2)
        'Invalid item'
        >>> east_machine = john_vendor.install()
        >>> west_machine.getStock
        {[156: [1.5, 0], 254: [2.0, 0], 384: [2.5, 0], 879: [3.0, 0]]}
        >>> east_machine.getStock
        {156: [1.5, 3], 254: [2.0, 3], 384: [2.5, 3], 879: [3.0, 3]}
        >>> east_machine.deposit(10)
        'Balance: $10'
        >>> east_machine.cancelTransaction()
        'Take your $10 back'
        >>> east_machine.purchase(156)
        'Please deposit $1.5'
        >>> east_machine.cancelTransaction()
    '''

    def __init__(self):
        #--- YOUR CODE STARTS HERE
        self.balance = 0
        self.stock = {156: [1.5, 3], 254: [2.0, 3], 384: [2.5, 3], 879: [3.0, 3]}



    def purchase(self, item, qty=1):
        #--- YOUR CODE STARTS HERE
        if item not in self.stock:
            return 'invalid item'
        elif all(i[1]==0 for i in self.stock.values()):
            return 'Machine out of stock'
        elif self.stock.get(item)[1] == 0:
            return 'Item out of stock'
        elif self.stock.get(item)[1] < qty:
            return f"current {item} stock: {self.stock.get(item)[1]}, try again"
        elif self.balance < qty * self.stock.get(item)[0]:
            return f"Please deposit ${qty * self.stock.get(item)[0] - self.balance}"
        elif self.balance - qty * self.stock.get(item)[0] == 0:
            self.balance -= qty * self.stock.get(item)[0]
            self.stock.get(item)[1] -= qty
            return 'Item dispensed'
        elif self.balance - qty * self.stock.get(item)[0] > 0:
            self.balance -= qty * self.stock.get(item)[0]
            self.stock.get(item)[1] -= qty
            return f"Item dispensed, take your ${self.balance} back"
            
        
        
        
    def deposit(self, amount):
        #--- YOUR CODE STARTS HERE
        
        if all(i[1]==0 for i in self.stock.values()):
            return f"Machine out of stock. Take your ${amount} back"
        else:
            self.balance = self.balance + amount
            return f"Balance: ${self.balance}"


    def _restock(self, item, stock):
        #--- YOUR CODE STARTS HERE
        if item not in self.stock:
            return 'Invalid item'
        else:
            self.stock.get(item)[1] += stock
            return f"Current item stock:{self.stock.get(item)[1]}"


    #--- YOUR CODE STARTS HERE
    @property
    def isStocked(self):
        if all(i[1]==0 for i in self.stock.values()):
            return False
        return True
        

    #--- YOUR CODE STARTS HERE
    @property
    def getStock(self):
        return self.stock


    def cancelTransaction(self):
        #--- YOUR CODE STARTS HERE
        if self.balance == 0:
            return None
        else:
            a = self.balance
            self.balance = 0
            return 'Take your ${a} back'
        
       


class Point2D:
    def __init__(self, x, y):
        self.x = x
        self.y = y


class Line: 
    ''' 
        >>> p1 = Point2D(-7, -9)
        >>> p2 = Point2D(1, 5.6)
        >>> line1 = Line(p1, p2)
        >>> line1.getDistance
        16.648
        >>> line1.getSlope
        1.825
        >>> line1
        y = 1.825x + 3.775
        >>> line2 = line1*4
        >>> line2.getDistance
        66.592
        >>> line2.getSlope
        1.825
        >>> line2
        y = 1.825x + 15.1
        >>> line1
        y = 1.825x + 3.775
        >>> line3 = 4*line1
        >>> line3
        y = 1.825x + 15.1
        >>> line1==line2
        False
        >>> line3==line2
        True
        >>> line5=Line(Point2D(6,48),Point2D(9,21))
        >>> line5
        y = -9.0x + 102.0
        >>> line5==9
        False
        >>> line6=Line(Point2D(2,6), Point2D(2,3))
        >>> line6.getDistance
        3.0
        >>> line6.getSlope
        inf
        >>> isinstance(line6.getSlope, float)
        True
        >>> line6
        Undefined
        >>> line7=Line(Point2D(6,5), Point2D(9,5))
        >>> line7.getSlope
        0.0
        >>> line7
        y = 5.0
    '''
    def __init__(self, point1, point2):
        #--- YOUR CODE STARTS HERE
        self.point1 = (point1.x,point1.y)
        self.point2 = (point2.x,point2.y)
        self.b = 0
    #--- YOUR CODE STARTS HERE
    
    @property
    def getDistance(self):
        import math
        return round(math.sqrt((self.point2[0]-self.point1[0])**2 + (self.point2[1] - self.point1[1])**2), 3)
       
    
    #--- YOUR CODE STARTS HERE
    @property
    def getSlope(self):
        if self.point2[0] - self.point1[0] != 0:
            return round((self.point2[1]-self.point1[1])/(self.point2[0] - self.point1[0]), 3)
        else:
            return float('infinity')


    #--- YOUR CODE CONTINUES HERE
    def __repr__(self):
        if self.point2[0] - self.point1[0] == 0:
            return 'Undefined'
        else:
            b = round(self.point1[1] - self.getSlope * self.point1[0], 3)
        return f"y = {self.getSlope}x + {b}"
    def __eq__(self,other):
        if isinstance(other, Line) and self.point1 == other.point1 and self.point2 == other.point2:
            return True
        else:
            return False
 
    def __mul__(self, other):
        point1 = Point2D(self.point1[0] * other,self.point1[1] * other)
        point2 = Point2D(self.point2[0] * other,self.point2[1] * other)
        return Line(point1, point2)
    def __rmul__(self, other):
        point1 = Point2D(self.point1[0] * other,self.point1[1] * other)
        point2 = Point2D(self.point2[0] * other,self.point2[1] * other)
        return Line(point1, point2)
    
        
            
