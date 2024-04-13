#include <iostream>
#include <fstream>
#include <regex>
using namespace std;

/* 
The node holds data and has links related to RBT 
Heap utils only change the `index_in_heap_vector`

Working:
We have 3 classes HeapUtils, RBTUtils and HeapRBT.
HeapUtils
    1. Implements the heap functionality and ordered on ride_cost and trip_duration
    2. Stores the reference to the nodes in the array
    3. Stores the index of the node in the heap-array in the node itself as a 'lookup' or mapping.
RBTUtils
    1. Implements RBT insert and delete methods with ride_number as the key
    2. The delete method has modifications from the usual internet implementation
     as we don't just want to retain the data but the node reference as well.
HeapRBT
    1. Co-ordinates between the main driver which reads files and does operations and
        the datastructures
    2. Keeps RBT and Heap in check. E.g insert first in RBT and also in Heap if RBT insert is successful

How each operation works:
Print x:        Traverse the RBT (log(n))
Print a, b:     Traverse the RBT recursively (log(n) + S)
Insert a, b, c: Insert in RBT (log(n)) and in Heap (log(n))
CancelRide x:   Get ride from RBT (log(n)), 
                Lookup Heap index (O(1)) and delete from heap (log(n)), 
                delete from RBT (log(n))
UpdateTrip x, u:Get rdie from RBT (log(n)),
                Lookup Heap index (O(1)) and update/delete from heap
GetNextRide:    Get next ride from heap (O(1)),
                Delete ride from RBT using the binary search (log(n))
                Delete from heap (log(n))
*/
typedef struct Node {
  struct Node *left, *right;
  enum { R, B } color;
  int ride_number;
  int ride_cost;
  int trip_duration;
  int index_in_heap_vector;  // points to where we are located in the heap array

  Node(int id, int cost, int duration) {
    left = nullptr;
    right = nullptr;
    color = R;
    ride_number = id;
    ride_cost = cost;
    trip_duration = duration;
  }

  bool is_red() { return color == R; }

  void debug() {
    cout << "RBT: " << left << " " << right << " " << color << endl;
    cout << "Ride: " << ride_number << " " << ride_cost << " " << trip_duration
         << endl;
  }

  void swap_index(Node* other) {
    int temp = index_in_heap_vector;
    index_in_heap_vector = other->index_in_heap_vector;
    other->index_in_heap_vector = temp;
  }

  bool less(Node* other) {
    if (ride_cost == other->ride_cost) {
      return trip_duration < other->trip_duration;
    }
    return ride_cost < other->ride_cost;
  }
} Node;

class HeapUtils {
 public:
  vector<Node*> heap_vector;
  int sz;
  HeapUtils() {
    heap_vector = vector<Node*>(2002, nullptr);
    sz = -1;
  }
  bool less(int a, int b) {
    // returns true if heap[a] < heap[b];
    return heap_vector[a]->less(heap_vector[b]);
  }

  void swap_nodes(int a, int b) {
    swap(heap_vector[a], heap_vector[b]);
    heap_vector[a]->swap_index(heap_vector[b]);
  }
  void fix_heap_up(int x) {
    while (x > 0 && less(x, x / 2)) {
      swap_nodes(x, x / 2);
      x /= 2;
    }
  }
  void fix_heap_down(int x) {
    while (2 * x + 1 <= sz) {
      int y = 2 * x + 1;  // left child
      if (y < sz && less(y + 1, y))
        ++y;
      if (less(x, y))
        break;
      swap_nodes(x, y);
      x = y;
    }
  }
  Node* pop_min() {
    if (sz == -1) {
      return nullptr;
    }
    Node* ret = heap_vector[0];
    swap_nodes(0, sz);
    heap_vector[sz] = nullptr;
    --sz;
    fix_heap_down(0);
    return ret;
  }
  void insert(Node* x) {
    heap_vector[++sz] = x;
    x->index_in_heap_vector = sz;
    fix_heap_up(sz);
  }

  void update_by_index(int ex) {
    // can never go up because we're only increasing the cost;
    fix_heap_down(ex);
  }

  void delete_by_index(int ex) {
    swap_nodes(ex, --sz);
    fix_heap_down(ex);
  }
};

class RBTUtils {
 public:
  Node* root;
  bool failure;
  RBTUtils() : root(nullptr) {}
  bool insert(Node* z) {
    root = insert(root, z);
    root->color = Node::B;
    if (failure) {
      failure = false;
      return false;
    }
    return true;
  }

  bool is_red(Node* x) {
    if (x == nullptr) {
      return false;
    }
    return x->color == Node::R;
  }

  void toggle_color(Node* e) {
    if (e == nullptr) {
      return;
    }
    if (e->color == Node::R) {
      e->color = Node::B;
    } else {
      e->color = Node::R;
    }
  }

  void fix_colors(Node* e) {
    if (e == nullptr || e->left == nullptr || e->left == nullptr) {
      cout << "Bad fix colors" << endl;
      return;
    }
    toggle_color(e);
    toggle_color(e->left);
    toggle_color(e->right);
  }

  Node* move_left(Node* z) {
    if (!((z != nullptr) && (z->left != nullptr) && (z->right != nullptr))) {
      cout << "bad move left" << endl;
      return z;
    }
    if (!((!is_red(z) && is_red(z->left) && is_red(z->right)) ||
          (is_red(z) && !is_red(z->left) && !is_red(z->right)))) {
      cout << "bad move left" << endl;
      return z;
    }
    fix_colors(z);
    if (is_red(z->right->left)) {
      z->right = rotate_right(z->right);
      z = rotate_left(z);
      fix_colors(z);
    }
    return z;
  }

  Node* move_right(Node* z) {
    if (z == nullptr || !is_red(z) || is_red(z->right) ||
        is_red(z->right->left)) {
      cout << "bad move right" << endl;
      return z;
    }
    fix_colors(z);
    if (is_red(z->left->left)) {
      z = rotate_right(z);
      fix_colors(z);
    }
    return z;
  }

  Node* rotate_right(Node* z) {
    if (z == nullptr || !is_red(z->left)) {
      cout << "Bad right rotate" << endl;
      return z;
    }
    Node* x = z->left;
    z->left = x->right;
    x->right = z;
    x->color = z->color;
    z->color = Node::R;
    return x;
  }

  Node* rotate_left(Node* z) {
    if (z == nullptr || !is_red(z->right)) {
      cout << "Bad left rotate" << endl;
      return z;
    }
    Node* x = z->right;
    z->right = x->left;
    x->left = z;
    x->color = z->color;
    z->color = Node::R;
    return x;
  }

  Node* re_balance(Node* x) {
    if (is_red(x->right) && !is_red(x->left)) {
      x = rotate_left(x);
    }
    if (is_red(x->left) && is_red(x->left->left)) {
      x = rotate_right(x);
    }
    if (is_red(x->left) && is_red(x->right)) {
      fix_colors(x);
    }
    return x;
  }

  Node* insert(Node* x, Node* z) {
    if (x == nullptr) {
      z->color = Node::R;
      return z;
    }
    if (x->ride_number == z->ride_number) {
      failure = true;
      return x;
    }
    if (x->ride_number < z->ride_number) {
      x->right = insert(x->right, z);
    } else {
      x->left = insert(x->left, z);
    }
    return re_balance(x);
  }

  Node* min_node(Node* z) {
    if (z->left == nullptr) {
      return z;
    }
    return min_node(z->left);
  }

  Node* delete_min(Node* z) {
    if (z->left == nullptr) {
      return nullptr;
    }
    if (!is_red(z->left) && !is_red(z->left->left)) {
      z = move_left(z);
    }
    z->left = delete_min(z->left);
    return re_balance(z);
  }

  Node* delete_by_id(Node* z, int id) {
    if (id < z->ride_number) {
      if (!is_red(z->left) && !is_red(z->left->left)) {
        z = move_left(z);
      }
      z->left = delete_by_id(z->left, id);
    } else {
      if (is_red(z->left)) {
        z = rotate_right(z);
      }
      if (id == z->ride_number && z->right == nullptr) {
        return nullptr;
      }
      if (!is_red(z->right) && !is_red(z->right->left)) {
        z = move_right(z);
      }
      if (id == z->ride_number) {
        Node* x = min_node(z->right);
        x->right = delete_min(z->right);
        x->left = z->left;
        z = x;
      } else {
        z->right = delete_by_id(z->right, id);
      }
    }
    return re_balance(z);
  }

  void delete_by_id(int id) {
    if (!is_red(root->left) && !is_red(root->right)) {
      root->color = Node::R;
    }
    root = delete_by_id(root, id);
  }

  // Debug and testing functions
  vector<int> inorder() {
    vector<int> res;
    get_inorder(root, res);
    return res;
  }

  void get_inorder(Node* z, vector<int>& res) {
    if (z == nullptr) {
      return;
    }
    get_inorder(z->left, res);
    res.push_back(z->ride_number);
    get_inorder(z->right, res);
  }

  void printTree(const std::string& prefix, Node* node, bool isLeft) {
    if (node != nullptr) {
      std::cout << prefix;

      std::cout << (isLeft ? "├──" : "└──");

      // print the value of the node
      std::cout << node->ride_number << "(" << (is_red(node) ? "R" : "B") << ")"
                << std::endl;

      // enter the next tree level - left and right branch
      printTree(prefix + (isLeft ? "│   " : "    "), node->left, true);
      printTree(prefix + (isLeft ? "│   " : "    "), node->right, false);
    }
  }

  void printRBT() { printTree("", root, false); }

  Node* get_node(int id) {
    Node* rc = root;
    while (rc != nullptr) {
      if (rc->ride_number < id) {
        rc = rc->right;
      } else if (rc->ride_number > id) {
        rc = rc->left;
      } else {
        return rc;
      }
    }
    return nullptr;
  }

  // For printing nodes in range
  void get_nodes_in_range(Node* x, int lo, int hi, vector<Node*>& res) {
    if (x == nullptr) {
      return;
    }
    if (x->ride_number < lo || x->ride_number > hi) {
      return;
    }
    get_nodes_in_range(x->left, lo, hi, res);
    res.push_back(x);
    get_nodes_in_range(x->right, lo, hi, res);
  }

  vector<Node*> get_nodes(int lo, int hi) {
    vector<Node*> res;
    get_nodes_in_range(root, lo, hi, res);
    return res;
  }
};

// Implements both data structures using utils and supports operations.
class HeapRBT : public HeapUtils, public RBTUtils {
 public:
  ofstream& file_out;
  HeapRBT(ofstream& out) : file_out(out) {}
  void print_node(Node* node, bool newline, bool comma) {
    file_out << "(" << node->ride_number << "," << node->ride_cost << ","
             << node->trip_duration << (comma ? ")," : ")");
    if (newline) {
      file_out << endl;
    }
  }
  void print(int ride_number) {
    Node* node = RBTUtils::get_node(ride_number);
    if (node == nullptr) {
      file_out << "(0,0,0)" << endl;
      return;
    }
    print_node(node, true, false);
  }
  void print(int ride_number_start, int ride_number_end) {
    vector<Node*> nodes =
        RBTUtils::get_nodes(ride_number_start, ride_number_end);
    int n = nodes.size();
    if (n == 0) {
      file_out << "(0, 0, 0)" << endl;
      return;
    }
    Node* node;
    for (int i = 0; i < n; ++i) {
      node = nodes[i];
      print_node(node, false, i != n - 1);
    }
    file_out << "\n";
  }
  void insert(int ride_number, int ride_cost, int trip_duration) {
    Node* z = new Node(ride_number, ride_cost, trip_duration);
    bool inserted = RBTUtils::insert(z);
    if (!inserted) {
      file_out << "Duplicate RideNumber" << endl;
      exit(1);
    }
    HeapUtils::insert(z);
  }
  void get_next_ride() {
    Node* ride = HeapUtils::pop_min();
    if (ride != nullptr) {
      // Crucial to print before deleting as RBT "delete" only changes the
      // values not the Node itself.
      print_node(ride, true, false);
      RBTUtils::delete_by_id(ride->ride_number);
    } else {
      file_out << "No active ride requests" << endl;
    }
  }
  void cancel_ride(int ride_number) {
    Node* ride = RBTUtils::get_node(ride_number);
    if (ride != nullptr) {
      HeapUtils::delete_by_index(ride->index_in_heap_vector);
      RBTUtils::delete_by_id(ride_number);
    }
  }

  void update_trip(int ride_number, int new_trip_duration) {
    Node* ride = RBTUtils::get_node(ride_number);
    if (ride == nullptr) {
      return;
    }
    int td = ride->trip_duration;
    if (new_trip_duration > 2 * td) {
      HeapUtils::delete_by_index(ride->index_in_heap_vector);
      RBTUtils::delete_by_id(ride_number);
      return;
    }
    if (new_trip_duration > td) {
      ride->ride_cost += 10;
      ride->trip_duration = new_trip_duration;
      HeapUtils::update_by_index(ride->index_in_heap_vector);
    }
  }

  // test if RBT is working fine
  void test_rbt_sorted() {
    int max = 1000;
    int min = 20;
    for (int test = 0; test < 10; ++test) {
      int sz = rand() % 50 + 50;
      for (int i = 0; i < sz; ++i) {
        int a = rand() % (max - min + 1) + min;
        int b = rand() % (max - min + 1) + min;
        insert(i, a, b);
      }
      vector<int> v = RBTUtils::inorder();
      assert((int)v.size() == sz);
      assert(is_sorted(v.begin(), v.end()));
    //   RBTUtils::printRBT();
      for (int x : v) {
        RBTUtils::delete_by_id(x);
      }
    }
    cout << "Test for sorting passed!" << endl;
  }
};

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Please pass file name as the argument" << endl;
    return 0;
  }
  // Read and process file
  string file_name(argv[1]);
  ifstream in(file_name);
  ofstream out("output_file.txt");
  HeapRBT h_rbt = HeapRBT(out);
  string line;

  // split on paranthesis, comma and spaces
  regex re("[(|)|,| ]");
  while (getline(in, line)) {
    sregex_token_iterator first(line.begin(), line.end(), re, -1), last;
    vector<string> tokens_raw(first, last);
    // remove empty strings due to spaces
    vector<string> tokens;
    for (string x : tokens_raw) {
      if (x.length() > 0) {
        tokens.push_back(x);
      }
    }

    string fn = tokens[0];
    if (fn == "Insert") {
      int ride_number = stoi(tokens[1]);
      int ride_cost = stoi(tokens[2]);
      int trip_duration = stoi(tokens[3]);
      h_rbt.insert(ride_number, ride_cost, trip_duration);
    } else if (fn == "Print") {
      if ((int)tokens.size() == 2) {
        int ride_number = stoi(tokens[1]);
        h_rbt.print(ride_number);
      } else {
        int start = stoi(tokens[1]);
        int end = stoi(tokens[2]);
        h_rbt.print(start, end);
      }
    } else if (fn == "GetNextRide") {
      h_rbt.get_next_ride();
    } else if (fn == "CancelRide") {
      int ride_number = stoi(tokens[1]);
      h_rbt.cancel_ride(ride_number);
    } else if (fn == "UpdateTrip") {
      int ride_number = stoi(tokens[1]);
      int trip_duration = stoi(tokens[2]);
      h_rbt.update_trip(ride_number, trip_duration);
    }
  }

  // RBT Testcases to check if it's working fine    
  h_rbt.test_rbt_sorted();

  in.close();
  out.close();
  return 0;
}