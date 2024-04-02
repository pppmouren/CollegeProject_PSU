def hailstone(num):
    lst = []
    if num == 1:
        lst.append(int(num))
        return lst
    else:
        if num % 2 == 0:
            new_num = num / 2
            lst.append(int(num))
            return lst + hailstone(new_num)
        else:
            new_num = 3 * num + 1
            lst.append(int(num))
            return lst + hailstone(new_num)
