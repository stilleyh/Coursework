Pseudo Implementation:

CSem(K) cs { // counting semaphore initialized to K
    int val ← K; // the value of csem
    BSem gate(min(1,val)); // 1 if val > 0; 0 if val = 0
    BSem mutex(1); // protects val

    Pc(cs) {
        P(gate)
    a1: P(mutex);
        val ← val − 1;
        if val > 0
            V(gate);
        V(mutex);
    }
    Vc(cs) {
        P(mutex);
        val ← val + 1;
        if val = 1
            V(gate);
        V(mutex);
    }
}