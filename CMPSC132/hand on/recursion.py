#sum of the Square of the even num
def sumSquare(numList):
    if len(numList) == 0:
        return 0
    else:
        if numList[0] % 4 == 0:
            return numList[0] ** 2 + sumSquare(numList[1:])
        else:
            return sunSquare(numList[1:])

def isPrime(num, i=2):
    if num <= 1:
        return False
    if num == i:
        return True
    if num%i == 0:
        return False
    return isPrime(num, i+1)
    
def hasNumbers(num1,num2)：
    if num1 == num2:
        return True
    if num1 > num2:
        return False
    if num1%10 == num2%10:
        return hasNumbers(num1//10,num2//10)
     else:
        return hasNumbers(num1, num2//10)

def getVowels(txt):
    if len(txt) == 0:
        return ''
    else:
        if txt[0] in 'aeiouAEIOU':
            return txt[0].lower() + getVowels(txt[1:])
        else:
            return getVowels(txt[1:])
            
def pascal(n):
    if n <= 0:
        return []
    if n = 1:
        return [1]
    else:
        row = [1]
        previous_row = pascal(n-1)
        for i in range(len(previous_row) - 1):
            row.append(previous_row[i] + previous_row[i+1])
        row.append(1)
        return row
