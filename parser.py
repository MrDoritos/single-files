#!/bin/python3
import sys
import copy

class Node:
    def __init__(self, token=None, left=None, right=None):
        self.left = left
        self.right = right
        self.token = token

    def copy(self, token=None, left=None, right=None):
        node = copy.deepcopy(self)
        node.token = token if token else self.token
        node.left = left if left else self.left
        node.right = right if right else self.right
        return node

    def __repr__(self):
        if self.left and self.right:
            return f"{self.left} '{self.token}' {self.right}"
        return f"'{self.token}'"

class Factor(Node):
    def __init__(self, token=None, left=None, right=None):
        Node.__init__(self, token, left, right)
        self.token = token

    def __repr__(self):
        return f"({self.left} {self.right})"

class Operation(Node):
    def __init__(self, op, token=None, left=None, right=None):
        Node.__init__(self, token, left, right)
        self.op = op
        self.token = token

    def __repr__(self):
        return f"{self.left} {self.token} {self.right}"

class Token:
    def __init__(self, name=None, value=None, pos=None):
        self.name = name
        self.value = value
        self.pos = None

    def __repr__(self, full=False):
        return f"{str(self.pos) + ':' if self.pos else ''}{self.name}{' ' + self.value if self.value else ''}"

    def copy(self, pos=None, value=None):
        token = copy.deepcopy(self)
        token.pos = pos
        token.value = self.value if value is None else value
        return token

    def parse(self, node, lexes, text):
        right_end = lexes[1].pos
        left_text = text[:self.pos]
        right_text = text[self.pos:right_end]
        node.left = lexes[0].parse(Node(), lexes[2:], left_text)
        node.right = lexes[1].parse(Node(), lexes[])
        return node

class Parenthesis(Token):
    def __init__(self, name=None, value=None, pos=None):
        Token.__init__(self, name, value, pos)

    def parse(self, node, lexes, text):
        

shared_lexers = {
    'PARENTHESIS':Token('PARENTHESIS'),
}

lexers = {
    '(':Token('PARENTHESIS_OPEN'),
    ')':Token('PARENTHESIS_CLOSE'),
    '*':Token('MULTIPLY'),
    '/':Token('DIVIDE'),
    '-':Token('SUBTRACT'),
    '+':Token('ADD'),
}

class Lexer:
    def __init__(self, text):
        self.text = text
        self.parse(text)

    def parse(self, text):
        self.lexes = []
        for i,c in enumerate(text):
            if c in lexers:
                self.lexes.append(lexers[c].copy(i))

    def __repr__(self):
        return ' '.join(map(lambda x: x.__repr__(), self.lexes))

class Parser:
    def __init__(self, lexer):
        self.lexer = lexer
        self.root = self.parse(Node(), lexer)

    def parse(self, node, lexes):
        for i,lex in enumerate(lexes):
            if 

    def __repr__(self):
        return self.root.__repr__()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('Please enter expression')
        exit()

    a=sys.argv[1:]
    s=''.join(a)
    print(f"'{a}' '{s}'")
    print(Lexer(s))
    print(Parser(s))
