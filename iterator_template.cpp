#include <string>
#include <iostream>
#include <string.h>
#include <stdio.h>

template<typename Derived, typename Node>
struct NodeIteratorT {
    using value_type = Node;
    using difference_type = void*;
    using pointer = Node*;
    using reference = Node&;

    pointer ptr;
    const pointer ptr_begin;

    constexpr NodeIteratorT(pointer p)
        :ptr(p),ptr_begin(p) { }
    constexpr NodeIteratorT():NodeIteratorT(nullptr) { }
    constexpr NodeIteratorT(const Derived &p):NodeIteratorT(p.ptr) { }

    constexpr inline reference operator*() const { return *ptr; }
    constexpr inline pointer operator->() const { return ptr; }

    constexpr inline Derived begin() const { return Derived(ptr_begin); }

    constexpr inline Derived end() const { return Derived(nullptr); }

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
            node = node->sibling;
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

    static constexpr inline Node *depth_first_reverse_next(Node *node) {
        Node *next = node->child ? node->child : node->sibling;

        if (!next && node->parent) {
            while (node && node->parent && !node->sibling) node = node->parent;
            return node->sibling;
        }
            
        return next;
    }

    static constexpr inline int node_depth(Node *node) {
        int i = 0;
        for (;node->parent;node = node->parent, i++);
        return i;
    }

    constexpr inline bool operator==(const Derived &other) const { return ptr == other.ptr; }
    constexpr inline bool operator!=(const Derived &other) const { return ptr != other.ptr; }
};

template<typename Node>
struct ChildIteratorT : public NodeIteratorT<ChildIteratorT<Node>, Node> {
    using NodeIterator = NodeIteratorT<ChildIteratorT<Node>, Node>;
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline ChildIteratorT &operator++() {
        ptr = ptr->sibling;
        return *this;
    }

    constexpr inline static ChildIteratorT make(Node *p) { return ChildIteratorT(p->child); }
};

template<typename Node>
struct ReverseChildIteratorT : public NodeIteratorT<ReverseChildIteratorT<Node>, Node> {
    using NodeIterator = NodeIteratorT<ReverseChildIteratorT<Node>, Node>;
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline ReverseChildIteratorT &operator++() {
        ptr = NodeIterator::previous_sibling(ptr);
        return *this;
    }

    constexpr inline static ReverseChildIteratorT make(Node *p) { return ReverseChildIteratorT(NodeIterator::last_sibling(p->child)); }
};

template<typename Node>
struct DepthFirstIteratorT : public NodeIteratorT<DepthFirstIteratorT<Node>, Node> {
    using NodeIterator = NodeIteratorT<DepthFirstIteratorT<Node>, Node>;
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline DepthFirstIteratorT &operator++() {
        ptr = NodeIterator::depth_first_next(ptr);
        return *this;
    }

    constexpr inline static DepthFirstIteratorT make(Node *p) { return DepthFirstIteratorT(NodeIterator::deepest_child(p)); }
};

template<typename Node>
struct DepthFirstReverseIteratorT : public NodeIteratorT<DepthFirstReverseIteratorT<Node>, Node> {
    using NodeIterator = NodeIteratorT<DepthFirstReverseIteratorT<Node>, Node>;
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline DepthFirstReverseIteratorT &operator++() {
        ptr = NodeIterator::breadth_first_next(ptr);
        return *this;
    }

    constexpr inline static DepthFirstReverseIteratorT make(Node *p) { return DepthFirstReverseIteratorT(p); }
};

template<typename NodeIterT, typename = std::is_base_of<NodeIteratorT<NodeIterT, typename NodeIterT::value_type>, NodeIterT>>
static constexpr inline NodeIterT operator++(NodeIterT &src) {
    NodeIterT temp = src;
    ++(src);
    return temp;
}

struct Node;

using NodeIterator = NodeIteratorT<bool, Node>;
using ChildIterator = ChildIteratorT<Node>;
using ReverseChildIterator = ReverseChildIteratorT<Node>;
using DepthFirstIterator = DepthFirstIteratorT<Node>;
using DepthFirstReverseIterator = DepthFirstReverseIteratorT<Node>;

struct Node {
    const char *name;

    Node *parent, *child, *sibling;

    constexpr Node(const char *name = nullptr, Node *parent = nullptr, Node *child = nullptr, Node *sibling = nullptr)
        :name(name),parent(parent),child(child),sibling(sibling) { }

    constexpr inline Node *append_sibling(Node *node) {
        Node *neighbor = NodeIterator::last_sibling(this);

        neighbor->sibling = node;
        node->parent = parent;

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
    
    constexpr inline ChildIterator children() { return ChildIterator::make(this); }
    constexpr inline ReverseChildIterator rchildren() { return ReverseChildIterator::make(this); }
    constexpr inline DepthFirstIterator depth_first() { return DepthFirstIterator::make(this); }
    constexpr inline DepthFirstReverseIterator rdepth_first() { return DepthFirstReverseIterator::make(this); }

    inline std::string get_name(const int &pad_left = 0, const char *class_name = "Node") const {
        std::string prepend(pad_left, ' ');

        return prepend + class_name + ": " + (this ? name : "null");
    }
};

void append_nodes(Node &parent, const int &breadth, const int &depth, const int &breadthsub=1, const int &depthsub=1) {
    const int cur_depth = NodeIterator::node_depth(&parent)+1;
    std::string depth_name(std::to_string(cur_depth));

    for (int i = 0; i < breadth; i++) {
        std::string node_name = depth_name + "." + std::to_string(i);
        const char *name = strcpy(new char[node_name.size()+1], node_name.c_str());
        Node *n = new Node(name);
        parent << *n;

        if (depth)
            append_nodes(*n, breadth-breadthsub, depth-depthsub, breadthsub, depthsub);
    }
}

template<typename Iter>
void print_nodes(Node &root, const char *name = "Iterator") {
    std::cout << "\n" << name << std::endl;
    Iter iterator = Iter::make(&root);
    for (Node &node : iterator)
        std::cout << node.get_name(NodeIterator::node_depth(&node)*2) << std::endl;
}

void print_nodes_manual(Node &node) {
    std::cout << node.get_name(NodeIterator::node_depth(&node)*2) << std::endl;
    if (node.child)
        print_nodes_manual(*node.child);
    if (node.sibling)
        print_nodes_manual(*node.sibling);
}

int main() {
    Node root("Root");

    append_nodes(root, 2, 4, 0);

    std::cout << "Manual" << std::endl;

    print_nodes_manual(root);

    print_nodes<ChildIterator>(root, "ChildIterator");
    print_nodes<ReverseChildIterator>(root, "ReverseChildIterator");
    print_nodes<DepthFirstIterator>(root, "DepthFirstIterator");
    print_nodes<DepthFirstReverseIterator>(root, "DepthFirstReverseIterator");

    return 0;
}