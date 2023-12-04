#include "mcts.h"

#include <vector>
#include <cmath>
#include <climits>

#include "eval.h"
#include "threads.h"

// Global evaluator
extern eval_t eval;

class Node {

    // Search should have access to all private members
    friend void MCTS_Search(board_t* board, searchinfo_t *info);

public:
    // Constructor
    Node(const board_t* board, move_t mv, Node* parent_node)
        : parent(parent_node)
        , a(mv)
        , total_reward(0)
        , visits(0)
    {
        // Generate all possible actions (chess moves) from this node
        generate_moves(board, &this->untried_moves);
    }

    // Destructor
    ~Node() {
        for (Node* child : this->children) {
            delete child;
        }
    }

    Node* insert_child(move_t mv, const board_t* board);
    Node* best_child(bool exploration_mode = true);
    void update(double res); // backprop update (increment visits etc.)
    inline bool is_terminal() {
        return this->children.size() == 0;
    }

    inline bool is_fully_expanded() {
        // REVIEW: Are leaves considered fully expanded?
        return this->untried_moves.size() == 0;
    }

    Node *parent;
    std::vector<Node *> children;
    // action that got us to this node (for performance reasons only the root
    // stores the actual board state)
    Action a;
    movelist_t untried_moves;
private:
    int total_reward;
    int visits;
};


/* TODO:
std::ostream& operator << (std::ostream &o, const Node* node) {
    return o << "Node; " << node->children.size() << " children; " \
    << node->visits << " visits; " << node->total_reward << " reward";
}
*/


// REVIEW: Here, we could experiment with multiple rollout policies
// and report the results?

[[__always_inline__]] static inline Action random_policy(movelist_t& actions) {
    return actions[rand_uint64() % actions.size()];
}

/**
 * Returns an action to play  during rollout.
 * This could be parametrized w.r.t the current state? NN?
 * For Pure MCTS we use random rollouts
*/
inline Action rollout_policy(movelist_t& actions) {
    return random_policy(actions);
}

constexpr int ROLLOUT_BUDGET = 10;

int rollout(Node *node, State *s) {

    // We limit the number of rollouts (tree height)
    int budget = ROLLOUT_BUDGET;
    Action a;
    while (!node->is_terminal() || budget --> 0) {
        // Choose action according to rollout policy
        a = rollout_policy(node->untried_moves);

        make_move(s, a); // TODO: undo_move? Might not be legal
        node = node->insert_child(a, s);
    }

    /* Leaf state's reward */

    // 1) If terminal, check who won the rollout 
    if (node->is_terminal()) {
        // If side to turn (us?) is in check and node is terminal (no moves),
        // we've been mated (we get a centipawn score of negative infinity,
        // equivalent to a zero probability of winning)
        if (is_in_check(s, s->turn)) {
            return -oo;
        } else if (is_in_check(s, s->turn ^ 1)) { // if opponent got mated
            return +oo;
        } else {
            return 0; // stalemate (i.e. draw)
        }
    }

    // 2) Otherwise, use the static evaluation function as a heuristic
    return evaluate(s, &eval);
}


void backprop(int reward, Node *node) {
    Node *curr = node;
    while (curr != nullptr) {
        curr->update(reward);
        curr = curr->parent;
    }
}


Node *Node::best_child(bool exploration_mode = true) {
    // Calculate UCB values for all the children and pick the highest
    double ucb;
    double best_value = static_cast<double>(INT_MIN);

    //Node *best;
    std::vector<Node*> best_children;

    for (Node* child : this->children) {

        assert (child->visits > 0);

        // Exploitation term
        ucb = static_cast<double>(child->total_reward) / child->visits;

        // Exploration term (TODO: UCB coefficient?)
        // TODO: potentially tune the CONST here (instead of sqrt(2))
        if(exploration_mode) {
            ucb += 2 * std::sqrt(std::log(this->visits) / child->visits);
        }

        if (ucb > best_value) {
            best_value = ucb;

            //best = child;
            best_children.clear();
            best_children.push_back(child);
        } else if (ucb == best_value) {
            best_children.push_back(child);
        }
        
    }

    assert(best != nullptr);

    // REVIEW: Currently, we consider the first best child encountered
    // An improvement could be to keep a list of them and randomly determine ties
    //return best;

    size_t random_pick = (size_t) rand() % best_children.size();
    return best_children[random_pick];
}

Node *Node::insert_child(move_t move, const board_t *board) {
    Node *child = new Node(board, move, this);

    // Mark move as tried
    for (auto it = this->untried_moves.begin(); it != this->untried_moves.end(); ++it) {
        if (*it == move) {
            untried_moves.erase(it);
            break;
        }
    }

    // Store the child within the node
    this->children.push_back(child);
    return child;
}

void Node::update(double result) {
    this->total_reward += result;
    ++this->visits;
}


/*
To avoid local optima, we can use some parametrized policy
to pick the most 'interesting' areas of the tree to expand
// See: https://xyzml.medium.com/learn-ai-game-playing-algorithm-part-ii-monte-carlo-tree-search-2113896d6072

(Could it be e.g. a Thompson distribution or sth?)

For now, we use a random policy lol
*/

Action prior_prob(movelist_t& actions) {
    return random_policy(actions);
}

// TODO: Review this for correctness
Node *insert_node_with_tree_policy(Node *root, State *s) {
    Node *node = root;
    while (!node->is_terminal()) {
        if (node->is_fully_expanded()) {
            node = node->best_child();
        } else {
            Action a = prior_prob(node->untried_moves);
            // TODO: pseudolegal move might require a validity check
            make_move(s, a);
            Node *child = node->insert_child(a, s);
            return child;
        }
    }
    return node;
}


// Recursively free the subtree rooted at 'root'
static void free_tree(Node *root) {
    for (Node *child : root->children) {
        free_tree(child);
    }
    delete root;
}

/* REVIEW:
    
    Optimization idea: Instead of rebuilding the entire tree everyime
    MCTS_Search() is called, we keep the old tree around (globally?). Depending
    on what moves actually get played, we delete all irrelevant subtrees and
    keep the relevant one.
*/
void MCTS_Search(board_t* board, searchinfo_t *info) {

    assert(check(board));
    assert(info->state == ENGINE_SEARCHING);

    // Set up the MCTS Tree
    Node* root = new Node(board, NULLMV, nullptr);

    // Perform the search
    int reward;
    Node* node;
    while (!search_stopped(info)) {
        // 1) Insert a new node (Selection + Expansion)
        node = insert_node_with_tree_policy(root);

        // 2) MC rollout (Simulation)
        reward = rollout(node);

        // 3) Backprop
        backprop(reward, node, root);
    }

    // Figure out the best move.
    // TODO: We should report the entire seq. of moves
    // Q1: why
    move_t best_move = root->best_child()->a;

    std::cout << "info pv " << move_to_str(best_move) << '\n';

    /* Cleanup */
    delete root; // should recursively delete the entire tree
    info->state = ENGINE_STOPPED;
    assert(check(board));
}
