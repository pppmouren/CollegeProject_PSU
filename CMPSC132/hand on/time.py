class Time:

    def __init__(self, hour=0, minute=0, second=0):
        self.__hour = hour  # 0-23
        self.minute = minute  # 0-59
        self.second = second  # 0-59

    def __str__(self):
        return f'{self.__hour}:{self.minute}:{self.second}'
