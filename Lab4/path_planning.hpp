#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>

using Map = std::vector<std::vector<uint8_t>>;

struct Node {
    int16_t x, y;
    float f, g;
    int16_t parent_x, parent_y;

    Node()
        : x(0),
          y(0),
          f(0),
          g(std::numeric_limits<float>::max()),
          parent_x(0),
          parent_y(0) {}
    Node(const Node& node)
        : x(node.x),
          y(node.y),
          f(node.f),
          g(node.g),
          parent_x(node.parent_x),
          parent_y(node.parent_y) {}
    Node(int16_t x, int16_t y)
        : x(x),
          y(y),
          f(0),
          g(std::numeric_limits<float>::max()),
          parent_x(0),
          parent_y(0) {}

    Node& operator=(const Node& node) {
        if (this == &node) {
            return *this;
        }
        x = node.x;
        y = node.y;
        f = node.f;
        g = node.g;
        parent_x = node.parent_x;
        parent_y = node.parent_y;
        return *this;
    }

    bool operator==(const Node& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator<(const Node& rhs) const { return f < rhs.f; }
    bool operator>(const Node& rhs) const { return f > rhs.f; }
};

class PairHash {
   public:
    size_t operator()(const std::pair<int16_t, int16_t>& pair) const {
        return ((size_t)pair.first << 16) | ((size_t)pair.second & 0xFFFF);
    }
};

class PathPlanner {
   public:
    PathPlanner(const Map& map);
    PathPlanner(const std::string& filename);
    static Map read_map(const std::string& filename);
    std::vector<Node> find_path(int16_t start_x, int16_t start_y, int16_t goal_x, int16_t goal_y) const;
    static void print_path(const std::vector<Node>& path);
    Map& get_map() { return map; }
    const Map& get_map() const { return map; }
    bool is_occupied(int16_t x, int16_t y) const { return map[x][y] == 0; }

   private:
    Map map;

    std::vector<Node> get_neighbors(const Node& node) const;
    static float distance(const Node& a, const Node& b);
    static float heuristic(const Node& node, const Node& goal);
};

PathPlanner::PathPlanner(const Map& map)
    : map(map) {}

PathPlanner::PathPlanner(const std::string& filename)
    : map(read_map(filename)) {}


Map PathPlanner::read_map(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << std::endl;
        exit(1);
    }

    int rows, cols;
    file >> rows >> cols;

    Map map(rows, std::vector<uint8_t>(cols, 0));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int value;
            file >> value;
            map[i][j] = static_cast<uint8_t>(value);
        }
    }

    return map;
}

std::vector<Node> PathPlanner::find_path(int16_t start_x, int16_t start_y, int16_t goal_x, int16_t goal_y) const {
    // Check if the start and goal coordinates are within the map bounds
    if (start_x < 0 || start_x >= map.size() || start_y < 0 ||
        start_y >= map[0].size() || goal_x < 0 || goal_x >= map.size() ||
        goal_y < 0 || goal_y >= map[0].size()) {
        std::cerr << "Error: Coordinates out of bounds for map with size "
                  << map.size() << "x" << map[0].size() << std::endl;
        return {};
    }

    // Check that the start and goal nodes are not obstructed
    if (map[start_x][start_y] == 0 || map[goal_x][goal_y] == 0) {
        std::cerr << "Error: start or goal node is blocked" << std::endl;
        return {};
    }

    // Declare a priority queue to store nodes to explore
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> queue;

    // Declare a set to store visited nodes
    std::unordered_map<std::pair<int16_t, int16_t>, Node, PairHash> visited;

    Node goal(goal_x, goal_y);
    Node start(start_x, start_y);
    start.g = 0;
    start.f = heuristic(start, goal);
    queue.push(start);

    // Explore nodes until the queue is empty
    while (!queue.empty()) {
        // Get the node with the lowest f value (top of the priority queue)
        Node current = queue.top();
        queue.pop();

        // Check if the node has already been visited, if so skip it
        if (visited.count({current.x, current.y}) > 0) {
            continue;
        }
        
        // Check if the current node is the goal node
        if (current == goal) {
            // Reconstruct the path from the goal node to the start node
            std::vector<Node> path;
            while (!(current.x == start.x && current.y == start.y)) {
                path.push_back(current);
                current = visited.at({current.parent_x, current.parent_y});
            }
            // Reverse the path to get the correct order
            std::reverse(path.begin(), path.end());
            return path;
        }
        // Mark the current node as visited
        visited.insert({{current.x, current.y}, current});

        // Get the neighbors of the current node
        std::vector<Node> neighbors = get_neighbors(current);
        for (Node& neighbor : neighbors) {
            // Calculate the g value for the neighbor node
            double tentative_g = current.g + distance(current, neighbor);
            // If the neighbor has not been visited or the new g value is less 
            // than the current g value of the neighbor, update the neighbor
            if (tentative_g < neighbor.g ||
                visited.count({neighbor.x, neighbor.y}) == 0) {
                neighbor.g = tentative_g;
                neighbor.f = neighbor.g + heuristic(neighbor, goal);
                neighbor.parent_x = current.x;
                neighbor.parent_y = current.y;
                // Add the neighbor to the queue
                queue.push(neighbor);
            }
        }
    }

    return {};
}

std::vector<Node> PathPlanner::get_neighbors(const Node& node) const {
    std::vector<Node> neighbors;
    // West, East, North, South
    // Check if the neighbor is within the map bounds and is not obstructed
    if (node.x > 0 && map[node.x - 1][node.y]) {
        neighbors.push_back(Node(node.x - 1, node.y));
    }
    if (node.x < map.size() - 1 && map[node.x + 1][node.y]) {
        neighbors.push_back(Node(node.x + 1, node.y));
    }
    if (node.y > 0 && map[node.x][node.y - 1]) {
        neighbors.push_back(Node(node.x, node.y - 1));
    }
    if (node.y < map[0].size() - 1 && map[node.x][node.y + 1]) {
        neighbors.push_back(Node(node.x, node.y + 1));
    }
    return neighbors;
}

float PathPlanner::distance(const Node& a, const Node& b) {
    // Euclidean distance
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

float PathPlanner::heuristic(const Node& node, const Node& goal) {
    return distance(node, goal);
}