# HW1
# Due Date: 09/10/2021, 11:59PM
# REMINDER: The work in this assignment must be your own original work and must be completed alone.



def rectangle(perimeter,area):
    """
        >>> rectangle(14, 10)
        5
        >>> rectangle(12, 5)
        5
        >>> rectangle(25, 25)
        False
        >>> rectangle(50, 100)
        20
        >>> rectangle(11, 5)
        False
        >>> rectangle(11, 4)
        False
    """
    #- YOUR CODE STARTS HERE
    import math
    a = perimeter ** 2 - 16 * area
    if a >= 0:
        delta = math.sqrt(a)
        length = (perimeter + delta) / 4
        width = (perimeter - delta) / 4
        if width <= 0:
            return False
        else:
            if length.is_integer() and width.is_integer():
                return int(length)
            else:
                return False
    else:
        return False
    
    




def frequency(aString):
    """
        >>> frequency('mama')
        ('consonant', {'m': 2, 'a': 2})
        >>> answer = frequency('We ARE Penn State!!!')
        >>> answer[0]
        'vowel'
        >>> answer[1]
        {'w': 1, 'e': 4, 'a': 2, 'r': 1, 'p': 1, 'n': 2, 's': 1, 't': 2}
        >>> frequency('One who IS being Trained')
        ('consonant', {'o': 2, 'n': 3, 'e': 3, 'w': 1, 'h': 1, 'i': 3, 's': 1, 'b': 1, 'g': 1, 't': 1, 'r': 1, 'a': 1, 'd': 1})
    """
    #- YOUR CODE STARTS HERE
    new_aString = []
    aString = aString.lower()
    consonant = ['a','e','i','o','u','y']
    for i in aString:
        if i.isalpha() == True:
            new_aString.append(i)
    dict = {}
    for i in new_aString:
        if i not in dict:
            dict[i] = 1
        else:
            dict[i] = dict[i] + 1
    max_value = 0
    max_key = 0
    for i in dict.values():
        if max_value < i:
            max_value = i
    for i in dict.keys():
        if dict.get(i) == max_value:
            max_key = i
        break
    
    if max_key in consonant:
        return "consonant", dict
    else:
        return "vowel", dict
    
        
            





def successors(file):
    """
        >>> expected = {'.': ['We', 'Maybe'], 'We': ['came'], 'came': ['to'], 'to': ['learn', 'have', 'make'], 'learn': [',', 'how'], ',': ['eat'], 'eat': ['some'], 'some': ['pizza'], 'pizza': ['and', 'too'], 'and': ['to'], 'have': ['fun'], 'fun': ['.'], 'Maybe': ['to'], 'how': ['to'], 'make': ['pizza'], 'too': ['!']}
        >>> returnedDict = successors('items.txt')
        >>> expected == returnedDict
        True
        >>> returnedDict['.']
        ['We', 'Maybe']
        >>> returnedDict['to']
        ['learn', 'have', 'make']
        >>> returnedDict['fun']
        ['.']
        >>> returnedDict[',']
        ['eat']
    """

    with open(file) as f:
        contents = f.read()


    #- YOUR CODE STARTS HERE
    lst = []
    data = {}
    for i in contents:
        if i != ' ' and i.isalnum() == False:
            contents = contents[:contents.find(i)] + ' ' + contents[contents.find(i):]
            contents = contents[:contents.find(i) + 1] + ' ' + contents[contents.find(i)+ 1:]
    lst = contents.split()
    lst.insert(0,'.')
    for i in range(len(lst)):
        if lst[i] not in data and i != len(lst) - 1:
            data[lst[i]] = [lst[i + 1]]
        elif lst[i] in data and i != len(lst) - 1 and lst[i+1] not in data.get(lst[i]):
            data[lst[i]].append(lst[i+1])
            
    return data
            
        
    
            
  
    



def getPosition(num, digit):
    """
        >>> getPosition(1495, 5)
        1
        >>> getPosition(1495, 1)
        4
        >>> getPosition(1495423, 4)
        3
        >>> getPosition(1495, 7)
        False
    """
    #- YOUR CODE STARTS HERE
    rem = 0
    pos = 1
    while num > 0:
        rem = num % 10
        num = num // 10
        if rem == digit:
            return pos
        pos += 1
        if num == 0 and rem != digit:
            return False
        
        
        
        



def hailstone(n):
    """
        >>> hailstone(10)
        [10, 5, 16, 8, 4, 2, 1]
        >>> hailstone(1)
        [1]
        >>> hailstone(27)
        [27, 82, 41, 124, 62, 31, 94, 47, 142, 71, 214, 107, 322, 161, 484, 242, 121, 364, 182, 91, 274, 137, 412, 206, 103, 310, 155, 466, 233, 700, 350, 175, 526, 263, 790, 395, 1186, 593, 1780, 890, 445, 1336, 668, 334, 167, 502, 251, 754, 377, 1132, 566, 283, 850, 425, 1276, 638, 319, 958, 479, 1438, 719, 2158, 1079, 3238, 1619, 4858, 2429, 7288, 3644, 1822, 911, 2734, 1367, 4102, 2051, 6154, 3077, 9232, 4616, 2308, 1154, 577, 1732, 866, 433, 1300, 650, 325, 976, 488, 244, 122, 61, 184, 92, 46, 23, 70, 35, 106, 53, 160, 80, 40, 20, 10, 5, 16, 8, 4, 2, 1]
        >>> hailstone(7)
        [7, 22, 11, 34, 17, 52, 26, 13, 40, 20, 10, 5, 16, 8, 4, 2, 1]
        >>> hailstone(19)
        [19, 58, 29, 88, 44, 22, 11, 34, 17, 52, 26, 13, 40, 20, 10, 5, 16, 8, 4, 2, 1]
    """
    #- YOUR CODE STARTS HERE
    lst = [n]
    num = n
    while num != 1:
        if num % 2 == 0:
            num = num / 2
            lst.append(int(num))
        elif num % 2 == 1:
            num = 3 * num + 1
            lst.append(int(num))
    return lst
            
        
            


def largeFactor(num):
    """
        >>> largeFactor(15)
        5
        >>> largeFactor(80)
        40
        >>> largeFactor(13)
        1
    """
    #- YOUR CODE STARTS HERE
    lst = []
    for i in range(1,num):
        if num % i == 0:
            lst.append(i)
    return int(max(lst))






#  To run doctes per function, uncomment the next three lines
#  and replace the word rectangle for the function name you want to test

'''
if __name__=='__main__':
    import doctest
    doctest.testmod()
'''

