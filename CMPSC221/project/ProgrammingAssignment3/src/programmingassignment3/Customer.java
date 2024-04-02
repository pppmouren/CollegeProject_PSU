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
public class Customer {
    private final String firstName;
    private final String lastName;
    private final String SSN;
    

    public Customer (String firstName, String lastName, String SSN) {
        this.firstName = firstName;
        this.lastName = lastName;
        this.SSN = SSN;
    }
    
    public String getfirstName() {
        return this.firstName;
    }
    
    public String getlastName() {
        return this.lastName;
    }
    
    public String getSSN() {
        return this.SSN;
    }
    

    ArrayList<LoanAccount> customer = new ArrayList<>();
    public void addLoanAccount(LoanAccount account) {
        customer.add(account);
    }
    
    public String printMonthlyReport() {
        String a = String.format("%s %s %s %s %s\n\n", "Account Report for Customer:", getfirstName(), getlastName(), "with SSN", getSSN());
        for (LoanAccount account : customer) {
            a = a + account.toString();
        }
        return a;
    }
    
    
    
    
}
