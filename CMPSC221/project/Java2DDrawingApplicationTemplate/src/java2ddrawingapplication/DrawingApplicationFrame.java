/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package java2ddrawingapplication;

import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridLayout;
import java.awt.Paint;
import java.awt.Point;
import java.awt.Stroke;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.util.ArrayList;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JColorChooser;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSpinner;


/**
 *
 * @author acv
 */
public class DrawingApplicationFrame extends JFrame
{
    private final JPanel panel_container = new JPanel();
    private final JPanel panel1 = new JPanel();
    private final JPanel panel2 = new JPanel();
    private final JLabel label1 = new JLabel("Shapes:");
    private final JLabel label2 = new JLabel("Options:");
    private final JLabel label3 = new JLabel("Line Width:");
    private final JLabel label4 = new JLabel("Dash Length:");
    private final JComboBox<String> combo_box;
    private final JButton first_color = new JButton("1st Color");
    private final JButton second_color = new JButton("2st Color");
    private final JButton undo = new JButton("Undo");
    private final JButton clear = new JButton("Clear");
    private final JCheckBox checkbox_filled = new JCheckBox("Filled");
    private final JCheckBox checkbox_gradient = new JCheckBox("Use Gradient");
    private final JCheckBox checkbox_dashed = new JCheckBox("Dashed");
    private final JSpinner line_width = new JSpinner();
    private final JSpinner dash_length = new JSpinner();
    private final String[] three_type = {"Line", "Oval", "Rectangle"};
    private Color color1;
    private Color color2;
    private ArrayList<MyShapes> shape = new ArrayList<>();
    private final DrawPanel drawpanel = new DrawPanel();
    private final JLabel status = new JLabel();
    
    public DrawingApplicationFrame()
    {
        //set tital and layout
        super("Java 2D Drawings");
        setLayout(new BorderLayout());
        
        //add label1 to panel1
        panel1.add(label1);
        
        //create a JConboBox and add to panel1
        combo_box = new JComboBox<>(three_type);
        combo_box.setMaximumRowCount(3);
        panel1.add(combo_box);
        
        //create a handler that pull out colorchooser panel for first_color
        ActionListener handler1 = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent){
                
                color1 = JColorChooser.showDialog(combo_box, "Chooose the first color", color1);
                
                //set default color
                if (color1 == null)
                    color1 = Color.LIGHT_GRAY;
                
             
            }
        };
        
        //create a handler that pull out colorchooser panel for second_color
        ActionListener handler2 = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent){
                
                color2 = JColorChooser.showDialog(combo_box, "Chooose the first color", color2);
                
                //set default color
                if (color2 ==null)
                    color2 = Color.LIGHT_GRAY;
                
                
            }
        };
        
        //add actionlistener to first and second color and add them to panel1
        first_color.addActionListener(handler1);
        second_color.addActionListener(handler2);
        panel1.add(first_color);
        panel1.add(second_color);
        
        //undo and clear
        UndoHandler undohandler = new UndoHandler();
        undo.addActionListener(undohandler);
        ClearHandler clearhandler = new ClearHandler();
        clear.addActionListener(clearhandler);
        
        panel1.add(undo);
        panel1.add(clear);
        
        //pnael2
        panel2.add(label2);
        panel2.add(checkbox_filled);
        panel2.add(checkbox_gradient);
        panel2.add(checkbox_dashed);
        panel2.add(label3);
        panel2.add(line_width);
        panel2.add(label4);
        panel2.add(dash_length);
        
        //put panel1 and panel2 into panel_container
        panel_container.setLayout(new GridLayout(2,1));
        panel_container.add(panel1);
        panel_container.add(panel2);
        add(panel_container, BorderLayout.NORTH);
        
        //add drawpanel and status
        add(drawpanel, BorderLayout.CENTER);
        add(status, BorderLayout.SOUTH);
    }

    // Create event handlers, if needed
    //UndoHandler
    private class UndoHandler implements ActionListener {
        @Override
        public void actionPerformed(ActionEvent e) {
           shape.remove(shape.size() - 1);
           drawpanel.repaint();       
    }
    }
    //ClearHandler
    private class ClearHandler implements ActionListener {
        @Override
        public void actionPerformed(ActionEvent e) {
           shape.clear();
           drawpanel.repaint();       
    }
    }
    
    
    // Create a private inner class for the DrawPanel.
    private class DrawPanel extends JPanel
    {

        public DrawPanel()
        {
            MouseHandler mousehandler = new MouseHandler();
            addMouseListener(mousehandler);
            addMouseMotionListener(mousehandler);
        }
        
        @Override
        public void paintComponent(Graphics g)
        {
            super.paintComponent(g);
            Graphics2D g2d = (Graphics2D) g;

            //loop through and draw each shape in the shapes arraylist
            for (MyShapes shape_element : shape) {
                shape_element.draw(g2d);
            }
                
                    
        }


        private class MouseHandler extends MouseAdapter implements MouseMotionListener
        {
            private Paint paint;
            private Stroke stroke;
            private boolean filled;
            private MyShapes current_shape;
            public void mousePressed(MouseEvent event)
            { 
                
                //set paint variable
                if (checkbox_gradient.isSelected()) {
                    paint = new GradientPaint(0, 0, color1, 50, 50, color2, true);}
                else {
                    paint = (color1);
                }
                
                //set strock variable
                float[] float_dash = new float[]{(int)dash_length.getValue()};
                if (checkbox_dashed.isSelected()){
                    stroke = new BasicStroke((int)line_width.getValue(), BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND, 10, float_dash, 0);}
                else{
                    stroke = new BasicStroke((int)line_width.getValue(), BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND);}
                
                //set filled variable
                if (checkbox_filled.isSelected()){
                    filled = true;
                }
                else{
                    filled = false;
                }
                
                //get the start and end point
                Point A = event.getPoint();
           
                //check which shape
                if (combo_box.getSelectedItem() == "Line") {
                    current_shape = new MyLine(A,A,paint,stroke);
                }
                else if(combo_box.getSelectedItem() == "Oval") {
                    current_shape = new MyOval(A,A,paint,stroke,filled);
                }
                else if(combo_box.getSelectedItem() == "Rectangle") {
                    current_shape = new MyRectangle(A,A,paint,stroke,filled);
                }
                
                //add current_shape to MyShapes arraylist
                shape.add(current_shape);
                
                //show position in JLabel
                status.setText(String.format("(%d,%d)", event.getX(), event.getY()));
            
            }

            public void mouseReleased(MouseEvent event)
            {
                //show position in JLabel
                status.setText(String.format("(%d,%d)", event.getX(), event.getY()));
            }
            
            

            @Override
            public void mouseDragged(MouseEvent event)
            {
                Point B = event.getPoint();
                //reset the end point of shape and repaint
                if (shape.size() != 0){
                    shape.get(shape.size()-1).setEndPoint(B); 
                }
                drawpanel.repaint();
                
                //show position in JLabel
                status.setText(String.format("(%d,%d)", event.getX(), event.getY()));
              
            }

            @Override
            public void mouseMoved(MouseEvent event)
            {
                //show position in JLabel
                status.setText(String.format("(%d,%d)", event.getX(), event.getY()));
               
            }
        }

    }
}
