/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package programmingassignment3;

import java.util.ArrayList;

/**
 *
 * @author handsomedad
 */
public class ProgrammingAssignment3 {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here
        // Create different loan objects, at least one of each type.
        CarLoan carLoan1 = new CarLoan(25000.00, 4.9, 72, "IRQ3458977");
        CarLoan carLoan2 = new CarLoan(12000.00, 5, 60, "NXK6767876");
        
        Address propertyAddress1 = new Address("321 Main Street", "State College", "PA", "16801");
        PrimaryMortgage propertyLoan1 = new PrimaryMortgage(250000.00, 3.75, 360, 35.12, propertyAddress1);
        Address propertyAddress2 = new Address("783 Maple Lane", "State College", "PA", "16801");
        PrimaryMortgage propertyLoan2 = new PrimaryMortgage(375000.00, 2.5, 360, 53.12, propertyAddress2);
        
        UnsecuredLoan unsecuredLoan = new UnsecuredLoan(5000.00, 10.75, 48);
        
        // create customers
        Customer customerA = new Customer("Tony", "Stark", "111-22-3333");
        Customer customerB = new Customer("Gal", "Gadot", "444-55-6666");
        
        // add loans for the customers.
        customerA.addLoanAccount(carLoan1);
        customerA.addLoanAccount(propertyLoan1);
        customerA.addLoanAccount(unsecuredLoan);
        
        customerB.addLoanAccount(carLoan2);
        customerB.addLoanAccount(propertyLoan2);
        
        // add the customers to an arraylist of customers.
        ArrayList<Customer> customers = new ArrayList<>();
        customers.add(customerA);
        customers.add(customerB);
        
        //Print out the loan information for each customer polymorhically.
        System.out.println("Monthly Report of Customers by Loan Account");
        for (Customer customer:customers)
        {
            System.out.println(customer.printMonthlyReport());
        }
    }
}
