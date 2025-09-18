#include <iostream>
#include <string>
#include <vector>
#include <map>

// this is for the find method.
#include <algorithm>


#include "main.cpp"

using namespace std;

int main() {

    cout << "ENSURE YOUR NFA IS 0-INDEXED BEFORE PROCEEDING." << endl;

    cout << "Enter the total number of states in your NFA: ";

    int numStates;

    cin >> numStates;

    // Number of the singular starting state.
    cout << "Enter the number of the start state (0-indexed): ";

    int startState;

    cin >> startState;

    // Number of how many final states there are.
    cout << "Enter the number of the final states there are: ";

    int numFinalStates;

    cin >> numFinalStates;

    // Listing all the final states. 
    cout << "List the final states: ";

    vector<int> finalStates;

    for (int i = 0; i < numFinalStates; i++) {
        int finalState;
        cin >> finalState;
        finalStates.push_back(finalState);
    }

    // Number of transitions in the NFA.
    cout << "Enter the number of transitions in your NFA: ";
    
    int numTransitions;

    cin >> numTransitions;

    // Listing all the transitions.
    cout << "Format: fromState toState regex" << endl;
    cout << "List the transitions: ";

    vector<Transition> transitions;

    for (int i = 0; i < numTransitions; i++) {
        int fromState, toState;
        string regex;
        cin >> fromState >> toState >> regex;
        transitions.push_back({fromState, toState, regex});
    }

    // Creating the NFA.
    NFA nfa;
    nfa.startState = startState;

    vector<State> states;
    for (int i = 0; i < numStates; i++) {
        // Find method returns the iterator to the first element in the range [first,last) that compares equal to val.
        // If no such element is found, the function returns end().
        // We could have also used a simple for loop, but this is more efficient + better for scalability.
        if (find(finalStates.begin(), finalStates.end(), i) != finalStates.end()) {
            states.push_back({i, true});
        } else {
            states.push_back({i, false});
        }
    }

    nfa.finalStates = finalStates;
    nfa.transitions = transitions;
    nfa.states = states;

    // Converting the NFA to a regex.
    string regex = convertToRegex(nfa);

    cout << "The regex is: " << regex << endl;

    return 0;
}