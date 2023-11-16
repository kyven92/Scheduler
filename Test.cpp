// test_taskgraph.cpp

#include "Scheduler.h"

void startFunction(SharedData& sharedData, TaskGraph& subgraph) {
    std::cout << "Start Task, Counter: " << sharedData.getCounter() << std::endl;
    sharedData.incrementCounter();
}

// Example of task functions with potential errors
void taskAFunction(SharedData& sharedData, TaskGraph& subgraph) {
    // throw std::runtime_error("Simulated error in Task A");

     std::cout << "Task A, Counter: " << sharedData.getCounter() << std::endl;
    sharedData.incrementCounter();
}

void taskBFunction(SharedData& sharedData, TaskGraph& subgraph) {
    std::cout << "Task B, Counter: " << sharedData.getCounter() << std::endl;
    sharedData.incrementCounter();

    // Execute a subgraph as a task
    // subgraph.executeTasks(sharedData);
}

void taskCFunction(SharedData& sharedData, TaskGraph& subgraph) {
    std::cout << "Task C, Counter: " << sharedData.getCounter() << std::endl;
    sharedData.incrementCounter();
}

void taskDFunction(SharedData& sharedData, TaskGraph& subgraph) {
    // std::cout << "Task D, Counter: " << sharedData.getCounter() << std::endl;
    // sharedData.incrementCounter();

    throw std::runtime_error("Simulated error in Task D");
}

void taskEFunction(SharedData& sharedData, TaskGraph& subgraph) {
    std::cout << "Task E, Counter: " << sharedData.getCounter() << std::endl;
    sharedData.incrementCounter();
}

void taskFFunction(SharedData& sharedData, TaskGraph& subgraph) {
    std::cout << "Task F, Counter: " << sharedData.getCounter() << std::endl;
    sharedData.incrementCounter();
}

void endFunction(SharedData& sharedData, TaskGraph& subgraph) {
    std::cout << "End Task, Counter: " << sharedData.getCounter() << std::endl;
    sharedData.incrementCounter();
}
void errorFunction(SharedData& sharedData, TaskGraph& subgraph) {
    std::cout << "Error Task, Counter: " << sharedData.getCounter() << std::endl;
    sharedData.incrementCounter();
}


// Example of condition functions

bool conditionEnd(SharedData& sharedData) {
    
    return false;
}

int main() {
    TaskGraph taskGraph;
    SharedData sharedData;

    // Create tasks with conditions
    Task startTask("StartTask", startFunction);
    Task taskA("TaskA", taskAFunction);
    Task taskB("TaskB", taskBFunction);
    Task taskC("TaskC", taskCFunction);
    Task taskD("TaskD", taskDFunction);
    Task taskE("TaskE", taskEFunction);
    Task taskF("TaskF", taskEFunction);
    Task endTask("EndTask", endFunction);
    ConditionalTask conditionalTaskWithCondition("conditionalTask", endTask, taskF, conditionEnd);


    // Add tasks to the graph
    taskGraph.addTask(std::move(startTask));
    taskGraph.addTask(std::move(taskA));
    taskGraph.addTask(std::move(taskB));
    taskGraph.addTask(std::move(taskC));
    taskGraph.addTask(std::move(taskD));
    taskGraph.addTask(std::move(taskE));
    taskGraph.addTask(std::move(taskF));
    taskGraph.addTask(std::move(endTask));
    taskGraph.addTask(std::move(conditionalTaskWithCondition));

    // Define task dependencies
    taskGraph.addDependency("TaskA", "StartTask");
    taskGraph.addDependency("TaskB", "TaskA");
    taskGraph.addDependency("TaskC", "TaskA");
    taskGraph.addDependency("TaskD", "TaskC");
    taskGraph.addDependency("TaskD", "TaskB");
    taskGraph.addDependency("TaskE", "TaskD");

    taskGraph.addDependency("conditionalTask", "TaskE");
    taskGraph.addDependency("EndTask", "conditionalTask");
    taskGraph.addDependency("TaskF", "conditionalTask");

    

    // Start monitoring task status in a separate thread
    std::thread monitorThread(&TaskGraph::monitorStatus, &taskGraph);

    // Execute tasks cyclically based on conditions, sharing data
    // Dump the task graph structure
 
    taskGraph.printMergedTaskFlow();

    taskGraph.executeTasks(sharedData);

    // Join the monitor thread
    monitorThread.join();

    

    // Check if all tasks were successful
    if (taskGraph.areAllTasksSuccessful()) {
        std::cout << "All tasks executed successfully." << std::endl;
    } else {
        std::cerr << "Some tasks encountered errors." << std::endl;
    }

    return 0;
}
