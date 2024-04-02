/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package programmingassignment3;

/**
 *
 * @author handsomedad
 */
public class Address {
    private String street;
    private String city;
    private String state;
    private String zipcode;
    
    public Address(String street, String city, String state, String zipcode) {
        this.city = city;
        this.state = state;
        this.street = street;
        this.zipcode = zipcode;
    }
    
    public String getstreet() {
        return this.street;
    }
    
    public String getcity() {
        return this.city;
    }
    
    public String getstate() {
        return this.state;
    }
    
    public String getzipcode() {
        return this.zipcode;
    }
    
    public String toString() {
        return String.format("  %s\n  %s%s %s %s", getstreet(), getcity(), ",", getstate(), getzipcode());
    }
}
