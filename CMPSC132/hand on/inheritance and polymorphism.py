class Account:
    interest = 0.02
    def __init__(self, name):
        self.holder = name
        self.balance = 0
    def deposite(self, amount):
        self.balance += amount
        return self.balance
    def withdraw(self, amount):
        if amount > self.blance:
            return 'Not enough funds'
        self.balance -= amount
        return self.balance
        
class CheakingAccount(Acount):
    withdraw_fee = 1
    interest = 0.01
    def __ init__(self, name):
        super().__init__(name)
    def withdraw(self, amount):
        return Account.withdraw(self, amount + self.withdraw_fee)

class BankAccount:
    def __init__(self):
        self.accounts = []
    def openAccount(self, name, amount, account_tpye=Account):
        customer=account_type(name)
        customer.deposit(amount)
        self.accounts.append(customer)
        return customer
    def payInterest(self):
        for customer in self.accounts:
            customer.deposite(customer.balance*customer.interest)

