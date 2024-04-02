class Counter:
    def __init__(self, aList):
        self.count_strings = {}
        for i in aList:
            if i not in self.count_strings:
                self.count_strings[i] = 1
            else:
                self.count_strings[i] += 1
def multiple(n,m):
    if not isinstance(n, int) and not isinstance(m, int):
        return False
    else:
        return n % m == 0

def square(n):
    k = 0
    for i in range(1,n):
        k += i**2
    return k

def count_vowel(text):
    t = 0
    for i in text.lower():
        if i in 'aeiou':
            t += 1
    return t

class Flower:
    def __ init__(self, name, qty, price):
        self._name = name
        self._numofpetals = qty
        self._price = price

    def get_name(self):
        return self._name
    def get_numofpetals(self):
        return self._numofpetals
    def get_price(self):
        return self._price
    def set_name(self, new_name):
        if isinstance(new_name, str):
            self._name = new_name
            return 'Completed'
        return 'invalid operation'
    def set_qty(self, new_qty):
        if isinstance(new_qty, int)"
            self._numofpetals = new_qty
            return 'Completed'
        return 'invalid operation'
    def set_price(self,new_price):
        if isinstance(new_price, float):
            self._price = new_price
            return 'Completed"
        return 'invalid operation'
    
        


    
        
