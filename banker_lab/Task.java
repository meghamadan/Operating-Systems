import java.util.ArrayList;

/**
 * Created by Megha on 4/8/16.
 *
 * Task object represent a task with various activities that need to run in a
 * resource manager
 */
public class Task {

    int taskNumber; // id

    ArrayList<Activity> activities = new ArrayList<>(); // array list holds the activities that need to be run

    int currentActivity = 0; // counter for the current activity the task is handling

    // flags for the state of the task
    boolean blocked = false;
    boolean terminated = false;
    boolean aborted = false;

    int timeWaiting = 0;
    int timeRunning = 0;

    int blockedAmount; // if a task is blocked, keep track of how much and what type of resource it is blocke don
    int blockedType;

    int[] changes; // changes that take place to total available resources at the next cycle
    boolean nextActivity = false; // determines if the task can move to next activity on the next cycle

    int[] heldResources; // keeps track of how many resources the task holds of each resource type

    int[] claim; // tracks the claim the task has of each resource type


    public Task(int taskNumber){
        this.taskNumber = taskNumber;
    }

    // copy constructor to deep copy Tasks array in the resource manager
    Task(Task t){
        taskNumber = t.taskNumber;
        activities = t.activities;;
        claim = t.claim;
        heldResources = t.heldResources;
        terminated = t.terminated;
        blocked = t.blocked;
        blockedType = t.blockedType;
        aborted = t.aborted;
        blocked = t.blocked;
        currentActivity = t.currentActivity;
    }



    // getters and setters for total time waiting and time running
    int getTimeWaiting(){
        return timeWaiting;
    }

    void incrementTimeWaiting(){
        timeWaiting++;
    }

    void setTimeRunning(int timeRunning){
        this.timeRunning = timeRunning;
    }

    int getTimeRunning(){
        return timeRunning;
    }

    // increments the current activity the task is handling
    void incrementActivity(){
        currentActivity++;
    }
}
