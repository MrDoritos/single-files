# Chapter 1

## Section 1

Symbols|Description
---|---
≡|Logical equivalence
↔|Bidirectional (if and only if)
→|Conditional (if then)
∴|Therefore
∧|Conjunction (and)
∨|Disjunction (or)
¬|Negation (not)
∃|Existential quantifier
∀|Universal quantifier
:|Such that
∈|Element of
⊆|Is a subset of
⊂|Is a proper subset of
∩|Set intersection
∪|Set union
×|Cartesian product
∖|Set difference

Sets|Description
---|---
∅|Empty set
N|Natural numbers
Z|Integers
Q|Rational numbers
R|Real numbers

### Logical Equivalences

Law|Equivalency
---|---
Identity Laws|p∧T ≡ p
-|p∨F ≡ p
Domination Laws|p∨T ≡ T
-|p∧F ≡ F
Idempotent Laws|p∨p ≡ p
-|p∧p ≡ p
Double Negation|¬¬p ≡ p
Commutative Laws|p∨q ≡ q∨p
-|p∧q ≡ q∧p
Associative Laws|(p∨q)∨r ≡ p∨(q∨r)
-|(p∧q)∧r ≡ p∧(q∧r)
Distributive Laws|p∨(q∧r) ≡ (p∨q)∧(p∨r)
-|p∧(q∨r) ≡ (p∧q)∨(p∧r)
De Morgan's Laws|¬(p∧q) ≡ ¬p∨¬q
-|¬(p∨q) ≡ ¬p∧¬q
Absorption Laws|p∨(p∧q) ≡ p
-|p∧(p∨q) ≡ p
Negation Laws|p∨¬p ≡ T
-|p∧¬p ≡ F

### Logical Equivalences Involving Conditional Statements

LHS|RHS
---|---
p→q|¬p∨¬q
p→q|¬q→¬p
p∨q|¬p→q
p∧q|¬(p→¬q)
¬(p→q)|p∧¬q
(p→q)∧(p→r)|p→(q∧r)
(p→r)∧(q→r)|(p∨q)→r
(p→q)∨(p→r)|p→(q∨r)
(p→r)∨(q→r)|(p∧q)→r

### Logical Equivalences Involving Biconditional Statements

LHS|RHS
---|---
p↔q|(p→q)∧(q→p)
p↔q|¬p↔¬q
p↔q|(p∧q)∨(¬p∧¬q)
¬(p↔q)|p↔¬q

### Negating Quantified Expressions