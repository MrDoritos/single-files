#!/bin/python3
import sys
import copy
import math

class Node:
    def __init__(self, token=None, left=None, right=None, parent=None):
        self.left = left
        self.right = right
        self.token = token
        self.parent = parent

    def copy(self, token=None, left=None, right=None):
        node = copy.deepcopy(self)
        node.token = token if token else self.token
        node.left = left if left else self.left
        node.right = right if right else self.right
        return node

    def getMaxDepth(self):
        return max(
            self.left.getMaxDepth() if isinstance(self.left, Node) else 0,
            self.right.getMaxDepth() if isinstance(self.right, Node) else 0
        ) + 1

    def getChild(self, leg):
        return getattr(self, leg)

    def setChild(self, node, leg):
        setattr(self, leg, node)
        if node is not None and isinstance(node, Node):
            node.parent = (self,leg)

    def withChild(self, node, leg):
        self.setChild(node, leg)
        return self
    
    def withChildren(self, left, right):
        self.setChild(left, 'left')
        self.setChild(right, 'right')
        return self

    def getChildStr(self, leg):
        child = self.getChild(leg)
        if child is not None and isinstance(child, Node):
            return child.token if child.token is not None else 'No Token'
        return child

    def swapChild(self, node, self_leg, child_leg):
        child=self.getChild(self_leg)
        node.setChild(child, child_leg)
        self.setChild(node, self_leg)
        return child

    def swapParent(self, node, node_leg):
        parent,leg=self.parent
        parent.setChild(node, leg)
        node.setChild(self, node_leg)

    def getTreeStr(self,depth=0,basis=0,our_basis=0):
        left=None
        right=None
        #basis_factor=basis * (1-pow(depth+2,-1))
        basis_factor=basis * .5 * math.sqrt(pow(depth+3,-1)*1.0)
        left_basis,left_depth=our_basis-basis_factor,depth+1
        right_basis,right_depth=our_basis+basis_factor,depth+1

        ret=[[depth,our_basis,self.token]]

        if isinstance(self.left, Node):
            left=self.left.getTreeStr(left_depth,basis,left_basis)
        else:
            left=[[left_depth,our_basis-3,self.left]]
        if isinstance(self.right, Node):
            right=self.right.getTreeStr(right_depth,basis,right_basis)
        else:
            right=[[right_depth,our_basis+3,self.right]]
        
        return ret + left + right
        
    def getSelfStr(self,basis=None):
        if basis is None:
            basis = self.getMaxDepth() * 7
        family=sorted(self.getTreeStr(0,basis,basis))
        print(family)
        ret=''
        cur_line=''
        y_pos=0
        for ancestor in family:
            if ancestor[0] > y_pos:
                y_pos += 1
                ret += cur_line + '\n'
                cur_line = ''
            anc_basis = int(ancestor[1])
            anc_data = ancestor[2]
            anc_str = anc_data.token if isinstance(anc_data, Node) else str(anc_data)
            if not anc_str:
                anc_str = ''
            anc_string = f"{anc_str:>{anc_basis}}"
            #print('1:', cur_line, anc_string, anc_str)
            if len(cur_line) < len(anc_string):
                cur_line += (' ' * (len(anc_string)-len(cur_line)))
            #print('2:', cur_line, anc_str)
            cur_line = cur_line[:len(anc_string)-len(anc_str)] + anc_str + cur_line[len(anc_string):]            
            #print('3:', cur_line, anc_str)

        return ret + cur_line

    def __repr__(self):
        return f"token: {self.token} left: {self.getChildStr('left')} right: {self.getChildStr('right')}"

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

class Lexeme:
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

    def parse(self, node, lexer, text, stack):
        _prev = lexer.peekPrev()
        _next = lexer.peekNext()

        node.token = self.value

        print('MODIFYING:', self.value, node.token)
        print(f'{lexer.getIndex():>4}|{self.pos:>4}|{self.value}')

        #if len(stack):
        #    node.left = Node(parent=(node,'left'))
        #    lexer.next().parse(node.left, lexer, text, stack)
        #    print('left is pop stack')
        #else:
        #    node.left = lexer.valueLeft()
        #    print(f'left is value: {node.left}')
        if not node.left:
            node.left = lexer.valueLeft()
            print(f'left is value: {node.left}')
        else:
            print(f'left set {node.left}')

        if _next:
            node.right = Node(token=_next.value,parent=(node,'right'))
            lexer.next().parse(node.right, lexer, text, stack)
            print('right is new node')
        else:
            node.right = lexer.valueRight()
            print(f'right is value: {node.right}')

        return node

class Parenthesis(Lexeme):
    def __init__(self, name=None, value=None, pos=None):
        Lexeme.__init__(self, name, value, pos)

    def parse(self, node, lexer, text, stack):
        print('MODIFYING:', self.value, node.token)
        print('parenthesis:', self.value, 'node:', node, 'stack:', stack, 'parent:', node.parent, 'value_left:', lexer.valueLeft(), 'value_right:', lexer.valueRight())
        if self.value == '(':
            stack.append(node.parent if node.parent is not None else (node, 'left'))
            lexer.next().parse(node, lexer, text, stack)
            print('end parenthesis:', self.value, 'node:', node, 'stack:',stack, 'parent:', node.parent, 'value_left:', lexer.valueLeft(), 'value_right:', lexer.valueRight())
            return node
        if self.value == ')':
            parent,leg=stack.pop()
            node.parent[0].right = lexer.valueLeft()
            if lexer.peekNext() is not None:
                _next = lexer.next()
                nnode = Node(token=_next.value,parent=(parent,leg))
                parent.right = nnode
                nnode.left = node.parent[0]
                _next.parse(nnode, lexer, text, stack)
            #if node.parent[0] is not parent:
            #    print('swapchild:',node.parent[0],'   ',leg,node,':',parent)
            #    node =parent.swapChild(node, leg, 'right')
            #node.right = lexer.valueLeft()
            print('end parenthesis:', self.value, 'node:', node, 'stack:',stack, 'parent:', node.parent, 'value_left:', lexer.valueLeft(), 'value_right:', lexer.valueRight())
            return node.parent

shared_lexemes = {
    'PARENTHESIS':Lexeme('PARENTHESIS'),
}

lexemes = {
    '(':Parenthesis('PARENTHESIS_OPEN'),
    ')':Parenthesis('PARENTHESIS_CLOSE'),
    '*':Lexeme('MULTIPLY'),
    '/':Lexeme('DIVIDE'),
    '-':Lexeme('SUBTRACT'),
    '+':Lexeme('ADD'),
}

class Lexer:
    def __init__(self):
        self.iter=0

    def resetIterator(self):
        self.iter=0

    def peekNext(self):
        return self.index(self.iter+1)
    
    def peekPrev(self):
        return self.index(self.iter-1)

    def valueLeft(self, i=None):
        if i is None:
            i = self.iter
        prev = self.index(i-1)
        cur = self.index(i)
        prn=lambda x: print(f'valueleft: index{i} prev: {prev} cur: {cur} text: {x}')
        if not cur and not prev:
            prn(self.text)
            return self.text
        if not prev:
            prn(self.text[:cur.pos])
            return self.text[:cur.pos]
        if not cur:
            prn(self.text[prev.pos+1:])
            return self.text[prev.pos+1:]
        prn(self.text[prev.pos+1:cur.pos])
        return self.text[prev.pos+1:cur.pos]

    def valueRight(self, i=None):
        if i is None:
            i = self.iter
        return self.valueLeft(i+1)

    def get(self):
        return self.index(self.iter)

    def getIndex(self):
        return self.iter

    def index(self, i):
        if i < 0 or i >= len(self.lexicon):
            return None
        return self.lexicon[i]

    def next(self):
        self.iter += 1
        return self.index(self.iter)

    def prev(self):
        self.iter -= 1
        return self.index(self.iter)

    def parse(self, text):
        self.text = text
        self.lexicon = []
        for i,c in enumerate(text):
            if c in lexemes:
                self.lexicon.append(lexemes[c].copy(i, c))

    def parseTree(self):
        if not self.get():
            return None
        self.resetIterator()
        print('L  i|T  p|type')
        return self.get().parse(Node(), self, self.text, [])

    def __repr__(self):
        return ' '.join(map(lambda x: x.__repr__(), self.lexicon))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('Please enter expression')
        exit()

    a=sys.argv[1:]
    s=''.join(a)
    print(f"'{a}' '{s}'")
    lexicon = Lexer()
    lexicon.parse(s)
    print(lexicon)
    print(lexicon.parseTree().getSelfStr())
    node=Node('+').withChildren(8, Node('+').withChildren(Node('*').withChildren(1,2), 5))
    #print(node.getSelfStr())
    #node.right.swapChild(Node('/').withChildren(4,5), 'left', 'right')
    #print(node.getSelfStr())
