network wn14
{
    types:
        channel canal extends ned.DelayChannel{ delay = 100ms; }
    submodules:
        box14[6]: box14;
    connections:
        box14[0].gate++ <--> canal <--> box14[1].gate++;
        box14[1].gate++ <--> canal <--> box14[2].gate++;
        box14[1].gate++ <--> canal <--> box14[4].gate++;
        box14[3].gate++ <--> canal <--> box14[4].gate++;
        box14[4].gate++ <--> canal <--> box14[5].gate++;
}

simple box14
{
    parameters:
        @display("i=block/rxtx");
    
    gates:
        inout gate[];
}



