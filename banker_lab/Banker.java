import java.util.*;

/**
 * Created by Megha on 4/12/16.
 */
public class Banker {

    private int T; // number of tasks

    private int R; // number of resources
    private int[] resources; // size R --> # of units present of each resource type

    private Task[] tasks; // array of all the tasks

    private int cycle = 0; // cycle counter

    private Queue<Task> blockedTasks = new LinkedList<Task>(); // queue of blocked tasks

    // optimistic manager constructor
    public Banker(int T, int R, int[] resources, Task[] tasks){
        this.T = T;
        this.R = R;
        this.resources = resources;
        this.tasks = tasks;
    }

    // returns true if all tasks are terminated
    public boolean allTerminated(Task[] t){
        for(int i = 0; i < T; i++){
            if(!t[i].terminated)
                return false;
        }
        return true;
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
            for(int j = 0; j < R; j++)
                resources[j] += tasks[i].changes[j];
        }
    }

    // goes through blocked queue and determines if their outstanding requests can be
    // granted otherwise increments their waiting time and puts it back on the queue
    public void checkBlocked(){

        int size = blockedTasks.size();

        for(int i = 0; i < size; i++){

            Task temp = blockedTasks.remove();
            int type = temp.blockedType;
            int amount = temp.blockedAmount;

            if (amount > resources[type-1]){
                temp.incrementTimeWaiting();
                blockedTasks.add(temp);
            }

            // request is still too large
            else if(!safetyCheck(temp.taskNumber, type)) {
                temp.incrementTimeWaiting();
                blockedTasks.add(temp);
            }
            // request can be granted, previously blocked task is unblocked
            else {
                temp.blocked = false;
                resources[type - 1] -= amount;
                temp.heldResources[type - 1] += amount;
                temp.nextActivity = true;
            }
        }
    }

    // check safety of resulting resource manager if current request is hypothetically granted
    // and returns true if safe otherwise false
    public boolean safetyCheck(int taskNumber, int type){

        // make deep copy of tasks array to make changes without changing the original array
        // also make copy of the resources each task holds and available resources
        Task[] tasksCopy = new Task[T];
        int[] resourcesCopy = new int[R];
        int[][] heldCopy = new int[T][R];
        for(int i = 0; i < T; i++) {
            Task temp = tasks[i];
            tasksCopy[i] = new Task(temp);
            for (int j = 0; j < R; j++) {
                heldCopy[i][j] = tasks[i].heldResources[j];
                resourcesCopy[j] = resources[j];
            }
        }

        // boolean that should remain true as long as a process that terminates is found
        // otherwise it will break out of while loop
        boolean foundP = true;

        // hypothetically grant request to task by allocating resource to task using heldCopy array and
        // removing it from total available using resourcesCopy
        int requestAmount = tasksCopy[taskNumber-1].activities.get(tasks[taskNumber-1].currentActivity).amount;
        heldCopy[taskNumber-1][type-1] += requestAmount;
        resourcesCopy[type-1] -= requestAmount;


        // while there are still tasks not terminated keep going through tasks
        while(!allTerminated(tasksCopy) && foundP) {
            foundP = false;

            for ( int i = 0; i < T; i++) {

                // max additional needed must be less than available for ALL resource types
                for(int t = 0; t < R; t++) {

                    // skip checking process if claim was aborted
                    if (tasksCopy[i].aborted)
                        continue;

                    // set variables for task
                    int claim = tasksCopy[i].claim[t];
                    int held = heldCopy[i][t];

                    // maximum additional resources for task is claim minus held resources
                    int additionalNeeded = claim - held;

                    // if maximum additional is less than available resources then process can terminate
                    if (additionalNeeded <= resourcesCopy[t]) {
                        // release resources held by task now that it is terminated
                        resourcesCopy[t] += heldCopy[i][t];
                        heldCopy[i][t] -= requestAmount;
                        tasksCopy[i].terminated = true;
                        foundP = true;
                    }
                    else {

                        // reset available resources and held resources
                        for (int c = 0; c < t; c++){
                            resourcesCopy[c] -= heldCopy[i][c];
                            heldCopy[i][c] += requestAmount;
                        }
                        // also must hypothetically grant resources again
                        requestAmount = tasksCopy[taskNumber-1].activities.get(tasks[taskNumber-1].currentActivity).amount;
                        heldCopy[taskNumber-1][type-1] += requestAmount;
                        resourcesCopy[type-1] -= requestAmount;

                        foundP = false;
                        break ;
                    }
                }
            }
        }
        return allTerminated(tasksCopy);
    }


    public void runBanker() {

        for (int l = 0; l < T; l++) {
            tasks[l].heldResources = new int[R];
            tasks[l].claim = new int[R];
        }

        int i;
        while (!allTerminated(tasks)) {

            // initialize the changes array for each cycle to make new resources available at cycle n+1
            for (i = 0; i < T; i++)
                tasks[i].changes = new int[R];

            // before anything check if any blocked task requests can be
            // granted otherwise increment their waiting time
            checkBlocked();

            for ( i = 0; i < T; i++) {

                // if task next activity flag is true, or activity is blocked, or it is terminated -- skip
                if (tasks[i].nextActivity || tasks[i].blocked || tasks[i].terminated)
                    continue;

                // set current variables
                Activity activity = tasks[i].activities.get(tasks[i].currentActivity);
                String activityName = activity.name;

                // keep track of delay
                if (activity.delay > 0) {
                    activity.delay--;
                    continue;
                }

                int type = activity.resourceType;
                // depending on name of activity take 4 different actions
                switch (activityName) {
                    // initiate is ignored in optimistic manager
                    case "initiate":
                        activityName.equals("initiate");
                        tasks[i].claim[type-1] = activity.amount;
                        //tasks[i].claimType = activity.resourceType;

                        if(tasks[i].claim[type-1] > resources[type-1]) {
                            tasks[i].aborted = true;
                            tasks[i].terminated = true;
                            System.out.printf("Banker aborts task %d before run begins:\n" +
                                    "\tclaim for resource %d (%d) exceeds number of units present (%d)\n",
                                    i+1, type , tasks[i].claim[type-1], resources[type-1]);
                        }
                        tasks[i].nextActivity = true;
                        break;

                    case "request":
                        activityName.equals("request");
                        // if request or total request is larger than claim, abort the task and release its resources
                        int totalRequest = activity.amount + tasks[i].heldResources[activity.resourceType-1];
                        if (activity.amount > tasks[i].claim[type-1] || totalRequest > tasks[i].claim[type-1]) {
                            tasks[i].aborted = true;
                            tasks[i].terminated = true;
                            tasks[i].changes[activity.resourceType-1] += tasks[i].heldResources[type-1];

                            System.out.printf("During cycle %d-%d of Banker's algorithm\n" +
                                    "\tTask %d's request exceeds its claim ; aborted; %d units available next cycle",
                                    cycle, cycle+1, i+1, tasks[i].heldResources[type-1]);
                        }
                        // if request is larger than available resources, block task and add to blocked queue
                        else if (activity.amount > resources[activity.resourceType-1]){
                            tasks[i].blocked = true;
                            // keep track of blocked type and amount to grant them later
                            tasks[i].blockedType = activity.resourceType;
                            tasks[i].blockedAmount = activity.amount;
                            tasks[i].incrementTimeWaiting();
                            blockedTasks.add(tasks[i]);

                        }
                        // otherwise, grant the appropriate resources
                        else {
                            if(safetyCheck(tasks[i].taskNumber, type)) {
                                // remove those resources from the total available
                                resources[activity.resourceType - 1] -= activity.amount;
                                // in case of abort, keep track of resources held by the task
                                tasks[i].heldResources[type - 1] += activity.amount;
                                // move to next activity flag to increment counter
                                tasks[i].nextActivity = true;
                            }
                            else{
                                tasks[i].blocked = true;
                                // keep track of blocked type and amount to grant them later
                                tasks[i].blockedType = type;
                                tasks[i].blockedAmount = activity.amount;
                                tasks[i].incrementTimeWaiting();
                                blockedTasks.add(tasks[i]);
                            }
                        }
                        break;

                    case "release":
                        activityName.equals("release");
                        // release resources to total available and keep track of held resources by the task
                        tasks[i].changes[type - 1] += activity.amount;
                        tasks[i].heldResources[type - 1] -= activity.amount;
                        tasks[i].nextActivity = true;
                        break;

                    case "terminate":
                        activityName.equals("terminate");
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
        System.out.printf("\t\t\tBANKER'S\n");
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