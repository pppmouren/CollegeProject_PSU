
import java.sql.Connection;
import java.util.ArrayList;
import java.util.Calendar;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author handsomedad
 */
public class ScheduleQueries {
    private static PreparedStatement addScheduleEntry;
    private static PreparedStatement getScheduleByStudent;
    private static PreparedStatement getScheduledStudentCount;
    private static PreparedStatement getScheduledStudentsByCourse;
    private static PreparedStatement getWaitlistedStudentsByCourse;
    private static PreparedStatement dropStudentScheduleByCourse;
    private static PreparedStatement dropScheduleByStudent;
    private static PreparedStatement dropScheduleByCourse;
    private static PreparedStatement updateScheduleEntry;
    private static Connection connection;
    private static ResultSet resultset;
    private static int count;
   
    public static void addScheduleEntry(ScheduleEntry entry){
        connection = DBConnection.getConnection();
        
        try{
            java.sql.Timestamp currentTimestamp = new java.sql.Timestamp(Calendar.getInstance().getTime().getTime());

            addScheduleEntry = connection.prepareStatement("insert into app.schedule (Semester, CourseCode, StudentID, Status, Timestamp) values (?, ?, ?, ?, ?)");
            addScheduleEntry.setString(1, entry.getSemester());
            addScheduleEntry.setString(2, entry.getCourseCode());
            addScheduleEntry.setString(3, entry.getStudentID());
            addScheduleEntry.setString(4, entry.getStatus());
            addScheduleEntry.setTimestamp(5, currentTimestamp);
            addScheduleEntry.executeUpdate();
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
    }
    
    public static ArrayList<ScheduleEntry> getScheduleByStudent(String semester, String studentID){
        connection = DBConnection.getConnection();
        ArrayList<ScheduleEntry> scheduleEntry = new ArrayList<>();
        
        try{
            
            addScheduleEntry = connection.prepareStatement("select * from app.schedule where Semester = ? and StudentID = ?");
            addScheduleEntry.setString(1, semester);
            addScheduleEntry.setString(2, studentID);
            resultset = addScheduleEntry.executeQuery();
            while (resultset.next()){
                ScheduleEntry element = new ScheduleEntry(resultset.getString(1),resultset.getString(2),resultset.getString(3),resultset.getString(4),resultset.getTimestamp(5));
                scheduleEntry.add(element);
            }
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
        return scheduleEntry;
    }
    
    public static int getScheduledStudentCount(String currentSemester, String courseCode){
        connection = DBConnection.getConnection();
        
        try{
            getScheduledStudentCount = connection.prepareStatement("select count(StudentID) from app.schedule where Semester = ? and courseCode = ?");
            getScheduledStudentCount.setString(1, currentSemester);
            getScheduledStudentCount.setString(2, courseCode);
            resultset = getScheduledStudentCount.executeQuery();
            resultset.next();
            count = resultset.getInt(1);
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
        return count;
    }


    public static ArrayList<ScheduleEntry> getScheduledStudentsByCourse(String semester, String courseCode){
        connection = DBConnection.getConnection();
        ArrayList<ScheduleEntry> Scheduled_scheduleEntry = new ArrayList<>();
        
        try{
            getScheduledStudentsByCourse = connection.prepareStatement("select * from app.schedule where Semester = ? and courseCode = ? and Status = ?");
            getScheduledStudentsByCourse.setString(1, semester);
            getScheduledStudentsByCourse.setString(2, courseCode);
            getScheduledStudentsByCourse.setString(3, "Scheduled");

            resultset = getScheduledStudentsByCourse.executeQuery();
            while(resultset.next()){
                ScheduleEntry element = new ScheduleEntry(resultset.getString(1),resultset.getString(2),resultset.getString(3),resultset.getString(4),resultset.getTimestamp(5));
                Scheduled_scheduleEntry.add(element);
            }
            
            
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
        return Scheduled_scheduleEntry;
    
    }

    
    public static ArrayList<ScheduleEntry> getWaitlistedStudentsByCourse(String semester, String courseCode){
        connection = DBConnection.getConnection();
        ArrayList<ScheduleEntry> Waitlisted_scheduleEntry = new ArrayList<>();
        
        try{
            getScheduledStudentsByCourse = connection.prepareStatement("select * from app.schedule where Semester = ? and courseCode = ? and Status = ? order by TimeStamp ASC");
            getScheduledStudentsByCourse.setString(1, semester);
            getScheduledStudentsByCourse.setString(2, courseCode);
            getScheduledStudentsByCourse.setString(3, "Waitlisted");

            resultset = getScheduledStudentsByCourse.executeQuery();
            while(resultset.next()){
                ScheduleEntry element = new ScheduleEntry(resultset.getString(1),resultset.getString(2),resultset.getString(3),resultset.getString(4),resultset.getTimestamp(5));
                Waitlisted_scheduleEntry.add(element);
            }
            
            
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
        return Waitlisted_scheduleEntry;
    
    }

    public static void dropStudentScheduleByCourse(String semester, String studentID, String courseCode){
        connection = DBConnection.getConnection();
        
        try{
            dropStudentScheduleByCourse = connection.prepareStatement("delete from app.schedule where Semester = ? and StudentID = ? and CourseCode = ?");
            dropStudentScheduleByCourse.setString(1, semester);
            dropStudentScheduleByCourse.setString(2, studentID);
            dropStudentScheduleByCourse.setString(3, courseCode);
            dropStudentScheduleByCourse.executeUpdate();
 
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
    }
    
    
    public static void dropScheduleByStudent(String studentID){
        connection = DBConnection.getConnection();
        
        try{
            dropScheduleByStudent = connection.prepareStatement("delete from app.schedule where studentID = ?");
            dropScheduleByStudent.setString(1, studentID);
            dropScheduleByStudent.executeUpdate();
 
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
    }
    
    
    public static void dropScheduleByCourse(String semester, String courseCode){
        connection = DBConnection.getConnection();
        
        try{
            dropScheduleByCourse = connection.prepareStatement("delete from app.schedule where Semester = ? and CourseCode = ?");
            dropScheduleByCourse.setString(1, semester);
            dropScheduleByCourse.setString(2, courseCode);
            dropScheduleByCourse.executeUpdate();
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
                
        
    }
    
    public static void updateScheduleEntry(ScheduleEntry entry){
        connection = DBConnection.getConnection();
        
        try{ 
            updateScheduleEntry = connection.prepareStatement("update app.schedule set Status = ? where Semester = ? and StudentId = ? and CourseCode = ?");
            updateScheduleEntry.setString(1, "Scheduled");
            updateScheduleEntry.setString(2, entry.getSemester());
            updateScheduleEntry.setString(3, entry.getStudentID());
            updateScheduleEntry.setString(4, entry.getCourseCode());
            updateScheduleEntry.executeUpdate();
        }
        catch(SQLException sqlException){
            sqlException.printStackTrace();
        }
    }
}