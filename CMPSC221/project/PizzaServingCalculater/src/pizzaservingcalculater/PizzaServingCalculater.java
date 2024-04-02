/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pizzaservingcalculater;

import java.awt.Color;
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingConstants;

/**
 *
 * @author handsomedad
 */
public class PizzaServingCalculater extends JFrame implements ActionListener{

    private final JLabel Line1;
    private final JPanel Line2;
    private final JButton Line3;
    private final JLabel Line4;
    private JLabel a1 = new JLabel("Enter the size of the pizza in inches:");
    private JTextField a2 = new JTextField(4);
    
 
    public static void main(String[] args) {
        // TODO code application logic here
        PizzaServingCalculater window = new PizzaServingCalculater();
        window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        window.setSize(350,300);
        window.setVisible(true);
    }
    
    
    public PizzaServingCalculater() {
        super("Pizza Servings Calculator");
        setLayout(new GridLayout(4,1));
        
        
        Line1 = new JLabel("Pizza Servings Calculator", SwingConstants.CENTER);
        Line1.setForeground(Color.red);
        Line1.setFont(new Font("Serif",Font.PLAIN, 30));
        add(Line1);
        
        Line2 = new JPanel();
        Line2.add(a1);
        Line2.add(a2);
        add(Line2);
        
        
        Line3 = new JButton("Calculate Servings");
        add(Line3);
        Line3.addActionListener(this);
        
        Line4 = new JLabel(" ", SwingConstants.CENTER);
        add(Line4);
    }   
    
    
    @Override
    public void actionPerformed(ActionEvent e){
        String text = a2.getText();
        double i = Integer.valueOf(text);
        double serving = Math.pow(i/8, 2);
        Line4.setText(String.format("A %s inch pizza will serve %.2f people.", text, serving));
    }   
    
}
