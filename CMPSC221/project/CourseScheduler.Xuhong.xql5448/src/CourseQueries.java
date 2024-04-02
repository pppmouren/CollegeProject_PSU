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
public class CourseQueries {
    private static PreparedStatement getAllCourses;
    private static PreparedStatement addCourse;
    private static PreparedStatement getAllCourseCodes;
    private static PreparedStatement getCourseSeats;
    private static PreparedStatement dropCourse;
    private static Connection connection;
    private static ResultSet resultset;
    private static Integer seats;
    
    public static void addCourse(CourseEntry course){
        connection = DBConnection.getConnection();
        
        try{
            addCourse = connection.prepareStatement("insert into app.course (Semester, CourseCode, Description, Seats) values (?, ?, ?, ?)");
            addCourse.setString(1, course.getSemester());
            addCourse.setString(2, course.getCourseCode());
            addCourse.setString(3, course.getDescription());
            addCourse.setInt(4, course.getSeats());
            addCourse.executeUpdate();
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
    }
    
    public static ArrayList<CourseEntry> getAllCourses(String semester){
        connection = DBConnection.getConnection();
        ArrayList<CourseEntry> course = new ArrayList<>();
        
        try{
            getAllCourses = connection.prepareStatement("select * from app.course where Semester = ?");
            getAllCourses.setString(1,semester);
            resultset = getAllCourses.executeQuery();
            
            while (resultset.next()){
                CourseEntry element = new CourseEntry(resultset.getString(1),resultset.getString(2),resultset.getString(3),resultset.getInt(4));
                course.add(element);
            }
            
        }
        catch(SQLException sqlException)
        {
            sqlException.printStackTrace();
        }
        return course;
    }
    
    //obtain all the coursecode in a particualr semester
    public static ArrayList<String> getAllCourseCodes(String semester){
        connection = DBConnection.getConnection();
        ArrayList<String> coursecode = new ArrayList<>();
        
        try{
            getAllCourseCodes = connection.prepareStatement("select CourseCode from app.course where Semester = ?");
            getAllCourseCodes.setString(1,semester);
            resultset = getAllCourseCodes.executeQuery();
            
            while (resultset.next()){
                coursecode.add(resultset.getString(1));
            }
        }
        catch(SQLException sqlException)
        {
            sqlException.printStackTrace();
        }
        return coursecode;
        
    }
    
    public static int getCourseSeats(String semester, String courseCode){
        connection = DBConnection.getConnection();
        
        try{
            getCourseSeats = connection.prepareStatement("select Seats from app.course where Semester = ? and CourseCode = ?");
            getCourseSeats.setString(1, semester);
            getCourseSeats.setString(2, courseCode);
            resultset = getCourseSeats.executeQuery();
            resultset.next();
            seats = resultset.getInt(1);
               
        }
        catch(SQLException sqlException)
        {
            sqlException.printStackTrace();
        }
        return seats;
    }
    
    public static void dropCourse(String semester, String courseCode){
        connection = DBConnection.getConnection();
        
        try{
            dropCourse = connection.prepareStatement("delete from app.course where Semester = ? and CourseCode = ?");
            dropCourse.setString(1, semester);
            dropCourse.setString(2,courseCode);
            dropCourse.executeUpdate();
        }
        catch(SQLException sqlException)
        {
            sqlException.printStackTrace();
        }
    }

}

