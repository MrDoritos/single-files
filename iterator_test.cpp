#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Node {
	const char *name;

	Node *parent;
	Node *child;
	Node *sibling;

	constexpr Node(const char *name, Node *parent, Node *child, Node *sibling)
		:name(name),parent(parent),child(child),sibling(sibling) {}
	constexpr Node(const char *name)
		:Node(name, nullptr, nullptr, nullptr) {}
	constexpr Node()
		:Node("unnamed") {}

	constexpr inline Node *append_sibling(Node *node) {
		if (sibling)
			node->append_sibling(node);

		sibling = node;
		sibling->parent = parent;

		return node;
	}

	constexpr inline Node *append_child(Node *node) {
		node->parent = this;

		if (child)
			child->append_sibling(node);
		else
			child = node;

		return node;
	}

	constexpr inline Node &append_child(Node &node) {
		return *append_child(&node);
	}

	constexpr inline Node &operator<<(Node &node) {
		return append_child(node);
	}
	
	constexpr inline Node *operator<<(Node *node) {
		return append_child(node);
	}

	constexpr inline Node &operator<<(const char *text) {
		name = text;
		return *this;
	}

	/*
	friend constexpr inline Node *operator<<(Node *node, const char *text) {
		*node << text;
		return node;
	}

	friend constexpr inline Node *operator<<(const char *text, Node *node) {
		*node << text;
		return node;
	}
	*/

	struct NodeIterator {
		using value_type = Node;
		using difference_type = void*;
		using pointer = Node*;
		using reference = Node&;
		//using iterator_category = 

		pointer ptr;

		constexpr NodeIterator(Node* p):ptr(p){}

		constexpr inline reference operator*() const { return *ptr; }
		constexpr inline pointer operator->() const { return ptr; }

		constexpr inline NodeIterator& operator++() {
			ptr = ptr->sibling;
			return *this;
		}

		constexpr inline NodeIterator operator++(int) {
			NodeIterator temp = *this;
			ptr = ptr->sibling;
			return temp;
		}

		constexpr inline bool operator==(const NodeIterator& other) const { return ptr == other.ptr; }
		constexpr inline bool operator!=(const NodeIterator& other) const { return ptr != other.ptr; }
	};

	constexpr inline Node *first_child() const {
		return child;
	}

	constexpr inline Node *last_child() const {
		Node *node = child;
		for (; (node != nullptr) && (node->sibling != nullptr); node = node->sibling);
		return node;
	}

	constexpr inline Node *deepest_child() const {
		Node *node = child;
		for (; (node != nullptr) && (node->child != nullptr); node = node->child);
		return node;
	}

	constexpr inline Node *next_node() const {
		if (sibling)
			return sibling;
		if (parent)
			return parent;
		return nullptr;
	}

	constexpr inline Node *root_node() const {
		Node *node = (Node*)this;
		for (; node->parent != nullptr; node = node->parent);
		return node;
	}

	constexpr inline Node *next_sibling() const {
		return sibling;
	}

	constexpr inline Node *parent_node() const {
		return parent;
	}

	constexpr inline Node *previous_sibling() const {
		if (!parent || parent->child == this) return nullptr;
		Node *node = parent->child;
		for (;(node != nullptr) && (node->sibling != this); node = node->sibling);
		return node;
	}

	struct ReverseNodeIterator : protected NodeIterator {
		constexpr ReverseNodeIterator(Node *p):NodeIterator(nullptr) {
			ptr = p;
			for (Node *i = p; (i != nullptr) && (ptr = i); i = i->sibling);
		}

		constexpr inline ReverseNodeIterator &operator++() {
			const Node *cur = ptr;
			if (!ptr || !ptr->parent || ptr->parent->child == ptr)
				ptr = nullptr;
			else
				for (Node *i = ptr->parent->child; (i != nullptr) && (i != cur) && (ptr = i); i = i->sibling);
			return *this;
		}

		constexpr inline ReverseNodeIterator operator++(int) {
			ReverseNodeIterator temp = *this;
			++*this;
			return temp;
		}

		constexpr inline reference operator*() const { return *ptr; }
		constexpr inline pointer operator->() const { return ptr; }

		constexpr inline ReverseNodeIterator begin() const {
			return *this;
		}

		constexpr inline ReverseNodeIterator end() const {
			return ReverseNodeIterator(nullptr);
		}

		constexpr inline bool operator==(const ReverseNodeIterator &other) const { return ptr == other.ptr; }
		constexpr inline bool operator!=(const ReverseNodeIterator &other) const { return ptr != other.ptr; }
	};

	struct DepthFirstIterator : protected NodeIterator {
		bool bubbling;
	
		constexpr DepthFirstIterator(Node *root):NodeIterator(root),bubbling(false) {}
		
		constexpr inline DepthFirstIterator &operator++() {
			Node *next = ptr->sibling;
				
			if (next) {
				next = next->deepest_child();
				ptr = next ? next : ptr->sibling;
				return *this;
			}

			ptr = ptr->parent;
			return *this;
		}

		constexpr inline DepthFirstIterator operator++(int) {
			DepthFirstIterator temp = *this;
			++*this;
			return temp;
		}

		constexpr inline reference operator*() const { return *ptr; }
		constexpr inline pointer operator->() const { return ptr; }

		constexpr inline DepthFirstIterator begin() const { return *this; }
		constexpr inline DepthFirstIterator end() const { return DepthFirstIterator(nullptr); }

		constexpr inline bool operator==(const DepthFirstIterator &other) const { return ptr == other.ptr; }
		constexpr inline bool operator!=(const DepthFirstIterator &other) const { return ptr != other.ptr; }
	};
	
	constexpr inline NodeIterator begin() const { return NodeIterator(child); }
	constexpr inline NodeIterator end() const { return NodeIterator(nullptr); }
	constexpr inline ReverseNodeIterator rbegin() const { return ReverseNodeIterator(child); }
	constexpr inline ReverseNodeIterator rend() const { return ReverseNodeIterator(nullptr); } 

	constexpr inline NodeIterator forward() const { return begin(); }
	constexpr inline ReverseNodeIterator backward() const { return rbegin(); }
	constexpr inline DepthFirstIterator depth_first() const { return DepthFirstIterator(deepest_child()); }

	constexpr inline void print_node_name(const int &depth = 0, const char *class_name = "Node") const {
		char prepend[depth+1] = {0};
		memset(prepend, ' ', depth);
		prepend[depth] = 0;

		printf(prepend);
		printf("%s: %s\n", class_name, this != nullptr ? name : "null");
	}

	constexpr inline void print_tree_native(const int &depth = 0) const {
		if (!depth)
			puts("print_tree_native");

		print_node_name(depth);

		if (child)
			child->print_tree_native(depth+2);
		if (sibling)
			sibling->print_tree_native(depth);
	}

	constexpr inline void print_tree_iterator(const int &depth = 0) const {
		if (!depth)
			puts("print_tree_iterator");

		print_node_name(depth);

		for (auto &child : *this)
			child.print_tree_iterator(depth+2);
	}

	constexpr inline void print_tree_reverse(const int &depth = 0) const {
		if (!depth)
			puts("print_tree_reverse");

		print_node_name(depth);

		for (auto &child : this->backward())
			child.print_tree_reverse(depth+2);
	}

	constexpr inline void print_tree_traversal(const int &depth = 0) const {
		if (!depth)
			puts("print_tree_traversal");

		print_node_name(depth);

		first_child()->print_node_name(depth+2, "first_child");
		last_child()->print_node_name(depth+2, "last_child");
		root_node()->print_node_name(depth+2, "root_node");
		previous_sibling()->print_node_name(depth+2, "previous_sibling");
		next_sibling()->print_node_name(depth+2, "next_sibling");
		parent_node()->print_node_name(depth+2, "parent_node");

		for (auto &child : *this)
			child.print_tree_traversal(depth+4);
	}

	constexpr inline void print_tree_depth_first(const int &depth = 0) const {
		print_node_name(depth, "Root");
		for (auto &node : depth_first())
			node.print_node_name(depth, "visit");
	}
};

/*
Node &operator<<(Node &node, const char *text) {
	*node << text;
	return node;
}
*/

int main() {
	Node root;

	root << "root";

	*(root << new Node()) << "node" << new Node("hello");

	//root << new Node("sibling") << new Node("child");
	Node sibling("sibling");
	Node child("child");

	root << sibling << child;

	root.print_tree_native();

	root.print_tree_iterator();

	root.print_tree_reverse();

	root.print_tree_traversal();

	root.print_tree_depth_first();

	return 0;
}
