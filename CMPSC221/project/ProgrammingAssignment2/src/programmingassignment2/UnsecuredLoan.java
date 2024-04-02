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
public class UnsecuredLoan extends LoanAccount{
    
    public UnsecuredLoan(double principal, double annualInterestRate, int months) {
        super(principal, annualInterestRate, months);
    }
    
    @Override
    public String toString() {
        return String.format("%s\n%s\n\n", "Unsecured Loan with:", super.toString());
    }
}
