


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.ArrayList;
import java.sql.ResultSet;
/**
 *
 * @author handsomedad
 */
public class StudentQueries {
    private static Connection connection;
    private static PreparedStatement addStudent;
    private static PreparedStatement getAllStudents;
    private static PreparedStatement getStudent;
    private static PreparedStatement dropStudent;
    private static ResultSet resultSet;
    private static StudentEntry result;
    
    public static void addStudent(StudentEntry student){
        connection = DBConnection.getConnection();
        try{
            addStudent = connection.prepareStatement("insert into app.student (StudentID, FirstName, LastName) values (?, ?, ?)");
            addStudent.setString(1, student.getStudentID());
            addStudent.setString(2, student.getFirstName());
            addStudent.setString(3, student.getLastName());
            addStudent.executeUpdate();
        }
        catch(SQLException sqlException)
        {
            sqlException.printStackTrace();
        }
    }
    
    public static ArrayList<StudentEntry> getAllStudents(){
        connection = DBConnection.getConnection();
        ArrayList<StudentEntry> student = new ArrayList<>();
        
        try{
            getAllStudents = connection.prepareStatement("select * from app.student order by LastName, FirstName");
            resultSet = getAllStudents.executeQuery();
            while(resultSet.next()){
                StudentEntry element = new StudentEntry(resultSet.getString(1),resultSet.getString(2),resultSet.getString(3));
                student.add(element);
            }
        }
        catch(SQLException sqlException)
        {
            sqlException.printStackTrace();
        }
        return student;
    }
    
    
    
    public static StudentEntry getStudent(String studentID){
        connection = DBConnection.getConnection();
        
        try{
            getStudent = connection.prepareStatement("select * from app.student where StudentID = ?");
            getStudent.setString(1, studentID);
            resultSet = getStudent.executeQuery();
            resultSet.next();
            result = new StudentEntry(resultSet.getString(1),resultSet.getString(2),resultSet.getString(3));
            
            
        }
        catch(SQLException sqlException)
        {
            sqlException.printStackTrace();
        }
        return result;
    }


    public static void dropStudent(String studentID){
        connection = DBConnection.getConnection();
        
        try{
            dropStudent = connection.prepareStatement("delete from app.student where StudentID = ? ");
            dropStudent.setString(1, studentID);
            dropStudent.executeUpdate();
        }
        catch(SQLException sqlException)
        {
            sqlException.printStackTrace();
        }
    }
}