import java.util.*;

/**
 * Created by Megha on 4/8/16.
 */
public class OptimisticManager {

    private int T; // number of tasks

    private int R; // number of resources
    private int[] resources; // size R --> # of units present of each resource type

    private Task[] tasks; // array of all the tasks

    private int cycle = 0; // cycle counter

    private Queue<Task> blockedTasks = new LinkedList<Task>(); // queue of blocked tasks

    // optimistic manager constructor
    public OptimisticManager(int T, int R, int[] resources, Task[] tasks){
        this.T = T;
        this.R = R;
        this.resources = resources;
        this.tasks = tasks;
    }

    // goes through blocked queue and determines if their outstanding requests can be
    // granted otherwise increments their waiting time and puts it back on the queue
    public void checkBlocked(){

        int size = blockedTasks.size();

        for(int i = 0; i < size; i++){

            Task temp = blockedTasks.remove();
            int type = temp.blockedType;
            int amount = temp.blockedAmount;

            // request is still too large
            if(amount > resources[type-1]) {
                temp.incrementTimeWaiting();
                blockedTasks.add(temp);
            }
            // request can be granted, previously blocked task is unblocked
            else {
                temp.blocked = false;
                resources[type-1] -= amount;
                temp.heldResources[type-1] += amount;
                temp.nextActivity = true;
            }
        }
    }

    // adds resources to total available at the next cycle
    public void setTasks(){
        for(int i = 0; i < T; i++){
            //increment activity counter if the flag is set
            if(tasks[i].nextActivity) {
                tasks[i].incrementActivity();
                tasks[i].nextActivity = false;
            }

            // add the resources released from last cycle to total available
            for(int j = 0; j < resources.length; j++)
                resources[j] += tasks[i].changes[j];
        }
    }

    // returns true if all non terminated tasks are blocked
    public boolean deadlock(){
        for(int i = 0; i < T; i++){
            if(!tasks[i].terminated)
                if(!tasks[i].blocked)
                    return false;
        }
        return true;
    }

    // returns true if all tasks are terminated
    public boolean allTerminated(){
        for(int i = 0; i < T; i++){
            if(!tasks[i].terminated)
                return false;
        }
        return true;
    }

    // goes through task by order of input and aborts the first non-terminated blocked task
    public void abortLowestTask(){
        for(int i = 0; i < T; i++){
            if(!tasks[i].terminated && tasks[i].blocked){
                // set task as termianted and aborted
                tasks[i].terminated = true;
                tasks[i].aborted = true;
                // remove the task from blocked queue
                blockedTasks.remove(tasks[i]);
                // release its held resources back to total available
                for(int j = 0; j < R; j++)
                    resources[j] += tasks[i].heldResources[j];

                // also check if any blocked resources request could be granted
                // in the future and ublock the task to remove deadlock
                for(int k = 0; k < T; k++)
                    if(tasks[k].blockedAmount <= resources[tasks[k].blockedType-1])
                        tasks[k].blocked=false;

                break;
            }
        }
    }


    public void runOptimistic() {

        for (int l = 0; l < T; l++)
            tasks[l].heldResources = new int[R];

        int i;
        while (!allTerminated()) {

            // while there is a deadlock, abort the lowest task number and remove it from blocked queue
            while (deadlock()) {
                abortLowestTask();
            }

            // initialize the changes array for each cycle to make new resources available at cycle n+1
            for (i = 0; i < T; i++)
                tasks[i].changes = new int[R];

            // before anything check if any blocked task requests can be
            // granted otherwise increment their waiting time
            checkBlocked();

            // cycle through all the tasks for each cycle
            for (i = 0; i < T; i++) {

                // if task next activity flag is true, or activity is blocked, or it is terminated -- skip
                if (tasks[i].nextActivity || tasks[i].blocked || tasks[i].terminated)
                    continue;

                // set current variables
                Activity activity = tasks[i].activities.get(tasks[i].currentActivity);
                String activityName = activity.name;

                // keep track of delay
                if (activity.delay > 0){
                    activity.delay--;
                    continue;
                }

                // depending on name of activity take 4 different actions
                switch (activityName) {
                    // initiate is ignored in optimistic manager
                    case "initiate": activityName.equals("initiate");
                        tasks[i].nextActivity = true;
                        break;

                    case "request": activityName.equals("request");
                        // if request is larger than resources available, add to blocked queue
                        if (activity.amount > resources[activity.resourceType - 1]) {
                            blockedTasks.add(tasks[i]);
                            tasks[i].blocked = true;
                            // keep track of blocked type and amount to grant them later
                            tasks[i].blockedType = activity.resourceType;
                            tasks[i].blockedAmount = activity.amount;
                            tasks[i].incrementTimeWaiting();
                        }
                        // otherwise, grant the appropriate resources
                        else {
                            // remove those resources from the total available
                            resources[activity.resourceType - 1] -= activity.amount;
                            // in case of abort, keep track of resources held by the task
                            tasks[i].heldResources[activity.resourceType - 1] += activity.amount;
                            // move to next activity flag to increment counter
                            tasks[i].nextActivity = true;
                        }
                        break;

                    case "release": activityName.equals("release");
                        // release resources to total available and keep track of held resources by the task
                        tasks[i].changes[activity.resourceType - 1] += activity.amount;
                        tasks[i].heldResources[activity.resourceType - 1] -= activity.amount;
                        tasks[i].nextActivity = true;
                        break;

                    case "terminate": activityName.equals("terminate");
                        // set total running time for the task and the terminated flag
                        tasks[i].setTimeRunning(cycle);
                        tasks[i].terminated = true;
                        break;
                }

            }

            // keep track of total number of cycles
            cycle++;
            // make appropriate resources available for next cycle and increment next activity counters
            setTasks();
        }

            // print the output
        System.out.printf("\t\t\tFIFO\n");
        int totalRunTime = 0;
            int totalWaitTime = 0;
            for (int k = 0; k < T; k++) {
                // if task was aborted
                if(tasks[k].aborted == true) {
                    System.out.printf("\tTask %d       aborted\n", k + 1);
                    continue;
                }
                totalRunTime += tasks[k].getTimeRunning();
                totalWaitTime += tasks[k].getTimeWaiting();
                System.out.printf("\tTask %d\t\t%2d   %2d   %2d%%\n",
                        k + 1, tasks[k].getTimeRunning(), tasks[k].getTimeWaiting(),
                        100 * tasks[k].getTimeWaiting() / tasks[k].getTimeRunning());
            }
            System.out.printf("\ttotal\t\t%2d   %2d   %2d%%\n\n", totalRunTime, totalWaitTime, 100 * totalWaitTime / totalRunTime);
    }
}