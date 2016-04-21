import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class Main {

    public static int T;
    public static int R;
    public static int[] resources;
    public static Task[] tasks;

    public static void  populate(File file) throws FileNotFoundException{

        Scanner input = new Scanner(file);

        T = input.nextInt(); // number of tasks

        // first make the list of tasks
        tasks = new Task[T];
        for(int i = 0; i < T ; i++){
            Task temp = new Task(i+1); // initialize one task object for the total number of tasks
            tasks[i] = temp; // add it to tasks array
        }

        R = input.nextInt(); // number of resource types

        // then make list of resource types
        resources = new int[R];
        for(int i = 0; i < R; i++){
            resources[i] = input.nextInt(); // record the number of units of each resource type available
        }

        // gather remaining info which are the list of activities
        while(input.hasNext()){

            // record appropriate info for each task
            String activityName = input.next();
            int taskNumber = input.nextInt();
            int delay = input.nextInt();
            int resourceType = input.nextInt();
            int amount = input.nextInt();

            // initialize activity object
            Activity temp = new Activity(activityName, taskNumber, delay, resourceType, amount);

            // add it to appropriate task in the order it is given in input
            tasks[taskNumber-1].activities.add(temp);
        }

        input.close();

    }

    public static void main(String[] args) throws FileNotFoundException{

        File inputFile = null;

        // first check if args exist
        if(args.length != 1){
            System.out.println("ERROR: Incorrect arguments.");
            System.exit(0);
        }
        else{
            // if argument file name is not an actual file print an error message,
            // otherwise create file object
            try {
                inputFile = new File(args[0]);

            }
            catch ( Exception FileNotFoundException){
                System.out.printf("ERROR: File %s not found.\n", args[0]);
                System.exit(0);
            }
        }

        // first extract info from file
        populate(inputFile);

        // create optimisticManager object and run it to produce output
        OptimisticManager OptimisticManager = new OptimisticManager(T,R,resources,tasks);

        OptimisticManager.runOptimistic();

        // extract info again to override previous data
        populate(inputFile);

        // create banker object and run it to produce output
        Banker Banker = new Banker(T,R,resources,tasks);

        Banker.runBanker();

    }
}