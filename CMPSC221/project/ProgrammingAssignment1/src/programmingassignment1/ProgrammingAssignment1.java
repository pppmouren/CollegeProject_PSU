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
public class ProgrammingAssignment1 {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here
        LoanAccount.setAnnualInterestRate(0.01);
        LoanAccount Loan1 = new LoanAccount(5000);
        LoanAccount Loan2 = new LoanAccount(31000);
        
        
        System.out.println("Monthly payments for loan1 of $5000.00 and loan2 of $31000.00 for 3, 5, and 6 year loans at 1% interest.");
        System.out.printf("%s\t%s\t%s\t%s%n%s\t%.2f\t%.2f\t%.2f%n%s\t%.2f\t%.2f\t%.2f","Loan", 
                "3 years", "5 years", "6 years", "Loan1", Loan1.calculateMonthlyPayment(36), Loan1.calculateMonthlyPayment(60), 
                Loan1.calculateMonthlyPayment(72), "Loan2", Loan2.calculateMonthlyPayment(36), Loan2.calculateMonthlyPayment(60), 
                Loan2.calculateMonthlyPayment(72));
        
        LoanAccount.setAnnualInterestRate(0.05);
        System.out.println("\n\nMonthly payments for loan1 of $5000.00 and loan2 of $31000.00 for 3, 5, and 6 year loans at 5% interest.");
        System.out.printf("%s\t%s\t%s\t%s%n%s\t%.2f\t%.2f\t%.2f%n%s\t%.2f\t%.2f\t%.2f","Loan", 
                "3 years", "5 years", "6 years", "Loan1", Loan1.calculateMonthlyPayment(36), Loan1.calculateMonthlyPayment(60), 
                Loan1.calculateMonthlyPayment(72), "Loan2", Loan2.calculateMonthlyPayment(36), Loan2.calculateMonthlyPayment(60), 
                Loan2.calculateMonthlyPayment(72));
    
    }
    
}
