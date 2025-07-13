#include <string>
#include <iostream>
#include <string.h>
#include <stdio.h>

template<typename Node>
struct NodeIteratorT {
    using value_type = Node;
    using difference_type = void*;
    using pointer = Node*;
    using reference = Node&;

    pointer ptr;
    const pointer ptr_begin;

    constexpr NodeIteratorT(pointer p)
        :ptr(p),ptr_begin(p) { }

    constexpr inline reference operator*() const { return *ptr; }
    constexpr inline pointer operator->() const { return ptr; }

    constexpr inline NodeIteratorT begin() const { return NodeIteratorT(ptr_begin); }

    constexpr inline NodeIteratorT end() const { return NodeIteratorT(nullptr); }

    static constexpr inline Node *last_sibling_r(Node *node) {
        if (node->sibling)
            return last_sibling_r(node->sibling);
        return node;
    }

    static constexpr inline Node *last_sibling(Node *node) {
        while (node->sibling) node = node->sibling;
        return node;
    }

    static constexpr inline Node *deepest_child(Node *node) {
        while (node->child) node = node->child;
        return node;
    }

    static constexpr inline Node *root_node(Node *node) {
        while (node->parent) node = node->parent;
        return node;
    }

    static constexpr inline Node *previous_sibling(Node *node) {
        if (!node->parent || !node->parent->child || node->parent->child == node)
            return nullptr;
        const Node *start = node;
        node = node->parent->child;
        while (node->sibling != start && node->sibling)
            node = node->sbiling;
        return node;
    }

    static constexpr inline Node *depth_first_next(Node *node) {
        Node *next = node->sibling;

        if (!next)
            return node->parent;

        if (next->child)
            return deepest_child(next);

        return next;
    }

    static constexpr inline Node *breadth_first_next(Node *node) {
        Node *next = node->child ? node->child : node->sibling;

        if (!next && node->parent)
            return node->parent->sibling;
            
        return next;
    }

    static constexpr inline int node_depth(Node *node) {
        int i = 0;
        for (;node->parent;node = node->parent, i++);
        return i;
    }

    constexpr inline bool operator==(const NodeIteratorT &other) const { return ptr == other.ptr; }
    constexpr inline bool operator!=(const NodeIteratorT &other) const { return ptr != other.ptr; }
};

template<typename Node, typename NodeIterator = NodeIteratorT<Node>>
struct ChildIteratorT : public NodeIterator {
    using NodeIterator::ptr;

    constexpr ChildIteratorT(Node *p)
        :NodeIterator(p->child) { }

    constexpr inline ChildIteratorT &operator++() {
        ptr = ptr->sibling;
        return *this;
    }
};

template<typename Node, typename NodeIterator = NodeIteratorT<Node>>
struct ReverseChildIteratorT : public NodeIterator {
    using NodeIterator::ptr;

    constexpr ReverseChildIteratorT(Node *p)
        :NodeIterator(NodeIterator::last_sibling(p)) { }

    constexpr inline ReverseChildIteratorT &operator++() {
        ptr = NodeIterator::previous_sibling(ptr);
        return *this;
    }
};

template<typename Node, typename NodeIterator = NodeIteratorT<Node>>
struct DepthFirstIteratorT : public NodeIterator {
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr DepthFirstIteratorT(Node *p)
        :NodeIterator(NodeIterator::deepest_child(p)) { }

    constexpr inline DepthFirstIteratorT &operator++() {
        ptr = NodeIterator::depth_first_next(ptr);
        return *this;
    }
};

template<typename Node, typename NodeIterator = NodeIteratorT<Node>>
struct BreadthFirstIteratorT : public NodeIterator {
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline BreadthFirstIteratorT &operator++() {
        ptr = NodeIterator::breadth_first_next(ptr);
        return *this;
    }
};

//template<typename NodeIter> requires ( std::is_base_of_v<NodeIteratorT<typename NodeIter::value_type>, NodeIter> )
template<typename NodeIter, typename = std::is_base_of<NodeIteratorT<typename NodeIter::value_type>, NodeIter>>
static constexpr inline NodeIter operator++(NodeIter &src) {
    NodeIter temp = src;
    ++(src);
    return temp;
}

struct Node;
using NodeIterator = NodeIteratorT<Node>;
using ChildIterator = ChildIteratorT<Node, NodeIterator>;
using ReverseChildIterator = ReverseChildIteratorT<Node, NodeIterator>;
using DepthFirstIterator = DepthFirstIteratorT<Node, NodeIterator>;
using BreadthFirstIterator = BreadthFirstIteratorT<Node, NodeIterator>;

struct Node {
    const char *name;

    Node *parent, *child, *sibling;

    constexpr Node(const char *name = nullptr, Node *parent = nullptr, Node *child = nullptr, Node *sibling = nullptr)
        :name(name),parent(parent),child(child),sibling(sibling) { }

    constexpr inline Node *append_sibling(Node *node) {
        Node *neighbor = NodeIterator::last_sibling(this);

        neighbor->sibling = node;
        neighbor->parent = parent;

        return node;
    }

    constexpr inline Node *append_child(Node *node) {
        if (child)
            return child->append_sibling(node);
        
        node->parent = this;
        return child = node;
    }

    constexpr inline Node &append_sibling(Node &node) { return *append_sibling(&node); }
    constexpr inline Node &append_child(Node &node) { return *append_child(&node); }
    constexpr inline Node &operator<<(Node &node) { return append_child(node); }
    constexpr inline Node &operator<<(Node *node) { return *append_child(node); }

    constexpr inline ChildIterator children() { return ChildIterator(this); }
    constexpr inline ReverseChildIterator rchildren() { return ReverseChildIterator(this); }
    constexpr inline DepthFirstIterator depth_first() { return DepthFirstIterator(this); }
    constexpr inline BreadthFirstIterator breadth_first() { return BreadthFirstIterator(this); }

    inline std::string get_name(const int &pad_left = 0, const char *class_name = "Node") const {
        std::string prepend(pad_left, ' ');

        return prepend + class_name + ": " + (this ? name : "null");
    }
};

void append_nodes(Node &node, const int &breadth, const int &depth, const int &breadthsub=1, const int &depthsub=1) {
    std::string depth_name(std::to_string(depth));
    
    for (int i = 0; i < breadth; i++) {
        std::string node_name = depth_name + "." + std::to_string(breadth);
        const char *name = strcpy(new char[node_name.size()+1], node_name.c_str());
        Node *n = new Node(name);
        node << n;
        append_nodes(*n, breadth-breadthsub, depth-depthsub);
    }
}

template<typename Iter>
void print_nodes(Node &root) {
    Iter iterator(&root);
    for (Node &node : iterator)
        std::cout << node.get_name(NodeIterator::node_depth(&node)) << std::endl;
}

int main() {
    Node root("0");

    append_nodes(root, 3, 3);

    print_nodes<ChildIterator>(root);
    print_nodes<ReverseChildIterator>(root);
    print_nodes<DepthFirstIterator>(root);
    print_nodes<BreadthFirstIterator>(root);

    return 0;
}