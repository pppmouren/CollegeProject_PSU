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
public class CarLoan extends LoanAccount {
    private String vehicleVIN;
    
    public CarLoan(double principal, double annualInterestRate, int months, String vehicleVIN) {
        super(principal, annualInterestRate, months);
        this.vehicleVIN = vehicleVIN;
    }
    
    public String getvehicleVIN() {
        return this.vehicleVIN;
    }
    
    @Override
    public String toString() {
        return String.format("%s\n%s\n%s%s\n\n", "Car Loan with:", super.toString(), "Vehicle VIN:", getvehicleVIN());
    }
    
    
}
