// taskgraph.h

#ifndef TASKGRAPH_H
#define TASKGRAPH_H

#include <iostream>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <set>
#include <shared_mutex>

/**
 * @brief SharedData class represents shared data among tasks in the TaskGraph.
 */
class SharedData {
public:
    /**
     * @brief Constructor for SharedData class.
     */
    SharedData();

    /**
     * @brief Getter for the counter variable in SharedData.
     * @return The value of the counter variable.
     */
    int getCounter() const;

    /**
     * @brief Increments the counter variable in SharedData.
     */
    void incrementCounter();

private:
    int counter;
    mutable std::mutex mutex;
};

class TaskGraph;

/**
 * @brief Task class represents a generic task in the TaskGraph.
 */
class Task {
public:
    /**
     * @brief Parameterized constructor for Task class.
     * @param name The name of the task.
     * @param function The function associated with the task.
     */
    Task(std::string name, std::function<void(SharedData&, TaskGraph&)> function);
    
    /**
     * @brief Copy constructor for Task class.
     * @param other The Task object to be copied.
     */
    Task(const Task& other);

    /**
     * @brief Virtual destructor for Task class.
     */
    virtual ~Task() = default;

    /**
     * @brief Executes the task by calling its associated function.
     * @param sharedData Reference to the SharedData object.
     * @param subgraph Reference to the TaskGraph object.
     */
    virtual void execute(SharedData& sharedData, TaskGraph& subgraph);

    /**
     * @brief Checks if the task was executed successfully.
     * @return True if the task was executed successfully, false otherwise.
     */
    bool isSuccess() const;

    /**
     * @brief Checks if the task was executed.
     * @return True if the task was executed, false otherwise.
     */
    bool isExecuted() const;

    /**
     * @brief Adds a dependency to the task.
     * @param dependency Pointer to the Task object to be added as a dependency.
     */
    void addDependency(Task* dependency);

    /**
     * @brief Gets the name of the task.
     * @return The name of the task.
     */
    const std::string& getName() const;

    /**
     * @brief Gets the dependencies of the task.
     * @return Vector of pointers to Task objects representing the dependencies.
     */
    const std::vector<Task*>& getDependencies() const;

private:
    std::string name;
    std::function<void(SharedData&, TaskGraph&)> function;
    std::vector<Task*> dependencies;
    mutable std::mutex executeMutex;
protected:
    bool executed;
    bool success;
};

/**
 * @brief ConditionalTask class represents a task with conditional execution in the TaskGraph.
 */
class ConditionalTask : public Task {
public:
    /**
     * @brief Constructor for ConditionalTask class.
     * @param name The name of the task.
     * @param successTask The Task object to be executed if the condition is true.
     * @param failureTask The Task object to be executed if the condition is false.
     * @param condition The condition function.
     */
    ConditionalTask(
        std::string name,
        Task successTask,
        Task failureTask,
        std::function<bool(SharedData&)> condition
    );

    /**
     * @brief Executes the ConditionalTask based on the provided condition.
     * @param sharedData Reference to the SharedData object.
     * @param subgraph Reference to the TaskGraph object.
     */
    void execute(SharedData& sharedData, TaskGraph& subgraph) override;

private:
    Task successTask;
    Task failureTask;
    std::function<bool(SharedData&)> condition;
    mutable std::mutex executeMutex;
};

/**
 * @brief TaskGraph class represents a directed acyclic graph of tasks.
 */
class TaskGraph {
public:
    /**
     * @brief Adds a task to the TaskGraph.
     * @param task The Task object to be added.
     */
    void addTask(Task task);

    /**
     * @brief Adds a dependency between two tasks in the TaskGraph.
     * @param dependentTaskName The name of the dependent task.
     * @param dependencyTaskName The name of the dependency task.
     */
    void addDependency(const std::string& dependentTaskName, const std::string& dependencyTaskName);

    /**
     * @brief Executes tasks in the TaskGraph until all tasks are executed.
     * @param sharedData Reference to the SharedData object.
     */
    void executeTasks(SharedData& sharedData);

    /**
     * @brief Monitors the execution status of tasks in the TaskGraph.
     */
    void monitorStatus();

    /**
     * @brief Checks if all tasks in the TaskGraph were executed successfully.
     * @return True if all tasks were executed successfully, false otherwise.
     */
    bool areAllTasksSuccessful() const;

    /**
     * @brief Dumps the structure of the TaskGraph, including dependencies.
     */
    void dumpGraph() const;

    /**
     * @brief Prints the names of tasks in the TaskGraph.
     */
    void printTaskNames() const;

    /**
     * @brief Prints the merged flow of tasks in the TaskGraph.
     */
    void printMergedTaskFlow() const;
    
    /**
     * @brief Prints the merged flow of tasks in the TaskGraph recursively.
     * @param task Pointer to the Task object.
     * @param depth The depth of the recursion.
     */
    void printMergedTaskFlowRecursive(const Task* task, int depth) const;

private:
    /**
     * @brief Finds a Task in the TaskGraph by its name.
     * @param taskName The name of the task to be found.
     * @return Pointer to the Task object if found, nullptr otherwise.
     */
    Task* findTaskByName(const std::string& taskName);

    /**
     * @brief Executes a specific task in the TaskGraph.
     * @param task Pointer to the Task object to be executed.
     * @param sharedData Reference to the SharedData object.
     */
    void executeTask(Task* task, SharedData& sharedData);

    /**
     * @brief Dumps the structure of the TaskGraph recursively.
     * @param task Pointer to the Task object.
     * @param depth The depth of the recursion.
     */
    void dumpGraphRecursive(const Task* task, int depth) const;

    /**
     * @brief Performs a topological sort on the TaskGraph and returns the sorted tasks.
     * @return Vector of pointers to Task objects representing the topologically sorted tasks.
     */
    std::vector<const Task*> topologicalSort() const;

    /**
     * @brief Performs a topological sort recursively on the TaskGraph.
     * @param task Pointer to the Task object.
     * @param visited Set of visited tasks.
     * @param sortedTasks Vector to store the sorted tasks.
     * @param added Set of added tasks to ensure uniqueness.
     */
    void topologicalSortRecursive(const Task* task, std::unordered_set<const Task*>& visited, std::vector<const Task*>& sortedTasks, std::set<const Task*>& added) const;

private:
    std::vector<Task> tasks;
};


#endif // TASKGRAPH_H
