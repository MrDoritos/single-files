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

		reference operator*() const { return *ptr; }
		pointer operator->() const { return ptr; }

		NodeIterator& operator++() {
			ptr = ptr->sibling;
			return *this;
		}

		NodeIterator operator++(int) {
			NodeIterator temp = *this;
			ptr = ptr->sibling;
			return temp;
		}

		constexpr inline bool operator==(const NodeIterator& other) const { return ptr == other.ptr; }
		constexpr inline bool operator!=(const NodeIterator& other) const { return ptr != other.ptr; }
	};

	struct ReverseNodeIterator : protected NodeIterator {
		constexpr ReverseNodeIterator(Node *p):NodeIterator(nullptr) {
			ptr = p;
			for (Node *i = p; (i != nullptr) && (ptr = i); i = i->sibling);
		}

		ReverseNodeIterator &operator++() {
			const Node *cur = ptr;
			if (!ptr || !ptr->parent || ptr->parent->child == ptr)
				ptr = nullptr;
			else
				for (Node *i = ptr->parent->child; (i != nullptr) && (i != cur) && (ptr = i); i = i->sibling);
			return *this;
		}

		ReverseNodeIterator operator++(int) {
			ReverseNodeIterator temp = *this;
			++*this;
			return temp;
		}

		reference operator*() const { return *ptr; }
		pointer operator->() const { return ptr; }

		constexpr inline ReverseNodeIterator begin() const {
			return *this;
		}

		constexpr inline ReverseNodeIterator end() const {
			return ReverseNodeIterator(nullptr);
		}

		constexpr inline bool operator==(const ReverseNodeIterator &other) const { return ptr == other.ptr; }
		constexpr inline bool operator!=(const ReverseNodeIterator &other) const { return ptr != other.ptr; }
	};
	
	constexpr inline NodeIterator begin() const { return NodeIterator(child); }
	constexpr inline NodeIterator end() const { return NodeIterator(nullptr); }
	constexpr inline ReverseNodeIterator rbegin() const { return ReverseNodeIterator(child); }
	constexpr inline ReverseNodeIterator rend() const { return ReverseNodeIterator(nullptr); } 

	constexpr inline NodeIterator forward() const { return begin(); }
	constexpr inline ReverseNodeIterator backward() const { return rbegin(); }

	constexpr inline void print_node_name(const int &depth = 0) const {
		char prepend[depth+1] = {0};
		memset(prepend, ' ', depth);
		prepend[depth] = 0;

		printf(prepend);
		printf("Node: %s\n", name);
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
		//if (sibling)
		//	sibling->print_tree_iterator(depth);
	}

	constexpr inline void print_tree_reverse(const int &depth = 0) const {
		if (!depth)
			puts("print_tree_reverse");

		print_node_name(depth);

		for (auto &child : this->backward())
			child.print_tree_reverse(depth+2);
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

	return 0;
}
