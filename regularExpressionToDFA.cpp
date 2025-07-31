#include <bits/stdc++.h>
using namespace std;

int nodeNumber = 0;
set<char> alphabets;
set<char> validChar = {'(',')','+','.','*','E'};

class Node;
class Machine;
void makeGrammar(string &str) {
    for(char ch : str) {
        if(ch == ' ' || ch == ',') {
            continue;
        }
        alphabets.insert(ch);
        validChar.insert(ch);
    }
}

bool checkValid(string &s) {
    if (s.empty()) return false;

    int balance = 0;
    char prev = '\0';

    for (int i = 0; i < s.size(); ++i) {
        char ch = s[i];
        if (validChar.find(ch) == validChar.end()) {
            cout << "Invalid Char" << endl;
            return false;
        }

        if (ch == '(') {
            balance++;
        } else if (ch == ')') {
            balance--;
            if (balance < 0) {
                cout << "More closing brackets" << endl;
                return false; 
            }
        }

        if ((ch == '+' || ch == '.' || ch == '*') && (prev == '+' || prev == '.' || prev == '(')) {
            cout << prev << " " << ch << endl;
            cout << "Invalid prev, curr pair" << endl;
            return false;
        }

        if(ch == '*' && prev == '*') {
            cout << prev << " " << ch << endl;
            cout << "Invalid curr, prev pair" << endl;
            return false;
        }

        if (i == 0 && (ch == '+' || ch == '.' || ch == '*')) {
            cout << "Operator cannot be at the starting" << endl;
            return false;
        }

        if (i == s.size() - 1 && (ch == '+' || ch == '.')) {
            cout << "+ and . cannot be at the end" << endl;
            return false;
        }

        prev = ch;
    }

    if(balance != 0) {
        cout << "More opening brackets" << endl;
        return false;
    }

    return true;
}

bool isAlpha(char ch) {
    return alphabets.count(ch) > 0;
}

class Node {
public:
    int nodeId;
    bool isStart;
    bool isFinal;
    
    // NFA-node
    map<Node*, char> connections;

    // DFA-node
    set<Node*> nfaStates; 
    map<char, Node*> transitions; 

    Node() {
        this->nodeId = -1;
        this->isStart = false;
        this->isFinal = false;
    }
    Node(int nodeId, bool isStart, bool isFinal) {
        this->nodeId = nodeId;
        this->isStart = isStart;
        this->isFinal = isFinal;
    }

    ~Node() {
        connections.clear();
        transitions.clear();
        nfaStates.clear();
    }
};

struct NodeComparator {
    bool operator()(const Node* a, const Node* b) const {
        return a->nodeId < b->nodeId;
    }
};

class Machine {
public:
    set<Node*, NodeComparator> nodes;
    Node* startNode;
    Node* endNode;
    
    Machine() {
        this->startNode = nullptr;
        this->endNode = nullptr;
    };

    void addNode(Node* currNode) {
        this->nodes.insert(currNode);
        
        if(currNode->isStart) {
            this->startNode = currNode;
        }
        if(currNode->isFinal) {
            this->endNode = currNode;
        }
    }
    void removeNode(Node* n) {
        n->isStart = false;
        this->nodes.erase(n);
        delete n;
    }
    ~Machine() {
        for (auto node : nodes) {
            delete node;
        }
        nodes.clear();
        startNode = nullptr;
        endNode = nullptr;
    }
};

void edgeConnector(Node* n1, Node* n2, char ch) {
    n1->connections[n2] = ch;
}

void edgeDisconnector(Node* n1, Node* n2) {
    n1->connections.erase(n2);
}

Machine* unionMachines(Machine* m1, Machine* m2) {
    Machine* m = new Machine();
    Node* startNode = new Node(nodeNumber++, true, false);
    Node* endNode = new Node(nodeNumber++, false, true);

    m->addNode(startNode);
    m->addNode(endNode);

    m1->startNode->isStart = false;
    m2->startNode->isStart = false;
    m1->endNode->isFinal = false;
    m2->endNode->isFinal = false;
    
    edgeConnector(startNode, m1->startNode, 'E');
    edgeConnector(startNode, m2->startNode, 'E');
    edgeConnector(m1->endNode, endNode, 'E');
    edgeConnector(m2->endNode, endNode, 'E');

    for(auto n : m1->nodes) m->addNode(n);
    for(auto n : m2->nodes) m->addNode(n);
    
    m1->nodes.clear();
    m2->nodes.clear();
    delete m1;
    delete m2;

    return m;
}

Machine* concatenateMachines(Machine* m1, Machine* m2) {
    m1->endNode->isFinal = false;
    
    for(auto c : m2->startNode->connections) {
        edgeConnector(m1->endNode, c.first, c.second);
    }
    
    m2->removeNode(m2->startNode);

    for(auto n : m2->nodes) {
        m1->addNode(n);
    }
    m1->endNode = m2->endNode;

    m2->nodes.clear();
    delete m2;

    return m1;
}

Machine* kleenesClousre(Machine* m) {
    Machine* m_new = new Machine();
    Node* startNode = new Node(nodeNumber++, true, false);
    Node* endNode = new Node(nodeNumber++, false, true);

    m->startNode->isStart = false;
    m->endNode->isFinal = false;

    m_new->addNode(startNode);
    m_new->addNode(endNode);
    
    edgeConnector(startNode, endNode, 'E');
    edgeConnector(startNode, m->startNode, 'E');
    edgeConnector(m->endNode, endNode, 'E');
    edgeConnector(m->endNode, m->startNode, 'E');

    for(auto n : m->nodes) {
        m_new->addNode(n);
    }
    
    m->nodes.clear();
    delete m;

    return m_new;
}

int precedence(char op) {
    if (op == '*') return 3;
    if (op == '.') return 2;
    if (op == '+') return 1;
    return 0;
}

string infixToPostfix(string &s) {
    stack<char> st;
    string postfix = "";
    for(char ch : s) {
        if(isAlpha(ch)) {
            postfix.push_back(ch);
        } else if(ch == '(') {
            st.push(ch);
        } else if(ch == ')') {
            while(!st.empty() && st.top() != '(') {
                postfix.push_back(st.top());
                st.pop();
            }
            st.pop(); 
        } else {
            while(!st.empty() && precedence(st.top()) >= precedence(ch)) {
                postfix.push_back(st.top());
                st.pop();
            }
            st.push(ch);
        }
    }
    while(!st.empty()) {
        postfix.push_back(st.top());
        st.pop();
    }
    return postfix;
}

void printNFA(Machine* m) {
    cout << endl;
    cout << "Start Node: " << m->startNode->nodeId << endl;
    cout << "End Node: " << m->endNode->nodeId << endl;
    cout << endl;
    for(auto n : m->nodes) {
        cout << "Node no: " << n->nodeId;
        if (n->isStart) cout << " (Start)";
        if (n->isFinal) cout << " (Final)";
        cout << endl;
        if(n->connections.size() == 0) {
            cout << "  No outgoing edge." << endl; 
        }
        for(auto c : n->connections) {
            cout << "  " << n->nodeId << " --" << c.second << "--> " << c.first->nodeId << endl;
        }
    }
    cout << endl;
}

Machine* nfa(string &s) {
    stack<Machine*> st;
    for(char ch : s) {
        if(ch == '*') {
            Machine* m = st.top(); st.pop();
            st.push(kleenesClousre(m));
        } else if(ch == '+') {
            Machine* m2 = st.top(); st.pop();
            Machine* m1 = st.top(); st.pop();
            st.push(unionMachines(m1, m2));
        } else if(ch == '.') {
            Machine* m2 = st.top(); st.pop();
            Machine* m1 = st.top(); st.pop();
            st.push(concatenateMachines(m1, m2));
        } else {
            Node* startNode = new Node(nodeNumber++, true, false);
            Node* endNode = new Node(nodeNumber++, false, true);
            Machine* m = new Machine();
            m->addNode(startNode);
            m->addNode(endNode);
            edgeConnector(startNode, endNode, ch);
            st.push(m);
        }
    }
    return st.top();
}

set<Node*> epsilonClosure(set<Node*>& states) {
    set<Node*> closure = states;
    stack<Node*> s;
    for (Node* n : states) {
        s.push(n);
    }

    while (!s.empty()) {
        Node* u = s.top();
        s.pop();

        for (auto c : u->connections) {
            auto v = c.first;
            if (c.second == 'E') {
                if (closure.find(v) == closure.end()) {
                    closure.insert(v);
                    s.push(v);
                }
            }
        }
    }
    return closure;
}

void printDfaTable(map<set<Node*>, Node*>& dfa, Node* startState) {
    cout << "--- DFA Transition Table ---" << endl;

    vector<int> finalStates;
    for(auto s : dfa) {
        if (s.second->isFinal) {
            finalStates.push_back(s.second->nodeId);
        }
    }

    cout << "Start State: " << startState->nodeId << endl;
    cout << "Final States: { ";
    int f = finalStates.size();
    for(int i = 0; i < f; ++i) {
        cout << finalStates[i] << (i == f - 1 ? "" : ", ");
    }
    cout << " }" << endl << endl;

    cout << "State\t|";
    for (char c : alphabets) {
        cout << "\t" << c << "\t|";
    }
    cout << endl;
    cout << "--------|";
    for (size_t i = 0; i < alphabets.size(); ++i) {
        cout << "---------------|";
    }
    cout << endl;

    vector<Node*> dfaNodes;
    for(auto s : dfa) {
        dfaNodes.push_back(s.second);
    }
    sort(dfaNodes.begin(), dfaNodes.end(), [](Node* a, Node* b){
        return a->nodeId < b->nodeId;
    });

    for(Node* n : dfaNodes) {
        cout << (n->isFinal ? "*" : " ") << n->nodeId << "\t|";
        for (char c : alphabets) {
            if (n->transitions.count(c)) {
                cout << "\t" << n->transitions[c]->nodeId << "\t|";
            } else {
                cout << "\t-\t|";
            }
        }
        cout << endl;
    }
    cout << "\n(* denotes final state)" << endl;
}

pair<map<set<Node*>, Node*>, Node*> convertToDfa(Machine* nfa) {
    if (!nfa || !nfa->startNode) {
        cout << "Cannot convert an empty NFA." << endl;
        return {{}, NULL};
    }
    
    map<set<Node*>, Node*> dfa;
    queue<set<Node*>> q;
    
    int counter = 0;

    set<Node*> nfaStart = {nfa->startNode};
    set<Node*> dfaStart = epsilonClosure(nfaStart);

    Node* startNode = new Node(counter++, true, false);
    startNode->nfaStates = dfaStart;
    dfa[dfaStart] = startNode;
    q.push(dfaStart);

    while (!q.empty()) {
        set<Node*> currSet = q.front();
        q.pop();
        Node* currNode = dfa[currSet];

        for (Node* n : currSet) {
            if (n->isFinal) {
                currNode->isFinal = true;
                break;
            }
        }

        for (char ch : alphabets) {
            set<Node*> transitions;
            for (Node* n : currSet) {
                for (auto c : n->connections) {
                    if (c.second == ch) {
                        transitions.insert(c.first);
                    }
                }
            }

            if (transitions.empty()) {
                continue; 
            }

            set<Node*> nextNodes = epsilonClosure(transitions);
            if (dfa.find(nextNodes) == dfa.end()) {
                Node* newNode = new Node(counter++, false, false);
                newNode->nfaStates = nextNodes;
                dfa[nextNodes] = newNode;
                q.push(nextNodes);
            }
            
            currNode->transitions[ch] = dfa[nextNodes];
        }
    }

    return {dfa, startNode};
}

int main() {
    cout << "\n\nThis program converts a Regular Expression to a DFA." << endl;
    cout << "The following characters are reserved operators: ";
    for(char ch : validChar) {
        cout << ch << " ";
    }
    cout << "\n\n";
    cout << "Please enter a grammar (e.g., a,b): ";
    string str;
    getline(cin, str);
    makeGrammar(str);

    cout << "Please enter a regular expression (e.g., (a+b)*.a.b.b): ";
    string s;
    getline(cin, s); 

    string newStr;
    if (!s.empty()) {
        newStr.push_back(s[0]);
        for(size_t i = 1; i < s.size(); i++) {
            if((isAlpha(s[i-1]) && isAlpha(s[i])) || 
               (s[i] == '(' && isAlpha(s[i-1])) || 
               (isAlpha(s[i]) && s[i-1] == ')') || 
               (s[i] == '(' && s[i-1] == ')') || 
               (isAlpha(s[i]) && s[i-1] == '*') ||
               (s[i-1] == ')' && isAlpha(s[i]))
            ) {
                newStr.push_back('.');
            }
            newStr.push_back(s[i]);
        } 
    }
    
    if(!checkValid(newStr)) {
        cout << "Invalid Regular Expression." << "\n";
        return 1;
    }   

    string postFix = infixToPostfix(newStr);
    cout << "\nConcatenation: " << newStr << endl;
    cout << "Postfix Expression: " << postFix << endl;

    Machine* m = nfa(postFix);
    
    cout << "\n--- NFA Construction ---";
    printNFA(m);
    auto it = convertToDfa(m);
    printDfaTable(it.first, it.second);

    delete m;

    return 0;
}
