// taskgraph.cpp

#include "Scheduler.h"

/**
 * @brief Constructor for SharedData class.
 */
SharedData::SharedData() : counter(0) {}

/**
 * @brief Getter for the counter variable in SharedData.
 * @return The value of the counter variable.
 */
int SharedData::getCounter() const {
    std::lock_guard<std::mutex> lock(mutex);
    return counter;
}

/**
 * @brief Increments the counter variable in SharedData.
 */
void SharedData::incrementCounter() {
    std::lock_guard<std::mutex> lock(mutex);
    counter++;
}

/**
 * @brief Copy constructor for Task class.
 * @param other The Task object to be copied.
 */
Task::Task(const Task& other)
    : name(other.name), function(other.function), executed(other.executed), success(other.success) {
    // Copy other necessary member variables as needed
}

/**
 * @brief Parameterized constructor for Task class.
 * @param name The name of the task.
 * @param function The function associated with the task.
 */
Task::Task(std::string name, std::function<void(SharedData&, TaskGraph&)> function)
    : name(std::move(name)), function(std::move(function)), executed(false), success(true) {}

/**
 * @brief Executes the task by calling its associated function.
 * @param sharedData Reference to the SharedData object.
 * @param subgraph Reference to the TaskGraph object.
 */
void Task::execute(SharedData& sharedData, TaskGraph& subgraph) {
    std::unique_lock<std::mutex> lock(executeMutex);
    try {
        if (!executed) {
            std::cout << "Executing Task: " << name << std::endl;
            function(sharedData, subgraph);
            executed = true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in task " << name << ": " << e.what() << std::endl;
        success = false;
    }
}

/**
 * @brief Checks if the task was executed successfully.
 * @return True if the task was executed successfully, false otherwise.
 */
bool Task::isSuccess() const {
    return success;
}

/**
 * @brief Checks if the task was executed.
 * @return True if the task was executed, false otherwise.
 */
bool Task::isExecuted() const {
    return executed;
}

/**
 * @brief Adds a dependency to the task.
 * @param dependency Pointer to the Task object to be added as a dependency.
 */
void Task::addDependency(Task* dependency) {
    dependencies.push_back(dependency);
}

/**
 * @brief Gets the name of the task.
 * @return The name of the task.
 */
const std::string& Task::getName() const {
    return name;
}

/**
 * @brief Gets the dependencies of the task.
 * @return Vector of pointers to Task objects representing the dependencies.
 */
const std::vector<Task*>& Task::getDependencies() const {
    return dependencies;
}

/**
 * @brief Constructor for ConditionalTask class.
 * @param name The name of the task.
 * @param successTask The Task object to be executed if the condition is true.
 * @param failureTask The Task object to be executed if the condition is false.
 * @param condition The condition function.
 */
ConditionalTask::ConditionalTask(
    std::string name,
    Task successTask,
    Task failureTask,
    std::function<bool(SharedData&)> condition
) : Task(std::move(name), [this](SharedData& sharedData, TaskGraph& subgraph) {
        this->execute(sharedData, subgraph);
    }), successTask(std::move(successTask)), failureTask(std::move(failureTask)), condition(std::move(condition)) {}

/**
 * @brief Executes the ConditionalTask based on the provided condition.
 * @param sharedData Reference to the SharedData object.
 * @param subgraph Reference to the TaskGraph object.
 */
void ConditionalTask::execute(SharedData& sharedData, TaskGraph& subgraph) {
    std::unique_lock<std::mutex> lock(executeMutex);
    try {
        if (!isExecuted()) {
            std::cout << "Executing ConditionalTask: " << getName() << std::endl;
            if (condition(sharedData)) {
                successTask.execute(sharedData, subgraph);
            } else {
                failureTask.execute(sharedData, subgraph);
            }
            executed = true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in ConditionalTask " << getName() << ": " << e.what() << std::endl;
        success = false;
    }
}

/**
 * @brief Adds a task to the TaskGraph.
 * @param task The Task object to be added.
 */
void TaskGraph::addTask(Task task) {
    tasks.push_back(std::move(task));
}

/**
 * @brief Adds a dependency between two tasks in the TaskGraph.
 * @param dependentTaskName The name of the dependent task.
 * @param dependencyTaskName The name of the dependency task.
 */
void TaskGraph::addDependency(const std::string& dependentTaskName, const std::string& dependencyTaskName) {
    Task* dependentTask = findTaskByName(dependentTaskName);
    Task* dependencyTask = findTaskByName(dependencyTaskName);

    if (dependentTask && dependencyTask) {
        dependentTask->addDependency(dependencyTask);
    }
}

/**
 * @brief Finds a Task in the TaskGraph by its name.
 * @param taskName The name of the task to be found.
 * @return Pointer to the Task object if found, nullptr otherwise.
 */
Task* TaskGraph::findTaskByName(const std::string& taskName) {
    auto it = std::find_if(tasks.begin(), tasks.end(), [&taskName](const Task& task) {
        return task.getName() == taskName;
    });

    return (it != tasks.end()) ? &(*it) : nullptr;
}


/**
 * @brief Executes tasks in the TaskGraph until all tasks are executed.
 * @param sharedData Reference to the SharedData object.
 */
void TaskGraph::executeTasks(SharedData& sharedData) {
    while (true) {
        std::vector<std::thread> threads;

        for (Task& task : tasks) {
            if (!task.isExecuted()) {
                threads.emplace_back([this, &task, &sharedData] {
                    executeTask(&task, sharedData);
                });
            }
        }

        for (std::thread& thread : threads) {
            thread.join();
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep for 1 second before the next cycle
    }
}

/**
 * @brief Executes a specific task in the TaskGraph.
 * @param task Pointer to the Task object to be executed.
 * @param sharedData Reference to the SharedData object.
 */
void TaskGraph::executeTask(Task* task, SharedData& sharedData) {
    bool execute = true;

    for (Task* dependency : task->getDependencies()) {
        execute = execute && dependency->isExecuted();
    }

    if (execute) {
        std::cout << "Executing Task: " << task->getName() << std::endl;
        task->execute(sharedData, *this);
    }
}

/**
 * @brief Monitors the execution status of tasks in the TaskGraph.
 */
void TaskGraph::monitorStatus() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Sleep for 2 seconds between status checks

        std::cout << "Task Status:" << std::endl;
        for (const Task& task : tasks) {
            std::cout << task.getName() << ": " << (task.isExecuted() ? "Executed" : "Pending") << std::endl;
        }

        std::cout << std::endl;
    }
}

/**
 * @brief Checks if all tasks in the TaskGraph were executed successfully.
 * @return True if all tasks were executed successfully, false otherwise.
 */
bool TaskGraph::areAllTasksSuccessful() const {
    for (const Task& task : tasks) {
        if (!task.isSuccess()) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Dumps the structure of the TaskGraph, including dependencies.
 */
void TaskGraph::dumpGraph() const {
    std::cout << "Task Graph:" << std::endl;
    for (const Task& task : tasks) {
        dumpGraphRecursive(&task, 0);
    }
}

/**
 * @brief Prints the names of tasks in the TaskGraph.
 */
void TaskGraph::printTaskNames() const {
    std::cout << " -------- Print Tasks in graph queue ----------" << std::endl;
    for (const Task& task : tasks) {
        std::cout << task.getName() << std::endl;
    }
    std::cout << " -------- END of Print Tasks in graph queue ----------" << std::endl;
}

/**
 * @brief Dumps the structure of the TaskGraph recursively.
 * @param task Pointer to the Task object.
 * @param depth The depth of the recursion.
 */
void TaskGraph::dumpGraphRecursive(const Task* task, int depth) const {
    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }

    std::cout << task->getName();

    // Check and print task type
    if (dynamic_cast<const ConditionalTask*>(task)) {
        std::cout << " (ConditionalTask)";
    } else {
        std::cout << " (Task)";
    }

    std::cout << std::endl;

    for (const Task* dependency : task->getDependencies()) {
        dumpGraphRecursive(dependency, depth + 1);
    }
}

/**
 * @brief Prints the merged flow of tasks in the TaskGraph.
 */
void TaskGraph::printMergedTaskFlow() const {
    std::vector<const Task*> sortedTasks = topologicalSort();

    std::cout << "Merged Task Flow:" << std::endl;

    for (const Task* task : sortedTasks) {
        printMergedTaskFlowRecursive(task, 0);
    }
}

/**
 * @brief Prints the merged flow of tasks in the TaskGraph recursively.
 * @param task Pointer to the Task object.
 * @param depth The depth of the recursion.
 */
void TaskGraph::printMergedTaskFlowRecursive(const Task* task, int depth) const {
    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }

    std::cout << task->getName();

    std::cout << std::endl;

    const auto& dependencies = task->getDependencies();
    for (const Task* dependency : dependencies) {
        printMergedTaskFlowRecursive(dependency, depth + 1);
    }
}

/**
 * @brief Performs a topological sort on the TaskGraph and returns the sorted tasks.
 * @return Vector of pointers to Task objects representing the topologically sorted tasks.
 */
std::vector<const Task*> TaskGraph::topologicalSort() const {
    std::vector<const Task*> sortedTasks;
    std::unordered_set<const Task*> visited;
    std::set<const Task*> added;  // Use std::set for uniqueness

    for (const Task& task : tasks) {
        topologicalSortRecursive(&task, visited, sortedTasks, added);
    }

    return sortedTasks;
}

/**
 * @brief Performs a topological sort recursively on the TaskGraph.
 * @param task Pointer to the Task object.
 * @param visited Set of visited tasks.
 * @param sortedTasks Vector to store the sorted tasks.
 * @param added Set of added tasks to ensure uniqueness.
 */
void TaskGraph::topologicalSortRecursive(const Task* task, std::unordered_set<const Task*>& visited, std::vector<const Task*>& sortedTasks, std::set<const Task*>& added) const {
    if (visited.find(task) == visited.end()) {
        visited.insert(task);

        const auto& dependencies = task->getDependencies();
        for (const Task* dependency : dependencies) {
            topologicalSortRecursive(dependency, visited, sortedTasks, added);
        }

        // Add the task to the sorted list only if it hasn't been added before
        if (added.find(task) == added.end()) {
            sortedTasks.push_back(task);
            added.insert(task);
        }
    }
}
