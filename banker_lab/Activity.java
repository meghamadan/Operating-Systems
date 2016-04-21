/**
 * Created by Megha on 4/8/16.
 *
 * Represents activities (initiate, request, release, terminate) that a task cycles through in a resource manager.
 */
public class Activity {

    // name of activity, task number, delay time, resource type,
    // and amount (initial claim, request amt, release amt)
    String name;
    int taskNumber;
    int delay;
    int resourceType;
    int amount;

    // one constructor that requires all 5 fields of activity object
    public Activity(String activityName, int taskNumber, int delay, int resourceType, int amount){
        this.name = activityName;
        this.taskNumber = taskNumber;
        this.delay = delay;
        this.resourceType = resourceType;
        this.amount = amount;

    }

    // printing for debugging purposes
    public String toString() {
        return this.name + " " + this.taskNumber + " " + this.delay + " " + this.resourceType + " " + this.amount;
    }

}