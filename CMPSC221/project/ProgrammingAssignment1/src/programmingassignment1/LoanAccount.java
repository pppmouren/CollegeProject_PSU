/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package programmingassignment1;

/**
 *
 * @author handsomedad
 */
public class LoanAccount {
    private static double annualInterestRate;
    private double principle;
    
    
    public LoanAccount(double principle) {
        this.principle = principle;
    }
    
    //double monthlyInterest = annualInterestRate;(question mark)
    public double calculateMonthlyPayment(int numberOfPayments) {
        double monthlyPayment = principle * ( (annualInterestRate/12) / (1 - Math.pow(1 + (annualInterestRate/12), -numberOfPayments)));  
        return monthlyPayment;
    }
    
    
    public static void setAnnualInterestRate(double newannualInterestRate) {
        annualInterestRate = newannualInterestRate;
    }
    
    
    public static double getAnnualInterestRate() {
        return annualInterestRate;
    }
    
    
    public double getprinciple() {
        return this.principle;
        
    }
   
}
