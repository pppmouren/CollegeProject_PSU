/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package programmingassignment2;

/**
 *
 * @author handsomedad
 */
public class LoanAccount {
    private double principal;
    private double annualInterestRate;
    private int months;
    
    public LoanAccount(double principal, double annualInterestRate, int months) {  
        this.principal = principal;
        this.annualInterestRate = annualInterestRate;
        this.months = months;
    }
    
    public double getprincipal() {
        return this.principal;
    }
    
    public double getannualInterestRate() {
        return this.annualInterestRate;
    }
    
    public int getmonths() {
        return this.months;
    }
    
    public double calculateMonthlyPayment() {
        return getprincipal() * ( (getannualInterestRate()/1200) / (1 - Math.pow(1 + (getannualInterestRate()/1200), -getmonths())));
    }
    
    public String toString() {
        return String.format("%s%.2f\n%s%.2f%s\n%s%d\n%s%.2f", "Principal:$", getprincipal(), "Annual Interest Rate:", 
                getannualInterestRate(), "%", "Term of loan in Months:", getmonths(), "Monthly Payment:$", calculateMonthlyPayment());
    }
   
    
}
