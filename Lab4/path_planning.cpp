#include "path_planning.hpp"
#include "thread_pool.hpp"

#include <iostream>
#include <mutex>
#include <semaphore>
#include <thread>

enum Mode
{
    INPUT,
    RANDOM,
    SINGLE
};

// Helper function to update the map with the path value
void update_map_with_path(Map &map, const std::vector<Node> &path,
                          uint8_t thread_id)
{
    for (const Node &node : path)
    {
        map[node.x][node.y] = thread_id + 2;
    }
}

// Helper function to print the map to stdout
void print_map(const Map &map)
{
    std::cout << map.size() << " " << map[0].size() << std::endl;
    for (const std::vector<uint8_t> &row : map)
    {
        for (uint8_t value : row)
        {
            std::cout << static_cast<int>(value) << " ";
        }
        std::cout << std::endl;
    }
}

// Worker thread function to find a path from start to goal
void path_planner_worker_thread(PathPlanner &path_planner,
                                int16_t start_x,
                                int16_t start_y,
                                int16_t goal_x,
                                int16_t goal_y,
                                std::queue<std::vector<Node>> &paths,
                                std::mutex &mutex,
                                std::counting_semaphore<255> &sem)
{
    std::vector<Node> path;
    // TODO: Use the path_planner to find a path from start to goal.
    path = path_planner.find_path(start_x, start_y, goal_x, goal_y);
    // TODO: Declare a std::lock_guard with the mutex to protect access to the
    //       paths queue.
    std::lock_guard<std::mutex> lock(mutex);
    // TODO: If the path is empty, release the semaphore and return.
    if (path.empty())
    {
        sem.release();
        return;
    }
    // TODO: Push the path to the paths queue and release the semaphore.
    paths.push(std::move(path));
    sem.release();
}

// Function to find paths from multiple start to goal coordinates
void multithread_path_plan(std::vector<std::pair<int16_t, int16_t>> &start,
                           std::vector<std::pair<int16_t, int16_t>> &goal,
                           PathPlanner &path_planner)
{
    size_t num_plans = start.size();
    size_t num_threads = std::thread::hardware_concurrency();
    ThreadPool pool(num_threads);

    std::mutex mutex;
    std::counting_semaphore<255> sem(0);
    std::queue<std::vector<Node>> paths;

    // Enqueue a task for each start and goal coordinate pair
    for (size_t i = 0; i < num_plans; ++i)
    {
        pool.enqueue(path_planner_worker_thread, std::ref(path_planner),
                     start[i].first,
                     start[i].second,
                     goal[i].first,
                     goal[i].second,
                     std::ref(paths),
                     std::ref(mutex),
                     std::ref(sem));
    }

    size_t threads_finished = 0;
    while (threads_finished < num_plans)
    {
        // TODO: Acquire the semaphore and inrcrement the threads_finished
        //       counter.
        sem.acquire();
        threads_finished++;

        std::vector<Node> path;
        {
            // TODO: Declare a std::lock_guard with the mutex to protect access
            //       to the paths queue.
            std::lock_guard<std::mutex> lock(mutex);

            // TODO: If the paths queue is empty, continue to the next
            //       iteration.
            if (paths.empty())
            {
                continue;
            }

            // TODO: Get the path from the front of the queue and pop it.
            // Hint: Use std::move to transfer ownership (this is faster than
            //       copying).
            path = std::move(paths.front());

            // TODO: Pop the path from the queue.
            paths.pop();
        }

        // Update the map with the path found by the thread
        update_map_with_path(path_planner.get_map(), path, threads_finished % 255);
    }
}

int main(int argc, char *argv[])
{
    // Check the number of arguments
    if (argc != 2 && argc != 3 && argc != 6)
    {
        std::cerr << "Usage  (INPUT): " << argv[0] << " <filename>" << std::endl;
        std::cerr << "Usage (RANDOM): " << argv[0] << " <filename> <num_coords>" << std::endl;
        std::cerr << "Usage (SINGLE): " << argv[0] << " <filename> <start_x> <start_y> <goal_x> <goal_y>" << std::endl;
        return 1;
    }

    // Parse the arguments
    std::string graph_file = argv[1];
    int num_coords;
    int16_t start_x, start_y, goal_x, goal_y;
    Mode mode;
    if (argc == 2)
    {
        mode = INPUT; // Input mode (read from stdin)
    }
    else if (argc == 3)
    {
        mode = RANDOM; // Random mode (generate random start and goal coordinates)
        num_coords = std::stoi(argv[2]);
    }
    else if (argc == 6)
    {
        mode = SINGLE; // Single mode (find path from start to goal)
        start_x = std::stoi(argv[2]);
        start_y = std::stoi(argv[3]);
        goal_x = std::stoi(argv[4]);
        goal_y = std::stoi(argv[5]);
    }

    PathPlanner path_planner(graph_file);

    if (mode == RANDOM)
    {
        std::vector<std::pair<int16_t, int16_t>> start(num_coords);
        std::vector<std::pair<int16_t, int16_t>> goal(num_coords);

        srand(time(nullptr));
        for (int i = 0; i < num_coords; i++)
        {
            do
            {
                start[i].first = rand() % path_planner.get_map().size();
                start[i].second = rand() % path_planner.get_map()[0].size();
            } while (path_planner.is_occupied(start[i].first, start[i].second));

            do
            {
                goal[i].first = rand() % path_planner.get_map().size();
                goal[i].second = rand() % path_planner.get_map()[0].size();
            } while (path_planner.is_occupied(goal[i].first, goal[i].second));
        }

        multithread_path_plan(start, goal, path_planner);
    }
    else if (mode == SINGLE)
    {
        std::cout << "Start: " << start_x << " " << start_y << std::endl;
        std::cout << "Goal: " << goal_x << " " << goal_y << std::endl;
        std::vector<Node> path =
            path_planner.find_path(start_x, start_y, goal_x, goal_y);
        if (path.empty())
        {
            std::cout << "No path found" << std::endl;
            return 1;
        }

        update_map_with_path(path_planner.get_map(), path, 2);
    }
    else if (mode == INPUT)
    {
        size_t num_coordinates;
        std::cin >> num_coordinates;
        std::vector<std::pair<int16_t, int16_t>> start(num_coordinates);
        std::vector<std::pair<int16_t, int16_t>> goal(num_coordinates);

        for (size_t i = 0; i < num_coordinates; i++)
        {
            int16_t start_x, start_y, goal_x, goal_y;
            std::cin >> start_x >> start_y >> goal_x >> goal_y;
            if (start_x < 0 || start_x >= path_planner.get_map().size() ||
                start_y < 0 || start_y >= path_planner.get_map()[0].size() ||
                goal_x < 0 || goal_x >= path_planner.get_map().size() ||
                goal_y < 0 || goal_y >= path_planner.get_map()[0].size())
            {
                std::cerr << "Error: Coordinates out of bounds for map with size "
                          << path_planner.get_map().size() << "x"
                          << path_planner.get_map()[0].size() << std::endl;
                return 1;
            }

            if (path_planner.is_occupied(start_x, start_y) ||
                path_planner.is_occupied(goal_x, goal_y))
            {
                std::cerr << "Error: start or goal node is blocked" << std::endl;
                return 1;
            }

            start[i] = {start_x, start_y};
            goal[i] = {goal_x, goal_y};
        }

        multithread_path_plan(start, goal, path_planner);

        // Convert all values in the map greater than or equal to 2 to 2
        std::vector<std::vector<uint8_t>> &map = path_planner.get_map();
        for (std::vector<uint8_t> &row : map)
        {
            for (uint8_t &value : row)
            {
                if (value >= 2)
                {
                    value = 2;
                }
            }
        }
    }

    print_map(path_planner.get_map());

    return 0;
}