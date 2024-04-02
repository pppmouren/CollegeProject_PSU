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
public class PrimaryMortgage extends LoanAccount {
    private double PMIMonthlyAmount;
    private Address properAddress;
    
    public PrimaryMortgage(double principal, double annualInterestRate, int months, double PMIMonthlyAmount, Address properAddress) {
        super(principal, annualInterestRate, months);
        this.PMIMonthlyAmount = PMIMonthlyAmount;
        this.properAddress = properAddress;
    }
    
    @Override
    public String toString() {
        return String.format("%s\n%s\n%s%.2f\n%s\n%s\n\n", "Primary Mortgage Loan with:", super.toString(), "PMI Monthly Amount: $", 
                this.PMIMonthlyAmount, "Property Address:", this.properAddress);
    }
    
}
