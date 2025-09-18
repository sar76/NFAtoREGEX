#include <iostream>
#include <vector>
#include <string>
#include <algorithm> 
#include <map>

using namespace std;


// We want some initial representation of NFA and its states.
// We want to be able to form GNFAs by combining NFA states. 
// Repeat this process until we have only a start and end state. 
// Keep track of transition functions between states, as this will hold our answer.
// Maybe we can attempt this recursively. 

struct Transition {
    int fromState;
    int toState;
    string regex;
};

struct State {
    int stateNumber;
    bool isFinal;
};

// Let's keep everything as a struct, to keep it all public, so no classes. 
struct NFA {
    int startState;
    vector<State> states;
    vector<Transition> transitions;
    // Using a set would be advantageous here, as we don't want duplicates. (using vector for simplicity)
    vector<int> finalStates;
};

// Helper function to handle epsilon transitions properly
string concatenateRegex(const string& r1, const string& r2) {
    if (r1.empty()) return r2;
    if (r2.empty()) return r1;
    return r1 + r2;
}

// & indicates we are passing by reference, so we can modify the original NFA.
void removeState(NFA& nfa, int stateToRemove) {
    // Function is void because we are modifying the original NFA over and over again in convertToRegex function. 

    // Step 0: Find self-loops on the state being removed
    string selfLoopRegex = "";
    for (auto& t : nfa.transitions) {
        if (t.fromState == stateToRemove && t.toState == stateToRemove) {
            if (selfLoopRegex.empty()) {
                selfLoopRegex = t.regex;
            } else {
                selfLoopRegex = "(" + selfLoopRegex + ")|(" + t.regex + ")";
            }
        }
    }
    
    // Step 1: Find transitions INTO stateToRemove (excluding self-loops)
    vector<Transition> inTransitions;
    for (auto& t : nfa.transitions) {
        if (t.toState == stateToRemove && t.fromState != stateToRemove) {
            inTransitions.push_back(t);
        }
    }
    
    // Step 2: Find transitions OUT of stateToRemove (excluding self-loops)
    vector<Transition> outTransitions;
    for (auto& t : nfa.transitions) {
        if (t.fromState == stateToRemove && t.toState != stateToRemove) {
            outTransitions.push_back(t);
        }
    }

    // Now there are two cases: 
        // Case 1: The same state is in both inTransitions and outTransitions. Indicating a self-loop on that state.
        // Case 2: Every other case, we must create a new transition that combines the inTransitions and outTransitions.
        // If such a transition already exists, we must combine them.
    // Step 3: Detect cycles (A -> removed -> A)
    map<int, string> cyclesCreated;
    for (auto& t1 : inTransitions) {      
        for (auto& t2 : outTransitions) { 
            if (t1.fromState == t2.toState) {  // Found a cycle
                int cycleState = t1.fromState;
                string cycleRegex = t1.regex;
                
                // Include self-loop on removed state if it exists
                if (!selfLoopRegex.empty()) {
                    cycleRegex += "(" + selfLoopRegex + ")*";
                }
                cycleRegex += t2.regex;
                
                if (cyclesCreated[cycleState].empty()) {
                    cyclesCreated[cycleState] = cycleRegex;
                } else {
                    cyclesCreated[cycleState] = "(" + cyclesCreated[cycleState] + ")|(" + cycleRegex + ")";
                }
            }
        }
    }

    // Step 4: Create bypass transitions
    vector<Transition> newTransitions;
    
    // Add existing transitions that don't involve removed state
    for (auto& existing : nfa.transitions) {
        if (existing.fromState != stateToRemove && existing.toState != stateToRemove) {
            newTransitions.push_back(existing);
        }
    }
    
    // Create all bypass transitions
    for (auto& t1 : inTransitions) {
        for (auto& t2 : outTransitions) {
            if (t1.fromState == t2.toState) {
                // Skip - already handled as cycle in Step 3
                continue;
            }
            
            // Build bypass regex including self-loop on removed state
            string newRegex = t1.regex;
            if (!selfLoopRegex.empty()) {
                newRegex += "(" + selfLoopRegex + ")*";
            }
            newRegex += t2.regex;
            
            // Check if transition already exists and merge
            bool foundInNew = false;
            for (auto& existing : newTransitions) {
                if (existing.fromState == t1.fromState && existing.toState == t2.toState) {
                    existing.regex = "(" + existing.regex + ")|(" + newRegex + ")";
                    foundInNew = true;
                    break;
                }
            }
            
            if (!foundInNew) {
                newTransitions.push_back({t1.fromState, t2.toState, newRegex});
            }
        }
    }

    // Add the cycles to the transitions list
    for (auto& cycle : cyclesCreated) {
        int state = cycle.first;
        string cycleRegex = cycle.second;  // NO KLEENE STAR HERE - already included above
        
        bool foundInNew = false;
        for (auto& existing : newTransitions) {
            if (existing.fromState == state && existing.toState == state) {
                existing.regex = "(" + existing.regex + ")|(" + cycleRegex + ")";
                foundInNew = true;
                break;
            }
        }
        
        if (!foundInNew) {
            newTransitions.push_back({state, state, cycleRegex});
        }
    }

    // Replace the old transitions with the new ones.
    nfa.transitions = newTransitions;

    // Remove the state that we are removing from the states list.
    for (int i = 0; i < nfa.states.size(); i++) {
        if (nfa.states[i].stateNumber == stateToRemove) {
            nfa.states.erase(nfa.states.begin() + i);
            break;
        }
    }
}

string convertToRegex(NFA nfa) {
    // This function is the blueprint for the process of recursively converting an NFA to a regex.
    // It will remove the intermediate states, and slowly compact the NFA into a two-state GNFA.

    // Edge case: Empty NFA or single state
    if (nfa.states.empty()) return "";
    if (nfa.states.size() == 1) {
        // If the only state is both start and final
        if (nfa.startState == nfa.finalStates[0]) {
            // Look for self-loop
            for (auto& t : nfa.transitions) {
                if (t.fromState == nfa.startState && t.toState == nfa.startState) {
                    return "(" + t.regex + ")*";
                }
            }
            return "";  // Accept empty string only
        }
        return "";  // No valid path
    }

    // Consolidate multiple transitions between same states
    map<pair<int,int>, string> transitionMap;
    for (auto& t : nfa.transitions) {
        auto key = make_pair(t.fromState, t.toState);
        if (transitionMap.find(key) != transitionMap.end()) {
            transitionMap[key] = "(" + transitionMap[key] + ")|(" + t.regex + ")";
        } else {
            transitionMap[key] = t.regex;
        }
    }

    // Rebuild transitions vector
    nfa.transitions.clear();
    for (auto& entry : transitionMap) {
        nfa.transitions.push_back({entry.first.first, entry.first.second, entry.second});
    }

    // Bonus Step: If we have multiple final state, we will combine them into one single final state. 
    if (nfa.finalStates.size() > 1) {
        int superFinalState = nfa.states.size(); // New state #.

        nfa.states.push_back({superFinalState, true});

        // Now we must connect all our final states to the super final state via epsilon transitions. 
        for (auto& finalState : nfa.finalStates) {
            nfa.transitions.push_back({finalState, superFinalState, ""});

            for (auto& state : nfa.states) {
                if (state.stateNumber == finalState) {
                    state.isFinal = false;
                }
            }
        }
        nfa.finalStates = {superFinalState};
    }

    // Beginning with assuming we have more than two states, so we can remove at least one. 
    while (nfa.states.size() > 2) {
        int stateToRemove = -1;
        for (auto& state : nfa.states) {
            if ((state.stateNumber != nfa.startState) && !state.isFinal) {
                // We take the first state we find that isn't part of the starting or final sets, to remove. 
                stateToRemove = state.stateNumber;
                break;
            }
        }

        // If stateToRemove is still -1, we have no more intermediate states to remove.

        if (stateToRemove == -1) {
            cout << "No more intermediate state to remove found." << endl;
            break;
        }
        else {
            removeState(nfa, stateToRemove);
        }
    }

    // Build final regex considering ALL possible patterns
    string startLoop = "";
    string startToFinal = "";
    string finalLoop = "";
    
    for (auto& t : nfa.transitions) {
        // Self-loop on start state
        if (t.fromState == nfa.startState && t.toState == nfa.startState) {
            if (startLoop.empty()) {
                startLoop = t.regex;
            } else {
                startLoop = "(" + startLoop + ")|(" + t.regex + ")";
            }
        }
        
        // Direct path from start to final
        for (auto& finalState : nfa.finalStates) {
            if (t.fromState == nfa.startState && t.toState == finalState) {
                if (startToFinal.empty()) {
                    startToFinal = t.regex;
                } else {
                    startToFinal = "(" + startToFinal + ")|(" + t.regex + ")";
                }
            }
            
            // Self-loop on final state
            if (t.fromState == finalState && t.toState == finalState) {
                if (finalLoop.empty()) {
                    finalLoop = t.regex;
                } else {
                    finalLoop = "(" + finalLoop + ")|(" + t.regex + ")";
                }
            }
        }
    }
    
    // Construct final regex: (startLoop)* startToFinal (finalLoop)*
    string regex = "";
    if (!startLoop.empty()) {
        regex += "(" + startLoop + ")*";
    }
    regex += startToFinal;
    if (!finalLoop.empty()) {
        regex += "(" + finalLoop + ")*";
    }
    
    return regex;
}


int backend_main() {
    // The following is a simple NFA that we will be using for testing. (hardcoded)
    // An image of this is in the project folder. 

    // In the future, we can also accept input from the user. (commented out for now)

    NFA nfa;
    nfa.startState = 0;
    nfa.finalStates = {1};

    // Adding middle states
    nfa.states.push_back({0, false}); // q0
    nfa.states.push_back({1, true}); // q1 (final state)
    nfa.states.push_back({2, false}); // q2

    // Adding transitions
    nfa.transitions.push_back({0, 0, "b"});  // q0 -> q0 on b
    nfa.transitions.push_back({0, 1, "a"});  // q0 -> q1 on a
    nfa.transitions.push_back({0, 2, "b"});  // q0 -> q2 on b
    nfa.transitions.push_back({1, 1, "a"});  // q1 -> q1 on a,b
    nfa.transitions.push_back({1, 1, "b"});
    nfa.transitions.push_back({2, 2, "a"});  // q2 -> q2 on a

    // The following is the expected output for the example NFA above.

    string result = convertToRegex(nfa);

    cout << "The result is: " << result << endl;

    return 0;
}